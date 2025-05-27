#include "fileUtils.h"

string getBaseDir() {
    return String("/") + systemStateToString(currentSystemState);
}

string getMetaFilePath() {
    return getSDBaseDir() + "meta_";
}

string getRSSIFilePath() {
    return getSDBaseDir() + "rssi_scan_data_" + CSV_SUFFIX;
}

string getTOFFilePath() {
    return getSDBaseDir() + "tof_scan_data_" + CSV_SUFFIX;
}

string getAccuracyFilePath() {
    return getSDBaseDir() + "location_accuracy_" + CSV_SUFFIX;
}
