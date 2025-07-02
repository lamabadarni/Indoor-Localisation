#include "fileUtils.h"

std::string getBaseDir() {
    return std::string(MOUNT_POINT);
}

std::string getStaticRSSIFilePath() {
    return getBaseDir() + "/" + STATIC_RSSI_FILENAME + CSV_SUFFIX;
}

std::string getRSSIFilePath() {
  //  return getBaseDir() + "/" + RSSI_FILENAME + CSV_SUFFIX;
}

std::string getTOFFilePath() {
    return getBaseDir() + "/" + TOF_FILENAME + CSV_SUFFIX;
}

std::string getAccuracyFilePath() {
    return getBaseDir() + "/" + ACCURACY_FILENAME + CSV_SUFFIX;
}