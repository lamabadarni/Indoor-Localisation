#ifndef UTILITIES_H
#define UTILITIES_H

#include "platform.h"
// ====================== Constants ======================

// == User UI ==
#define USER_PROMPTION_DELAY               (100)
#define DELAY_BETWEEN_PHASES               (500)

// == Coverage ==
#define MIN_ANCHORS_VISIBLE                (3)
#define MIN_APS_VISIBLE                    (3)
#define TOF_MIN_RESPONDERS_VISIBLE         (2)
#define MIN_AVERAGE_RSSI                   (-75)
#define TOF_MAX_AVERAGE_DISTANCE_CM        (600)

// == Scanner Defines ==
#define MAX_RETRIES_FOR_RSSI               (3)
#define RSSI_DEFAULT_VALUE                 (-100)
#define RSSI_SCAN_DELAY_MS                 (150)
#define MAC_ADDRESS_SIZE                   (6)

// == Static RSSI Scanner ==
#define STATIC_RSSI_SCAN_BATCH_SIZE        (15)
#define STATIC_RSSI_SCAN_SAMPLE_PER_BATCH  (3)
#define NUMBER_OF_ANCHORS                  (9)

// == Dynamic RSSI Scanner ==
#define DYNAMIC_RSSI_SCAN_BATCH_SIZE       (15)
#define DYNAMIC_RSSI_SCAN_SAMPLE_PER_BATCH (5)
#define NUMBER_OF_DYNAMIC_APS              (9)

// == TOF Scanner ==
#define TOF_SCAN_BATCH_SIZE                (15)
#define TOF_SCAN_SAMPLE_PER_BATCH          (2)
#define TOF_MAX_VALID_CM                   (500.0)  // Adjustable max valid TOF reading
#define TOF_DEFAULT_DISTANCE_CM            (1500.0)
#define TOF_SCAN_DELAY_MS                  (20)
#define NUMBER_OF_RESPONDERS               (1)

// == Validation == 
#define VALIDATION_MAX_ATTEMPTS            (3)
#define VALIDATION_PASS_THRESHOLD          (0.6)

// == Predection ==
#define ALPHA                              (0.7)
#define K_RSSI                             (4) 
#define K_TOF                              (2)
#define MIN_VALID_DATA_SET_SIZE            (K_RSSI * 10) 
#define MIN_DATA_PER_LABEL_SIZE            (K_RSSI * 3) 
#define PREDICTION_MAX_RETRIES             (2)
#define PREDICTION_SAMPLES                 (3)
#define PREDICTION_SAMPLES_THRESHOLD       (2)
#define MAX_LOG_LINES                      (7) // Adjust this value based on your display and font size

// Hardware pins
#define PIN_BTN_UP      GPIO_NUM_9
#define PIN_BTN_DOWN    GPIO_NUM_7
#define PIN_BTN_SELECT  GPIO_NUM_11

// I²C pins for the OLED
#define OLED_I2C_SDA    GPIO_NUM_3
#define OLED_I2C_SCL    GPIO_NUM_5

// ====================== Enums ======================

typedef enum  {
    ROOM_236 = 0,
    BETWEEN_ROOMS_234_236,
    BETWEEN_ROOM_232_236,
    ROOM_231,
    BETWEEN_ROOMS_231_236,
    NEAR_KITCHEN_BATHROOM,
    KITCHEN,
    MAIN_ENTRANCE,
    LOBBY,
    PRINTER,
    OFFICES_HALL_1,
    OFFICES_HALL_2,
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
    MODE_ANCHORS_RSSI_DIAGNOSTIC,
    MODE_APS_RSSI_DIAGNOSTIC,
    MODE_INIT_RESPONDER,
    SYSTEM_BOOT_MODES_NUM
} SystemBootMode;

typedef enum {
    STATIC_RSSI = 0,
    DYNAMIC_RSSI,
    TOF,
    SYSTEM_SCANNER_MODES_NUM
} SystemScannerMode;

typedef enum {
    STATICRSSI = 0,
    DYNAMICRSSI,
    TOfF,
    SYSTEM_PREDICTION_NODES_NUM
} SystemPredictionMode;

typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
} LogLevel;

typedef enum {
    OLED,
    SERIAL
} UI;

typedef enum {
    COVERAGE_GOOD,
    COVERAGE_WEAK,
    COVERAGE_NONE,
    COVERAGE_STATES_NUM
} Coverage;

// ====================== Data Structures ======================

struct StaticRSSIData {
    int RSSIs[NUMBER_OF_ANCHORS];  
    Label label;
};

struct DynamicRSSIData {
    int RSSIs[NUMBER_OF_DYNAMIC_APS];  
    Label label;
};

struct DynamicMacData {
    uint8_t macAddresses[NUMBER_OF_DYNAMIC_APS][MAC_ADDRESS_SIZE];
    Label label;
};

struct TOFData {
    double TOFs[NUMBER_OF_RESPONDERS];
    Label label;
};

struct SaveBufferedData {
   static SystemScannerMode scanner;
   static int lastN;
};

struct DeleteBufferedData {
   static SystemScannerMode scanner;
   static int lastN;
};

struct DataLoaded {
    static bool TOF;
    static bool Dynamic;
    static bool Static;
};

// ====================== System Setup ======================

struct SystemSetup {
    static SystemMode           currentSystemMode;
    static SystemScannerMode    currentSystemScannerMode;
    static SystemPredictionMode currentSystemPredictionMode;
    static SystemBootMode       currentSystemBootMode;
    static bool                 enableValidationPhase;
    static LogLevel             logLevel;
};

// ====================== Globals ======================

extern Label   currentLabel;
extern bool    shouldAbort;
extern double  accumulatedStaticRSSIs[NUMBER_OF_ANCHORS];
extern double  accumulatedDynamicRSSIs[NUMBER_OF_DYNAMIC_APS];
extern uint8_t accumulatedMacAddresses[NUMBER_OF_DYNAMIC_APS][MAC_ADDRESS_SIZE];
extern double  accumulatedTOFs[NUMBER_OF_RESPONDERS];
extern uint8_t responderMacs[NUMBER_OF_RESPONDERS][MAC_ADDRESS_SIZE];
extern double  tofAccuracy[LABELS_COUNT];
extern double  staticRSSIAccuracy[LABELS_COUNT];
extern double  dynamicRSSIAccuracy[LABELS_COUNT];
extern bool    validForPredection[LABELS_COUNT];
extern FILE*   logFile;

extern std::vector<StaticRSSIData>  staticRSSIDataSet;
extern std::vector<DynamicRSSIData> dynamicRSSIDataSet;
extern std::vector<DynamicMacData>  dynamicMacDataSet;
extern std::vector<TOFData>         tofDataSet;
extern std::vector<std::string>     log_buffer;

extern const std::string   anchorSSIDs[NUMBER_OF_ANCHORS];
extern const std::string   tofSSIDs[NUMBER_OF_RESPONDERS];
extern const std::string   labels[LABELS_COUNT];
extern const std::string   systemModes[MODES_NUM];
extern const std::string   systemScannerModes[SYSTEM_SCANNER_MODES_NUM];
extern const std::string   systemPredictionModes[SYSTEM_PREDICTION_NODES_NUM];
extern const std::string   systemBootModes[SYSTEM_BOOT_MODES_NUM];
extern const std::string   coverages[COVERAGE_STATES_NUM];
extern const UI            systemUI;
extern const std::string   responderMacStr;

// ====================== Utility Functions ======================

int applyEMA(int prevRSSI, int newRSSI);

int readIntFromUser();

char getCharFromUserWithTimeout(int timeoutMs);

bool isStaticRSSIActiveForPrediction();

bool isDynamicRSSIActiveForPrediction();

bool isTOFActiveForPrediction();

bool isDataLoadedForScan();

bool isDataLoadedForPrediction();

void resetStaticRssiBuffer();

void resetDynamicRssiBuffer();

void resetTOFScanBuffer();

void setValidForPredection();

float getAccuracyForValidation();


std::vector<std::string> arrayToVector(const std::string arr[], int size);


#endif // UTILITIES_H