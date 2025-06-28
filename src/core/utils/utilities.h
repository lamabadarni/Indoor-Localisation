#ifndef UTILITIES_H
#define UTILITIES_H

#include "platform.h"
// ====================== Constants ======================

// == User UI ==
#define USER_PROMPTION_DELAY         (100)
#define DELAY_BETWEEN_PHASES         (500)

// == Coverage ==
#define MIN_ANCHORS_VISIBLE          (3)
#define MIN_AVERAGE_RSSI             (-75)
#define TOF_MIN_RESPONDERS_VISIBLE   (2)
#define TOF_MAX_AVERAGE_DISTANCE_CM  (600)

// == RSSI Scanner ==
#define RSSI_SCAN_BATCH_SIZE         (15)
#define RSSI_SCAN_SAMPLE_PER_BATCH   (3)
#define RSSI_DEFAULT_VALUE           (-100)
#define RSSI_SCAN_DELAY_MS           (150)
#define NUMBER_OF_ANCHORS            (9)
#define MAX_RETRIES_FOR_RSSI         (3)

// == TOF Scanner ==
#define TOF_NUMBER_OF_MAC_BYTES      (6)
#define TOF_SCAN_BATCH_SIZE          (15)
#define TOF_SCAN_SAMPLE_PER_BATCH    (2)
#define TOF_MAX_VALID_CM             (500.0)  // Adjustable max valid TOF reading
#define TOF_DEFAULT_DISTANCE_CM      (1500.0)
#define TOF_SCAN_DELAY_MS            (20)
#define NUMBER_OF_RESPONDERS         (4)

// == Validation == 
#define VALIDATION_MAX_ATTEMPTS      (5)
#define VALIDATION_PASS_THRESHOLD    (0.6)

// == Predection ==
#define ALPHA                        (0.7)
#define K_RSSI                       (4) 
#define K_TOF                        (2)
#define MIN_VALID_DATA_SET_SIZE      (K_RSSI * 10) 
#define MIN_DATA_PER_LABEL_SIZE      (K_RSSI * 3) 
#define PREDICTION_MAX_RETRIES       (2)
#define PREDICTION_MAX_LABEL_FAILURE (5)
#define PREDICTION_SAMPLES           (5)
#define PREDICTION_SAMPLES_THRESHOLD (3)

// ====================== Enums ======================

typedef enum  {
    NEAR_ROOM_234 = 0,
    BETWEEN_ROOMS_234_236,
    ROOM_236,
    NEAR_ROOM_232,
    ROOM_231,
    BETWEEN_ROOMS_231_236,
    NEAR_BATHROOM,
    NEAR_KITCHEN,
    KITCHEN,
    NEAR_ROOM_230,
    MAIN_ENTRANCE,
    LOBBY,
    PRINTER,
    OFFICES_HALL,
    MAIN_EXIT,
    BALCONY_ENTRANCE,
    LABELS_COUNT
} Label;

typedef enum {
    MODE_SYSTEM_BOOT ,
    MODE_SCANNING_SESSION,
    MODE_PREDICTION_SESSION,
    MODE_FULL_SESSION,
    MODES_NUM
} SystemMode;

typedef enum {
    MODE_TOF_DIAGNOSTIC = 0,
    MODE_COLLECT_TOF_RESPONDERS_MAC,
    MODE_RSSI_DIAGNOSTIC,
    MODE_RESTORE_DATA_TEST,
    SYSTEM_BOOT_MODES_NUM
} SystemBootMode;

typedef enum {
    STATIC_RSSI = 0,
    TOF,
    STATIC_RSSI_TOF,
    SYSTEM_SCANNER_MODES_NUM
} SystemScannerMode;

typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
} LogLevel;

typedef enum {
    NONE,
    STATICRSSI,
    TOF_,
    BOTH
} ScannerFlag; 

// ====================== Data Structures ======================

struct RSSIData {
    int RSSIs[NUMBER_OF_ANCHORS];  
    Label label;
};

struct TOFData {
    double TOFs[NUMBER_OF_RESPONDERS];
    Label label;
};

struct BufferedData {
   static ScannerFlag scanner;
   static int lastN;
};

// ====================== System Setup ======================

struct SystemSetup {
    static SystemMode         currentSystemMode;
    static SystemScannerMode  currentSystemScannerMode;
    static SystemBootMode     currentSystemBootMode;
    static bool               enableBackup;
    static bool               enableRestore;
    static bool               enableValidationPhase;
    static LogLevel           logLevel;
};

// ====================== Globals ======================

extern Label  currentLabel;
extern bool   shouldAbort;
extern bool   reconfigure;

extern bool    reuseFromMemory[LABELS_COUNT];
extern bool    validForPredection[LABELS_COUNT];
extern double  accumulatedRSSIs[NUMBER_OF_ANCHORS];
extern double  accumulatedTOFs[NUMBER_OF_RESPONDERS];
extern uint8_t responderMacs[NUMBER_OF_RESPONDERS][TOF_NUMBER_OF_MAC_BYTES];
extern double  tofAccuracy[LABELS_COUNT];
extern double  rssiAccuracy[LABELS_COUNT];

extern std::vector<RSSIData>  rssiDataSet;
extern std::vector<TOFData>   tofDataSet;

extern const std::string   anchorSSIDs[NUMBER_OF_ANCHORS];
extern const std::string   tofSSIDs[NUMBER_OF_RESPONDERS];
extern const std::string   labels[LABELS_COUNT];
extern const std::string   systemModes[MODES_NUM];
extern const std::string   systemScannerModes[SYSTEM_SCANNER_MODES_NUM];
extern const std::string   systemBootMode[SYSTEM_BOOT_MODES_NUM];
extern const std::string   systemStates[SYSTEM_SCANNER_MODES_NUM];

// ====================== Utility Functions ======================

/**
 * @brief Apply EMA (exponential moving average) to RSSI.
 */
int applyEMA(int prevRSSI, int newRSSI);

char readCharFromUser();

int readIntFromUser();

char getCharFromUserWithTimeout(int timeoutMs);

bool isRSSIActive();

bool isTOFActive();

void resetRssiBuffer();

void resetTOFScanBuffer();

void setValidForPredection();

float getAccuracy();

#endif // UTILITIES_H