#include "fileUtils.h"

static constexpr char STATIC_RSSI_FILENAME[]  = "static";
static constexpr char DYNAMIC_RSSI_FILENAME[] = "dynamic";
static constexpr char TOF_FILENAME[]          = "tof";
static constexpr char LOG_FILENAME[]          = "logs";
static constexpr char TMP_SUFFIX[]            = ".tmp";
static constexpr char CSV_SUFFIX[]            = ".csv";
static constexpr char TXT_SUFFIX[]            = ".txt";

std::string getBaseDir() {
    return std::string(MOUNT_POINT) + "/Data";
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

std::string getLogFilePath(void) {
    return getBaseDir() + "/" + LOG_FILENAME + TXT_SUFFIX;
}
