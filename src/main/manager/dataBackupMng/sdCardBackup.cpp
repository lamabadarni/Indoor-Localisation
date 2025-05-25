
#include <SD.h>
#include <SPI.h>
#include <Arduino.h>
#include <sdCardBackup.h>
#include <userUI.h>


static String CurrCsvPathToOpen = "";
static String metaPathToOpen = "";
static String accuracyPathToOpen = "";
String tmp = ".tmp";
//------------------------------------------------------------------------------
// Forward declarations for local (static) functions
//------------------------------------------------------------------------------
static bool loadFileToDataset(const String& path, bool isTofFile);
static bool verifyCSVFormat(const String& header, bool isTofFile);
static std::vector<String> splitedString;
static void fromCSVRssiToVector( String line);
static void fromCSVTofToVector( String line);
static bool deleteInvalidLocations(const String& filePath);
//==================================================================
//          PUBLIC FUNCTIONS
//==================================================================

static void splitByComma(String data, char comma) {
    splitedString.clear();
    int sepIndex;
    while ((sepIndex = data.indexOf(comma)) != -1) {
        splitedString.push_back(data.substring(0, sepIndex));
        data = data.substring(sepIndex + 1);
    }
    splitedString.push_back(data); // last segment
}

bool initSDCard() {
    if (!SD.begin(csPin)) {
        Serial.println("SD init failed. Prompting user...");
        return promptUserSDCardInitializationApprove();
    }
    Serial.println("SD card initialized.");
    return true;
}

bool loadLocationDataset() {
    rssiDataSet.clear();
    bool ok = true;
    auto state = currentSystemState;

    // Load RSSI if required

        if (!loadFileToDataset(getRSSIFilePath(), false)) 
           ok = false;  // isTofFile=false
    

    // Load TOF if required
    if (state == SystemState::STATIC_RSSI_TOF ||
        state == SystemState::STATIC_DYNAMIC_RSSI_TOF) {
        if (!loadFileToDataset(getTOFFilePath(), true)) ok = false;    // isTofFile=true
    }

    if (!ok) Serial.println("Error loading one or more dataset files.");
    return ok;
}

bool deleteInvalidLocations(const String& filePath) {
    if (!SD.exists(filePath)) {
        Serial.println("File not found for filtering: " + filePath);
        return false;
    }

    String tmpPath = filePath + tmp;
    File inputFile = SD.open(filePath, FILE_READ);
    File outputFile = SD.open(tmpPath, FILE_WRITE);
    if (!inputFile || !outputFile) {
        Serial.println("Error opening files for filtering.");
        if (inputFile) inputFile.close();
        if (outputFile) outputFile.close();
        return false;
    }

    // Copy header
    String header = inputFile.readStringUntil('\n');
    outputFile.println(header);

    // Copy only valid data rows
    while (inputFile.available()) {
        String line = inputFile.readStringUntil('\n');
        line.trim();
        if (line.length() < 3) continue;

        int lastComma = line.lastIndexOf(',');
        String temp = line.substring(lastComma+1);
        temp.trim();
        Label loc = (Label)temp.toInt();
        if (reuseFromSD[loc]) {
            outputFile.println(line);
        }
    }
    inputFile.close();
    outputFile.close();

    SD.remove(filePath);
    SD.rename(tmpPath, filePath);
    Serial.println("Filtered file: " + filePath);
    return true;
}

bool updateCSV() {
    bool ok = true;
    auto state = currentSystemState;

    if (state == SystemState::STATIC_RSSI_TOF || state == SystemState::STATIC_DYNAMIC_RSSI_TOF) {
        ok &= deleteInvalidLocations(getRSSIFilePath());
        ok &= deleteInvalidLocations(getTOFFilePath());
    } else if (state == SystemState::STATIC_RSSI || state == SystemState::STATIC_DYNAMIC_RSSI) {
        ok &= deleteInvalidLocations(getRSSIFilePath());
    }

    
    if (ok) 
      return loadLocationDataset();
    Serial.println("updateCSV failed.");
    return false;
}

void fromCSVRssiToVector( String line){
    RSSIData row{};
     splitByComma(line,',');
     if(splitedString.size() != NUMBER_OF_ANCHORS+1){
        return;
     }

     for(int i=0 ; i<NUMBER_OF_ANCHORS;i++){
     row.RSSIs[i]=splitedString[i].toInt();
     }
     row.label=(Label)splitedString[NUMBER_OF_ANCHORS].toInt();
     rssiDataSet.push_back(row);
     return;
}

void fromCSVTofToVector( String line){
    TOFData row{};
     splitByComma(line,',');
     if(splitedString.size() != NUMBER_OF_RESPONDERS+1){
        return;
     }

     for(int i=0 ; i<NUMBER_OF_RESPONDERS;i++){
     row.TOFs[i]=splitedString[i].toDouble();
     }
     row.label=(Label)splitedString[NUMBER_OF_RESPONDERS].toInt();
}


//==================================================================
//          PRIVATE (STATIC) FUNCTIONS
//==================================================================

static bool loadFileToDataset(const String& path, bool isTofFile) {
    File f = SD.open(path, FILE_READ);
    if (!f) {
        Serial.println("Failed to open for reading: " + path);
        return false;
    }
    String header = f.readStringUntil('\n');
    header.trim();
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() < 3) continue;
        if (isTofFile) (fromCSVTofToVector(line));
        else           (fromCSVRssiToVector(line));
    }
    f.close();
    Serial.println("Successfully loaded data from: " + path);
    return true;
}


bool saveRSSIScan(const RSSIData &row) {
    // Open (or create) in append mode
    File f = SD.open(getRSSIFilePath(), FILE_APPEND);
    if (!f) return false;

    // If file empty, write header first
    if (f.size() == 0) {
        for (int i = 1; i <= NUMBER_OF_ANCHORS; ++i) {
            f.print(String(i) + "_rssi,");
        }
        f.println("Location");
    }

    // Write the RSSI values
    for (int i = 0; i < NUMBER_OF_ANCHORS; ++i) {
        f.print(String(row.RSSIs[i]));
        f.print(',');
    }
    // Then location label and a timestamp
    f.println(String((int)row.label));

    f.close();
    return true;
}

bool saveTOFScan(const TOFData &row) {
    File f = SD.open(getTOFFilePath(), FILE_APPEND);
    if (!f) return false;

    // If file empty, write header first
    if (f.size() == 0) {
        for (int j = 1; j <= NUMBER_OF_RESPONDERS; ++j) {
            f.print(String(j) + "_tof,");
        }
        f.println("Location");
    }

    // Write the TOF values
    for (int j = 0; j < NUMBER_OF_RESPONDERS; ++j) {
        f.print(String(row.TOFs[j]));
        f.print(',');
    }
    // Then location label and a timestamp
    f.println(String((int)row.label));

    f.close();
    return true;
}