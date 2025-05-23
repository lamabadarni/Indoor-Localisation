
#include <Arduino.h>
#include <vector>
#include <WiFi.h>
#include "../localisationMng/scanning.h"
#include "../localisationMng/predictionPhase.h"

String sessionFile;  // defined as extern in utilities.h
extern ScanConfig currentConfig;

std::vector<Data> dataSet;
enum class InvalidationReason { MissingFiles, CsvIncomplete, MetaMismatch, SdUnavailable };
typedef void(*InvalidationHandler)(InvalidationReason);
void defaultInvalidationHandler(InvalidationReason r) {
    const char* msgs[] = { "Missing files", "CSV incomplete", "Meta mismatch", "SD unavailable" };
    Serial.printf("⚠ Invalidate: %s\n", msgs[(int)r]);
}

InvalidationHandler onInvalidate = defaultInvalidationHandler;

// Globals
bool sdAvailable = false;




SystemState currentSystemState = SystemState::STATIC_RSSI_TOF;
ScanConfig   currentConfig = {
    .layoutVersion    = 1,
    .anchorMacs       = { "AA:BB:CC:11:22:33", "DD:EE:FF:44:55:66" },
    .expectedSegments = 3,
    .scansPerSegment  = 5,
    .featureCountRSSI = FEATURE_COUNT_RSSI,
    .featureCountTOF  = FEATURE_COUNT_TOF
};

String makeDatasetKey(const ScanConfig& cfg, SystemState state) {
    String k = "v" + String(cfg.layoutVersion)
             + "-a" + String(cfg.anchorMacs.size())
             + "-s" + String(cfg.expectedSegments)
             + "-n" + String(cfg.scansPerSegment)
             + "-r" + String(cfg.featureCountRSSI)
             + "-t" + String(cfg.featureCountTOF)
             + "_"  + stateToString(state);
    return k;
}

const char* anchorSSIDs[TOTAL_APS] = {
        "234/236",
        "236",
        "231/236",
        "231", 
        "kitchen",
        "entrance",
        "lobby",
        "balcony",
        "201",
        "offices"
};

void sdInitInteractive() {

  if (enablements::enable_SDCard_backup) {
    while (true) {
      Serial.print("Init SD… ");
      sdAvailable = initSD();
      if (sdAvailable) { Serial.println("OK "); break; }
      Serial.println("FAIL");
      Serial.println("(I) Retry  (C) Skip SD  (A) Abort");
      while (!Serial.available()) delay(50);
      char c = Serial.read(); Serial.read();
      if (c=='I'||c=='i') continue;
      if (c=='C'||c=='c') { sdAvailable=false; Serial.println("skip SD"); break; }
      if (c=='A'||c=='a') { Serial.println("Aborting"); while(1) delay(1000); }
    }
  } 
  else {
    sdAvailable = initSD();
    if (!sdAvailable) Serial.println(" SD disabled");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // 1) Init SD (interactive if enabled)
  sdInitInteractive();            // sets sdAvailable
  if (sdAvailable) pruneOldCSVs(FILES_THRESHOLD);

  // 2) Build per‐config filenames
  String key = sessionFilename(currentConfig.layoutVersion);
  sessionFile = "/" + key + ".csv";
  metaFile    = "/" + key + ".meta";
  // 3) If no SD, do fully interactive scan and exit setup
  if (!sdAvailable) {
    Serial.println(" SD unavailable  ENTER INTERACTIVE SCAN MODE");
    for (int i = 0; i < currentConfig.expectedSegments; ++i) {
    std::vector<Data> LocationScanBuffer;
      LOCATIONS loc = promptLocationSelection();
      Serial.println("Press Enter to start scanning...");
      while (!Serial.available()) delay(50);
      Serial.read();  // clear newline
      Serial.printf(" Scanning location %d\n", (int)loc);

      performScan(loc, LocationScanBuffer);
      dataSet.insert(dataSet.end(), LocationScanBuffer.begin(), LocationScanBuffer.end());
      Serial.printf(" Completed %u scans for loc %d\n", LocationScanBuffer.size(), (int)loc);
    }
    Serial.printf("INTERACTIVE SCAN COMPLETE: total=%u\n", dataSet.size());
    return;
  }

  // 4) SD is available → decide reuse vs. fresh
  bool reuse = shouldReuseScans(metaFile.c_str(),
                                sessionFile.c_str(),
                                currentConfig);

  std::vector<int> doneLocs;
  if (reuse) {
    // prune partial segments
    pruneIncompleteSegments(sessionFile.c_str(), currentConfig, doneLocs);

    // load fully-done locations into RAM
    for (int locId : doneLocs) {
      std::vector<Data> rows;
      loadLocationDataset(sessionFile.c_str(),
                          (LOCATIONS)locId,
                          rows);
      dataSet.insert(dataSet.end(), rows.begin(), rows.end());
      Serial.printf(" Reused %u scans for loc %d\n", rows.size(), locId);
    }
  } else {
    // wipe old debris
    SD.remove(sessionFile);
    SD.remove(metaFile);
  }

  // 5) Scan any still-missing locations
  for (int locId = 0; locId < currentConfig.expectedSegments; ++locId) {
       std::vector<Data> LocationScanBuffer;
    if (reuse && std::count(doneLocs.begin(), doneLocs.end(), locId))
      continue;

    Serial.printf(" Scanning location %d…\n", locId);
    performScan((LOCATIONS)locId, LocationScanBuffer);

    for (auto &d : LocationScanBuffer) {
      dataSet.push_back(d);
      saveLocationDataset(sessionFile, d);
    }
    Serial.printf("Scanned & saved %u entries for loc %d\n", LocationScanBuffer.size(), locId);
  }

  // 6) Snapshot metadata for next run
  saveMeta(metaFile.c_str(), currentConfig, millis()/1000);

  Serial.printf("SETUP COMPLETE: total scans=%u\n", dataSet.size());
}

void confirmLocation(LOCATIONS loc) {
    Serial.printf("Predicted location: %s\n", locationToString(loc));
    Serial.println("Is this correct? (Y/N)");
    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == 'Y' || c == 'y') {
                Serial.println("Location confirmed.");
                break;
            } else if (c == 'N' || c == 'n') {
                Serial.println("Location not confirmed.");
                break;
            }
        }
    }
}

void loop() {
    Serial.println("Stand still to predict location. When you're ready, press Enter...");
    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') break;
        }
    }

    int point[NUMBER_OF_ANCHORS] = {-100, -100, -100, -100, -100, -100, -100, -100, -100, -100}; 
    for (int sample = 0; sample < SAMPLES_PER_SCAN; ++sample) {
        int n = WiFi.scanNetworks();
        for (int j = 0; j < n; ++j) {
            String ssid = WiFi.SSID(j);
            int rssi = WiFi.RSSI(j);
            for (int k = 0; k < TOTAL_APS; ++k) {
                if (ssid.equals(anchorSSIDs[k])) {
                    point[k] = applyEMA(point[k], rssi);
                }
            }
        }
    }

    // Predict location and confirm with user
    preparePoint(point);
    LOCATIONS predictedLoc = static_cast<LOCATIONS>(knnPredict(point));
    confirmLocation(predictedLoc);
}
