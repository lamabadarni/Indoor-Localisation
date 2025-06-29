#include "fileUtils.h"

// (1) We guarantee a trailing slash here
std::string getBaseDir() {
    return std::string(MOUNT_POINT) + "/" 
         + systemScannerModes[SystemSetup::currentSystemScannerMode];
}

std::string getMetaFilePath() {
    return getBaseDir()+ "/" + "meta_";
}

std::string getRSSIFilePath() {
    return getBaseDir() + "/" + RSSI_FILENAME + CSV_SUFFIX;
}

std::string getTOFFilePath() {
    return getBaseDir() + "/" + TOF_FILENAME + CSV_SUFFIX;
}

std::string getAccuracyFilePath() {
    return getBaseDir() + "/" + ACCURACY_FILENAME + CSV_SUFFIX;
}