#include "fileUtils.h"

// (1) We guarantee a trailing slash here
std::string getBaseDir() {
    return std::string(MOUNT_POINT) + "/" 
         + systemScannerModes[SystemSetup::currentSystemScannerMode]
         + "/";
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