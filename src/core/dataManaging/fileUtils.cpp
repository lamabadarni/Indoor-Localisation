#include "fileUtils.h"

static constexpr char STATIC_RSSI_FILENAME[]  = "static_rssi_data";
static constexpr char DYNAMIC_RSSI_FILENAME[] = "dynamic_rssi_data";
static constexpr char TOF_FILENAME[]          = "tof_data";
static constexpr char ACCURACY_FILENAME[]     = "acc_data";
static constexpr char LOG_FILENAME[]          = "indoor-localisation-log";
static constexpr char TMP_SUFFIX[]            = ".tmp";
static constexpr char CSV_SUFFIX[]            = ".csv";
static constexpr char TXT_SUFFIX[]            = ".txt";

std::string getBaseDir() {
    return std::string(MOUNT_POINT);
}

std::string getStaticRSSIFilePath(void) {
    return getBaseDir() + "/" + STATIC_RSSI_FILENAME + CSV_SUFFIX;
}

std::string getDynamicRSSIFilePath() {
    return getBaseDir() + "/" + DYNAMIC_RSSI_FILENAME + CSV_SUFFIX;
}

std::string getTOFFilePath(void) {
    return getBaseDir() + "/" + TOF_FILENAME + CSV_SUFFIX;
}

std::string getAccuracyFilePath(void) {
    return getBaseDir() + "/" + ACCURACY_FILENAME + CSV_SUFFIX;
}

std::string getLogFilePath(void) {
    return getBaseDir() + "/" + LOG_FILENAME + TXT_SUFFIX;
}
