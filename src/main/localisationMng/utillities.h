/*
 * @file utillities.h
 * @brief Final version with only constants, enums, data structures, and API declarations.
 */

#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <vector>

// =================== Constants ===================
#define SCAN_BATCH_SIZE             (15)
#define SCAN_SAMPLE_PER_BATCH       (3)
#define SCAN_VALIDATION_SAMPLE_SIZE (5)
#define VALIDATION_PASS_THRESHOLD   (3)
#define MAX_SCAN_RETRY_ATTEMPTS     (5)
#define SCAN_DELAY_MS               (100)
#define ALPHA                       (0.7f)
#define DEFAULT_RSSI_VALUE          (-100)
#define NUMBER_OF_ANCHORS           (10)
#define NUMBER_OF_RESPONDERS        (4)
#define NUMBER_OF_LOCATIONS         (17)  

//Just for KNN use
#define K (4)

extern const char* anchorSSIDs[NUMBER_OF_ANCHORS];

// =================== Enums ===================

typedef enum Label {
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
} Label;

typedef enum SystemState {
    STATIC_RSSI             = 0, 
    STATIC_RSSI_TOF         = 1,
    STATIC_DYNAMIC_RSSI     = 2, 
    STATIC_DYNAMIC_RSSI_TOF = 3,
    OFFLINE                 = 4
} SystemState;

// =================== Structures ===================

struct RSSIData {
    int RSSIs[TOTAL_APS];
    LOCATIONS label;
};

struct TOFData {
    float TOFs[NUM_TOF_RESPONDERS];
    LOCATIONS label;
};

// =================== Globals ===================

extern std::vector<RSSIData> rssiDataSet;
extern std::vector<TOFData> tofDataSet;
extern Label currentScanningLabel;

// =================== Utilities Functions Decleration ===================

/**
 * @brief Applies Exponential Moving Average (EMA) filter to RSSI.
 * @param prevRSSI Previous RSSI value
 * @param newRSSI Newly measured RSSI value
 * @return Smoothed RSSI
 */
int applyEMA(int prevRSSI, int newRSSI);

/**
 * @brief Prompts user to select a location label.
 * @return Label selected by the user
 */
char* promptLocationSelection();

/**
 * @brief Maps a label enum to its string name.
 * @param label Integer label
 * @return String representation
 */
const char* labelToString(int label);

/**
 * @brief Prompts user to select system mode.
 * @return Selected SystemState enum
 */
SystemState promptSystemState();

/**
 * @brief Maps a SystemState enum to string.
 * @param state Integer state
 * @return String name
 */
const char* systemStateToString(int state);

/**
 * @brief Prompts user whether they approve the scan accuracy.
 * @return true if approved, false otherwise
 */
bool promptUserAccuracyApprove();

#endif // _UTILITIES_H_
