#ifndef UTILITIES_H
#define UTILITIES_H

#include "platform.h"
// ====================== Constants ======================

// == User UI ==
#define USER_PROMPTION_DELAY         (100)
#define DELAY_BETWEEN_PHASES         (500)

// == Coverage ==
#define MIN_ANCHORS_VISIBLE          (3)
#define MIN_APS_VISIBLE              (3)
#define TOF_MIN_RESPONDERS_VISIBLE   (2)
#define MIN_AVERAGE_RSSI             (-75)
#define TOF_MAX_AVERAGE_DISTANCE_CM  (600)

// == Scanner Defines ==
#define MAX_RETRIES_FOR_RSSI         (3)
#define RSSI_DEFAULT_VALUE           (-100)
#define RSSI_SCAN_DELAY_MS           (150)
#define MAC_ADDRESS_SIZE             (6)

// == Static RSSI Scanner ==
#define STATIC_RSSI_SCAN_BATCH_SIZE         (15)
#define STATIC_RSSI_SCAN_SAMPLE_PER_BATCH   (3)
#define NUMBER_OF_ANCHORS            (9)

// == Dynamic RSSI Scanner ==
#define DYNAMIC_RSSI_SCAN_BATCH_SIZE        (15)
#define DYNAMIC_RSSI_SCAN_SAMPLE_PER_BATCH  (3)
#define NUMBER_OF_DYNAMIC_APS               (9)

// == TOF Scanner ==
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
#define PREDICTION_SAMPLES           (3)
#define PREDICTION_SAMPLES_THRESHOLD (2)
#define MAX_LOG_LINES                (7) // Adjust this value based on your display and font size

// Hardware pins
#define PIN_BTN_UP      GPIO_NUM_9
#define PIN_BTN_DOWN    GPIO_NUM_12
#define PIN_BTN_SELECT  GPIO_NUM_11

// I²C pins for the OLED
#define OLED_I2C_SDA    GPIO_NUM_33
#define OLED_I2C_SCL    GPIO_NUM_18

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
    MODE_ANCHORS_RSSI_DIAGNOSTIC,
    MODE_APS_RSSI_DIAGNOSTIC,
    MODE_INITIATOR_RESPONDER_TEST,
    MODE_RESTORE_BACKUP_DATA_TEST,
    SYSTEM_BOOT_MODES_NUM
} SystemBootMode;

typedef enum {
    STATIC_RSSI = 0,
    DYNAMIC_RSSI,
    TOF,
    SYSTEM_SCANNER_MODES_NUM
} SystemScannerMode;

typedef enum {
    STATIC_RSSI = 0,
    DYNAMIC_RSSI,
    TOF,
    STATIC_RSSI_DYNAMIC_RSSI,
    STATIC_RSSI_TOF,
    DYNAMIC_RSSI_TOF,
    STATIC_RSSI_DYNAMIC_RSSI_TOF,
    SYSTEM_PREDICTION_NODES_NUM
} SystemPredictionMode;

typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
} LogLevel;

typedef enum {
    OLED,
    SERIAL
} UI;

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
extern double  accumulatedTOFs[NUMBER_OF_RESPONDERS];
extern uint8_t responderMacs[NUMBER_OF_RESPONDERS][MAC_ADDRESS_SIZE];
extern double  tofAccuracy[LABELS_COUNT];
extern double  staticRSSIAccuracy[LABELS_COUNT];
extern double  dynamicRSSIAccuracy[LABELS_COUNT];
extern bool    validForPredection[LABELS_COUNT];

extern std::vector<StaticRSSIData>  staticRSSIDataSet;
extern std::vector<DynamicRSSIData> dynamicRSSIDataSet;
extern std::vector<DynamicMacData>  dynamicMacDataSet;
extern std::vector<TOFData>         tofDataSet;

extern const std::string   anchorSSIDs[NUMBER_OF_ANCHORS];
extern const std::string   tofSSIDs[NUMBER_OF_RESPONDERS];
extern const std::string   labels[LABELS_COUNT];
extern const std::string   systemModes[MODES_NUM];
extern const std::string   systemScannerModes[SYSTEM_SCANNER_MODES_NUM];
extern const std::string   systemPredictionModes[SYSTEM_SCANNER_MODES_NUM];
extern const std::string   systemBootModes[SYSTEM_BOOT_MODES_NUM];
extern const UI            systemUI;

// ====================== Utility Functions ======================

int applyEMA(int prevRSSI, int newRSSI);

char readCharFromUserSerial();

int readIntFromUser();

char getCharFromUserWithTimeout(int timeoutMs);

bool isStaticRSSIActiveForPrediction();

bool isDynamicRSSIActiveForPrediction();

bool isTOFActiveForPrediction();

void resetStaticRssiBuffer();

void resetDynamicRssiBuffer();

void resetTOFScanBuffer();

void setValidForPredection();

float getAccuracy();

std::vector<std::string> arrayToVector(const std::string arr[], int size);


#endif // UTILITIES_H