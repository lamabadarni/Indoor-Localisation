/**
 * @file utillities.h
 * @brief System-wide definitions for constants, enums, data structures, and utility APIs 
 *        used by the indoor localization platform (RSSI/TOF-based).
 */

#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <vector>

// ====================== Constants ======================

#define MIN_ANCHORS_VISIBLE         (3)
#define MIN_AVERAGE_RSSI_DBM        (-75)
#define TOF_MIN_RESPONDERS_VISIBLE  (2)
#define TOF_MAX_AVERAGE_DISTANCE_CM (400)

#define RSSI_SCAN_BATCH_SIZE        (15)
#define RSSI_SCAN_SAMPLE_PER_BATCH  (3)
#define RSSI_DEFAULT_VALUE          (-100)
#define RSSI_SCAN_DELAY_MS          (100)
#define NUMBER_OF_ANCHORS           (10)

#define TOF_NUMBER_OF_MAC_BYTES     (6)
#define TOF_SCAN_BATCH_SIZE         (5)
#define TOF_MAX_VALID_CM            (500.0f)  // Adjustable max valid TOF reading
#define TOF_DEFAULT_DISTANCE_CM     (-1)
#define NUMBER_OF_RESPONDERS        (4)

#define SCAN_VALIDATION_SAMPLE_SIZE (5)
#define VALIDATION_PASS_THRESHOLD   (3)
#define MAX_RETRIES                 (5)

#define ALPHA                       (0.7f)
#define NUMBER_OF_LABELS            (17)
#define K                           (4) // KNN usage


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
    int RSSIs[NUMBER_OF_ANCHORS];  // Define TOTAL_APS if needed
    LOCATIONS label;
};

struct TOFData {
    float TOFs[NUMBER_OF_RESPONDERS]; // Define if needed
    LOCATIONS label;
};


// ====================== Globals ======================

extern SystemMode            currentSystemMode;
extern SystemState           currentSystemState;
extern Label                 currentScanningLabel;

extern std::vector<RSSIData> rssiDataSet;
extern std::vector<TOFData>  tofDataSet;

extern const char*           anchorSSIDs[NUMBER_OF_ANCHORS];
extern const uint8_t         responderMacs[NUMBER_OF_RESPONDERS][TOF_NUMBER_OF_MAC_BYTES];


// ====================== Enablements ======================

struct Enablements {
    static bool enable_training_model_on_host_machine;
    static bool enable_SD_Card_backup;
    static bool run_validation_phase;
    static bool verify_responder_mac_mapping;
    static bool verify_rssi_anchor_mapping;
};


// ====================== Utility Functions ======================

/**
 * @brief Apply EMA (exponential moving average) to RSSI.
 */
int applyEMA(int prevRSSI, int newRSSI);

/**
 * @brief Prompt user to select a location label.
 */
char* promptLocationSelection();

/**
 * @brief Convert label enum to string.
 */
const char* labelToString(int label);

/**
 * @brief Prompt user to select system state.
 */
SystemState promptSystemState();

/**
 * @brief Convert system state enum to string.
 */
const char* systemStateToString(int state);

/**
 * @brief Prompt user to approve scan accuracy.
 */
bool promptUserAccuracyApprove();

/**
 * @brief Check whether the stored data is valid for a location.
 */
bool isLocationValid(LOCATION location);

#endif // _UTILITIES_H_
