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

//Just for KNN use
#define K (4)

extern const char* anchorSSIDs[TOTAL_APS];

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
} Label ;

typedef enum SystemState {
    STATIC_RSSI             = 0, 
    STATIC_RSSI_TOF         = 1,
    STATIC_DYNAMIC_RSSI     = 2, 
    STATIC_DYNAMIC_RSSI_TOF = 3,
    OFFLINE                 = 4
} SystemState
 

// =================== Data Structures ===================

struct Data {
    int RSSIs[TOTAL_APS];
    float TOFs[NUM_TOF_RESPONDERS];
    LOCATIONS label;
};

//Enablements -- regarding to system running mode
struct Enablements {
    static bool enable_training_model_on_host_machine = false;
    static SystemState currentSystemState             = SystemState::OFFLINE;
    static bool enable_SD_Card_backup 
}

// =================== Data Set Vector ===================
extern std::vector<Data> dataSet;


// =================== Utilities Functions Decleration ===================

/// Smoothing RSSI measurements
int applyEMA(int prevRSSI, int newRSSI)

/// @brief   User UI for choosing label 
/// @return  string describes specific label 
char* promptLocationSelection();

/// @brief  Map specefic label to string 
/// @param  label described by integer
/// @return String 
const char* labelToString(int label);

/// @brief   User UI for choosing System State 
/// @return  string describes specific label 
SystemState promptSystemState()

/// @brief  Map specefic label to string 
/// @param  state described by integer
/// @return String 
const char* systemStateToString(int state)


#endif // _UTILITIES_H_
