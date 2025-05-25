/**
 * @file utilities.h
 * @brief System-wide definitions for constants, enums, data structures, and utility APIs 
 *        used by the indoor localization platform (RSSI/TOF-based).
 */

#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <Arduino.h>
#include <vector>
#include <stdint.h>
#include <SD.h>
#include <WiFi.h>

// ====================== Constants ======================

#define NUMBER_OF_LABELS            (17)
#define MODE_COUNT                  (5)

// == Coverage ==
#define MIN_ANCHORS_VISIBLE         (3)
#define MIN_AVERAGE_RSSI_DBM        (-75)
#define TOF_MIN_RESPONDERS_VISIBLE  (2)
#define TOF_MAX_AVERAGE_DISTANCE_CM (600)

// == RSSI Scanner ==
#define RSSI_SCAN_BATCH_SIZE        (15)
#define RSSI_SCAN_SAMPLE_PER_BATCH  (3)
#define RSSI_DEFAULT_VALUE          (-100)
#define RSSI_SCAN_DELAY_MS          (100)
#define NUMBER_OF_ANCHORS           (10)

// == TOF Scanner ==
#define TOF_NUMBER_OF_MAC_BYTES     (6)
#define TOF_SCAN_BATCH_SIZE         (5)
#define TOF_MAX_VALID_CM            (500.0)  // Adjustable max valid TOF reading
#define TOF_DEFAULT_DISTANCE_CM     (1500.0)
#define NUMBER_OF_RESPONDERS        (4)

// == Validation consts == 
#define NUM_OF_VALIDATION_SCANS     (5)
#define VALIDATION_MAX_ATTEMPTS     (3)
#define VALIDATION_PASS_THRESHOLD   (0.6)

// == User UI ==
#define MAX_RETRIES                 (5)

// == Predection consts ==
#define ALPHA                       (0.7f)
#define K_RSSI                      (4) 
#define K_TOF                       (2)
#define MIN_VALID_DATA_SET_SIZE     (K_RSSI * 10) 
#define MIN_DATA_PER_LABEL_SIZE     (K_RSSI * 3) 

// == SD Card Consts ==
#define MAX_RETRIES_TO_INIT_SD_CARD (2)
#define csPin                       (5) // Chip Select pin for SD card
static constexpr char META_FILENAME[]        = "meta_.csv";
static constexpr char RSSI_FILENAME[]        = "rssi_scan_data_.csv";
static constexpr char TOF_FILENAME[]         = "tof_scan_data_.csv";
static constexpr char ACCURACY_FILENAME[]    = "location_accuracy_.csv";
static constexpr char TMP_SUFFIX[]           = ".tmp";

// ====================== Enums ======================

typedef enum Label {
    NOT_ACCURATE = 0,
    NEAR_ROOM_232,
    NEAR_ROOM_234,
    BETWEEN_ROOMS_234_236,
    ROOM_236,
    ROOM_231,
    BETWEEN_ROOMS_231_236,
    NEAR_BATHROOM,
    NEAR_KITCHEN,
    KITCHEN,
    MAIN_ENTRANCE,
    NEAR_ROOM_230,
    LOBBY,
    ROOM_201,
    PRINTER,
    MAIN_EXIT,
    BALCONY_ENTRANCE,
    OFFICES_HALL
} Label;

typedef enum SystemState {
    STATIC_RSSI = 0,
    STATIC_RSSI_TOF,
    STATIC_DYNAMIC_RSSI,
    STATIC_DYNAMIC_RSSI_TOF,
    OFFLINE
} SystemState;

typedef enum SystemMode {
    MODE_FULL_SESSION = 0,
    MODE_TOF_DIAGNOSTIC,
    MODE_RSSI_MODEL_DIAGNOSTIC,
    MODE_TRAINING_ONLY,
    MODE_PREDICTION_ONLY
} SystemMode;

// ====================== Enablements ======================

struct Enablements {
    static bool enable_training_model_on_host_machine;
    static bool enable_SD_Card_backup;
    static bool run_validation_phase;
    static bool verify_responder_mac_mapping;
    static bool verify_rssi_anchor_mapping;
};

// ====================== Data Structures ======================

struct RSSICoverageResult {
    Label label;
    int visibleAnchors;
    int averageRSSI;
    bool anchorVisibility[NUMBER_OF_ANCHORS];
    int anchorRSSI[NUMBER_OF_ANCHORS];
};

struct TOFCoverageResult {
    Label label;
    int visibleResponders;
    int averageDistance;
    bool responderVisibility[NUMBER_OF_RESPONDERS];
    int responderDistance[NUMBER_OF_RESPONDERS];  // in cm
};

struct RSSIData {
    int RSSIs[NUMBER_OF_ANCHORS];  
    Label label;
};

struct TOFData {
    double TOFs[NUMBER_OF_RESPONDERS];
    Label label;
};

struct AccuracyData {
    Label location;
    float accuracy;
};

struct ScanConfig {
    SystemState systemState;
    int RSSINum;
    int TOFNum;
};

// ====================== Globals ======================

extern SystemMode               currentSystemMode;
extern SystemState              currentSystemState;
extern Label                    currentLabel;
extern ScanConfig               currentConfig;

extern std::vector<RSSIData>     rssiDataSet;

//extern std::vector<AccuracyData> accuracyDatas;
extern bool                      reuseFromSD[NUMBER_OF_LABELS];
extern double                    accuracy;
extern int                       accumulatedRSSIs[NUMBER_OF_ANCHORS];

extern const char*               anchorSSIDs[NUMBER_OF_ANCHORS];

// ====================== Utility Functions ======================

/**
 * @brief Apply EMA (exponential moving average) to RSSI.
 */
int applyEMA(int prevRSSI, int newRSSI);

/**
 * @brief Convert label enum to string.
 */
const char* labelToString(int label);

/**
 * @brief Convert system state enum to string.
 */
const char* systemStateToString(int state);

/**
 * @brief Prompt user to approve scan accuracy.
 */
bool promptUserAccuracyApprove();

/**
 * @brief Get the base directory on the SD card for the current system state.
 */
String getSDBaseDir();

/**
 * @brief Get the full path to the meta file based on current system state.
 */
String getMetaFilePath();

/**
 * @brief Get the full path to the RSSI scan data file.
 */
String getRSSIFilePath();

/**
 * @brief Get the full path to the TOF scan data file.
 */
String getTOFFilePath();

/**
 * @brief Get the full path to the location accuracy file.
 */
String getAccuracyFilePath();

bool resetStorage();


// ====================== Conversion ======================

const char* systemStateToString(int state);
const char* modeToString(SystemMode mode);

#endif // _UTILITIES_H_
