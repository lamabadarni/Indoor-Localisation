
#include "fileUtils.h"

std::string getBaseDir() {
    return std::string("/") + systemScannerModes[SystemSetup::currentSystemScannerMode];
}

std::string getMetaFilePath() {
    return getBaseDir() + "meta_";
}

std::string getRSSIFilePath() {
    return getBaseDir() + "rssi_scan_data_" + CSV_SUFFIX;
}

std::string getTOFFilePath() {
    return getBaseDir() + "tof_scan_data_" + CSV_SUFFIX;
}

std::string getAccuracyFilePath() {
    return getBaseDir() + "location_accuracy_" + CSV_SUFFIX;
}