/*
 * @file utillities.h
 * @brief Final version with only constants, enums, data structures, and API declarations.
 */

#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <vector>

// =================== Constants ===================
#define SCANS_PER_LOCATION (20)
#define SAMPLES_PER_SCAN (3)
#define SCAN_DELAY_MS (100)
#define ALPHA (0.7f)
#define NUMBER_OF_ANCHORS (10)
#define NUM_TOF_RESPONDERS (4)
#define NUMBER_OF_LOCATIONS (17)  
#define NUMBER_OF_SCANS (SCANS_PER_LOCATION * NUMBER_OF_LOCATIONS)
#define TOTAL_APS (NUMBER_OF_ANCHORS + NUM_TOF_RESPONDERS)
#define FILES_THRESHOLD (5) /// need to check!
//Just for KNN use
#define K (4)

extern const char* anchorSSIDs[TOTAL_APS];

typedef enum SystemState {
    STATIC_RSSI             = 0, 
    STATIC_RSSI_TOF         = 1,
    STATIC_DYNAMIC_RSSI     = 2, 
    STATIC_DYNAMIC_RSSI_TOF = 3,
    OFFLINE                 = 4
} SystemState

struct CurrentConfig {
    String layoutVersion;
    std::vector<String> anchorMacs; // Store MAC addresses
    int numSegments;
    int numScansPerSegment;
    int featureCount;
    unsigned long lastScanTimestamp;
    // Add other fields as per meta.txt requirements from the PDF
};


//Enablements -- regarding to system running mode
struct enablements {
    static bool enable_training_model_on_host_machine = false;
    static SystemState currentSystemState             = SystemState::OFFLINE;
    static bool enable_SD_Card_backup 
}
// =================== Enums ===================


typedef enum LOCATIONS {
    NOT_ACCURATE          = 0,
    NEAR_ROOM_232         = 1,
    NEAR_ROOM_234         = 2,
    BETWEEN_ROOMS_234_236 = 3,
    ROOM_236              = 4,
    ROOM_231              = 5,
    BETWEEN_ROOMS_231_236 = 6,
    NEAR_BATHROOM         = 7,
    NEAR_KITCHEN          = 8,
    KITCHEN               = 9,
    MAIN_ENTRANCE         = 10,
    NEAR_ROOM_230         = 11,
    LOBBY                 = 12,
    ROOM_201              = 13,
    PRINTER               = 14,
    MAIN_EXIT             = 15,
    BALCONY_ENTRANCE      = 16,
    OFFICES_HALL          = 17
} LOCATIONS ;
 

// =================== Data Structures ===================

struct Data {
    int RSSIs[TOTAL_APS];
    float TOFs[NUM_TOF_RESPONDERS];
    LOCATIONS location;
     unsigned long timestamp;
     
    // parse one CSV line
    static Data fromCSV(const String &line) {
        Data row{};
        int start = 0, comma;
        for (int i = 0; i < FEATURE_COUNT_RSSI; ++i) {
            comma = line.indexOf(',', start);
            row.RSSIs[i] = line.substring(start, comma).toInt();
            start = comma + 1;
        }
        for (int i = 0; i < FEATURE_COUNT_TOF; ++i) {
            comma = line.indexOf(',', start);
            row.TOFs[i] = line.substring(start, comma).toInt();
            start = comma + 1;
        }
        comma = line.indexOf(',', start);
        row.location = (LOCATIONS)line.substring(start, comma).toInt();
        start = comma + 1;
        row.timestamp = line.substring(start).toInt();
        return row;
    }

    // serialize to CSV
    String toCSV() const {
        String s;
        for (int i = 0; i < FEATURE_COUNT_RSSI; ++i) s += String(rssi[i]) + ',';
        for (int i = 0; i < FEATURE_COUNT_TOF;  ++i) s += String(tof[i])  + ',';
        s += String((int)location) + ',' + String(timestamp);
        return s;
    }

};

// =================== Data Set Vector ===================
extern std::vector<Data> dataSet;

// =================== Function Declarations ===================
inline const char* stateToString(SystemState s) {
    switch (s) {
        case SystemState::STATIC_RSSI:             return "STATIC_RSSI";
        case SystemState::STATIC_RSSI_TOF:         return "STATIC_RSSI_TOF";
        case SystemState::STATIC_DYNAMIC_RSSI:     return "STATIC_DYNAMIC_RSSI";
        case SystemState::STATIC_DYNAMIC_RSSI_TOF: return "STATIC_DYNAMIC_RSSI_TOF";
        case SystemState::OFFLINE:                 return "OFFLINE";
    }
    return "UNKNOWN";
}
// —————— Session CSV helpers ——————

// Build a per-session CSV name, e.g. "/scans_v03.csv"
inline String sessionFilename(int layoutVersion) {
  char buf[24];
  sprintf(buf, "/scans_v%02d.csv", layoutVersion);  // two-digit padding :contentReference[oaicite:2]{index=2}
  return String(buf);
}

// Prune old CSVs, keeping only the newest N
inline void pruneOldCSVs(int keepNewest) {
  File root = SD.open("/");
  std::vector<String> files;
  File f;
  while ((f = root.openNextFile())) {
    String n = f.name();
    if (n.startsWith("/scans_v") && n.endsWith(".csv"))
      files.push_back(n);
    f.close();
  }
  std::sort(files.begin(), files.end());  // lexicographic = version order
  while ((int)files.size() > keepNewest) {
    SD.remove(files.front());
    files.erase(files.begin());
  }
}

// Global var carrying current session’s CSV path
extern String sessionFile;


#endif // _UTILITIES_H_
