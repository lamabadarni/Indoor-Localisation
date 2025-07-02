#include "data.h"
#include "fileUtils.h"

#include <dirent.h>    // <-- needed for opendir/readdir
#include <unistd.h>    // <-- needed for unlink()/rmdir()
#include "sdCardInit.h"
/*
static std::vector<std::string> splittedString;
static void _splitBySeparator(const std::string &data, char separator);
static bool _deleteDirectory(const char* path);
static bool _createFileWithHeader(const std::string& filePath, const std::string& header);
static bool _readLineFromFile(FILE* file, std::string& outLine);
static bool _fromCSVStaticRssiToVector(std::string &line);
static bool _fromCSVDynamicRssiToVector(std::string& line);
static bool _fromCSVTofToVector(std::string &line);

//-----------------------------------------------------------------------------
// Public APIs
//-----------------------------------------------------------------------------

bool initDataBackup() {
    return initSDCard();
}
void saveData(const StaticRSSIData &scanData) {
    if (SaveBufferedData::scanner != STATIC_RSSI) {
        return;
    } // TODO: change to assert
    staticRSSIDataSet.push_back(scanData);
    LOG_DEBUG("DATA", "Buffered RSSI scan for label %d", scanData.label);
}

void saveData(const TOFData &scanData) {
    if (SaveBufferedData::scanner != TOF) {
        return;
    } // TODO: change to assert
    tofDataSet.push_back(scanData);
    LOG_DEBUG("DATA", "Buffered TOF scan for label %d", scanData.label);
}

static bool saveRSSIScan(const StaticRSSIData& row) {
    FILE* f = fopen(getRSSIFilePath().c_str(), "a");
    if (!f) {
        LOG_ERROR("FLASH", "Failed to write static RSSI row.");
        return false;
    }

    for (int i = 0; i < NUMBER_OF_ANCHORS; ++i) {
        fprintf(f, "%d,", row.RSSIs[i]);
    }

    fprintf(f, "%d\n", row.label);
    fclose(f);

    return true;
}

static bool saveTOFScan(const TOFData& row) {
    FILE* f = fopen(getTOFFilePath().c_str(), "a");

    if (!f) {
        LOG_ERROR("FLASH", "Failed to write TOF row.");
        return false;
    }

    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        fprintf(f, "%.2f,", row.TOFs[i]);
    }

    fprintf(f, "%d\n", row.label);
    fclose(f);

    return true;
}

// save buffered data..
void doneCollectingData() {
    LOG_DEBUG("DATA", "Finishing scan collection. Buffered: RSSI=%d, TOF=%d",
             (int)staticRSSIDataSet.size(), (int)tofDataSet.size());

    if (SaveBufferedData::scanner == SYSTEM_SCANNER_MODES_NUM) {
        LOG_INFO("DATA", "bufferToCSV disabled — data not flushed.");
        return;
    } // TODO: change to assert

    if (SaveBufferedData::scanner == STATIC_RSSI) {
        int datasetSize = staticRSSIDataSet.size();
        
        for (int row = datasetSize - 1; row >= datasetSize - SaveBufferedData::lastN ; row--) {
            if (!saveRSSIScan(staticRSSIDataSet[row])) {
                LOG_ERROR("DATA", "Failed to write RSSI row.");
            }
        }
    }
    else if (SaveBufferedData::scanner == TOF) {
    else if (SaveBufferedData::scanner == TOF) {
        int datasetSize = tofDataSet.size();

        for (int row = datasetSize - 1 ; row >= datasetSize - SaveBufferedData::lastN ; row--) {
        for (int row = datasetSize - 1 ; row >= datasetSize - SaveBufferedData::lastN ; row--) {
            if (!saveTOFScan(tofDataSet[row])) {
                LOG_ERROR("DATA", "Failed to write TOF row.");
            }
        }
    }
    else if (SaveBufferedData::scanner == DYNAMIC_RSSI) {
        int datasetSize = dynamicRSSIDataSet.size();
        int macDatasetSize = dynamicMacDataSet.size();

        for (int row = datasetSize ; row >= datasetSize - SaveBufferedData::lastN ; row--) {
            if (!saveDynamicRSSIScan(dynamicMacDataSet[row], dynamicRSSIDataSet[row])) {
                LOG_ERROR("DATA", "Failed to write TOF row.");
            }
        }
    }

    LOG_INFO("DATA", "Flush complete");

    // reset BufferedData
    SaveBufferedData::scanner = SYSTEM_SCANNER_MODES_NUM;
    SaveBufferedData::lastN   = 0;
}
bool loadDataset(void) {
    bool ok = true;
    SystemScannerMode currentState = SystemSetup::currentSystemScannerMode;

    if (currentState == STATIC_RSSI) {
        FILE* f = fopen(getStaticRSSIFilePath().c_str(), "r");
        int validData = 0;

    // This logic is based on the version you provided in the previous turn
    if (isStaticRSSIActiveForPrediction()) {
        FILE* f = fopen(getRSSIFilePath().c_str(), "r");
        if (f) {
            LOG_INFO("DATA", "Loading RSSI dataset...");

            std::string line;

            // Read and discard the header line first
            _readLineFromFile(f, line);

            // Now, loop through the rest of the file (the actual data)
            while (_readLineFromFile(f, line)) {
                if(!_fromCSVStaticRssiToVector(line)) {
                    LOG_ERROR("FLASH", "Dynamic RSSI Data is not in the correct format");
                }
                else {
                    validData++;
                }
            }

            fclose(f);
        } else {
            LOG_WARN("DATA", "RSSI file not found.");

            ok = false;
        }
    }

    if (isTOFActiveForPrediction()) {
        FILE* f = fopen(getTOFFilePath().c_str(), "r");
        int validData = 0;

        if (f) {
            LOG_INFO("DATA", "Loading TOF dataset...");

            std::string line;

            // Read and discard the header line first
            _readLineFromFile(f, line);

            // Now, loop through the rest of the file (the actual data)
            while (_readLineFromFile(f, line)) {
                if (!_fromCSVTofToVector(line)) {
                    LOG_ERROR("FLASH", "TOF Data is not in the correct format");
                }
                else {
                    validData++;
                }
            }

            fclose(f);
        } else {
            LOG_WARN("DATA", "TOF file not found.");

            ok = false;
        }
    }
    else if (currentState == DYNAMIC_RSSI) {
        FILE* f = fopen(getDynamicRSSIFilePath().c_str(), "r");
        bool validData = 0;

        if (f) {
            LOG_INFO("DATA", "Loading RSSI dataset...");

            std::string line;

            // Read and discard the header line first
            _readLineFromFile(f, line);

            // Now, loop through the rest of the file (the actual data)
            while (_readLineFromFile(f, line)) {
                if (!_fromCSVDynamicRssiToVector(line)) {
                    LOG_ERROR("DATA", "Dynamic RSSI Data is not in the correct format");
                }
                else {
                    validData++;
                }
            }

            fclose(f);
        } else {
            LOG_WARN("FLASH", "Could not open Dynamic RSSI file.");

            ok = false;
        }
    }

    return ok;
}

bool formatStorage(void) {
    // Use the corrected getBaseDir() function to get the full path
    std::string baseDir = getBaseDir();

    // 1) Recursively delete anything under baseDir
    _deleteDirectory(baseDir.c_str());

    // 2) Make a fresh directory
    if (mkdir(baseDir.c_str(), 0777) != 0) {
        LOG_ERROR("FLASH", "Failed to mkdir: %s", baseDir.c_str());
        return false;
    }

    LOG_DEBUG("FLASH", "Created directory: %s", baseDir.c_str());

    return createCSV();
}

bool createCSV(void) {
    std::string header, colHeader, filePath;
    int numOfCols;

    switch (SystemSetup::currentSystemScannerMode) {
        case SystemScannerMode::STATIC_RSSI:
            colHeader = "rssi_";
            numOfCols = NUMBER_OF_ANCHORS;
            filePath = getStaticRSSIFilePath();
            break;
        case SystemScannerMode::TOF:
            colHeader = "tof_";
            numOfCols = NUMBER_OF_RESPONDERS;
            filePath = getDynamicRSSIFilePath();
            break;
    }
    LOG_DEBUG("FLASH", "Storage reset complete for state: %s", systemScannerModes[SystemSetup::currentSystemScannerMode].c_str());
    return true;
}

bool filterNonValidData(const bool validMap[LABELS_COUNT]) {
    bool success = true;

    if (isStaticRSSIActiveForPrediction()) {
        FILE* in = fopen(getRSSIFilePath().c_str(), "r");
        if (!in) return false;
        std::string tmpPath = getBaseDir()  + TMP_SUFFIX;
        FILE* out = fopen(tmpPath.c_str(), "w");
        if (!out) {
            fclose(in);
            return false;
        }

        // Write header
        for (int i = 1; i <= NUMBER_OF_ANCHORS; ++i) {
            fprintf(out, "%d_rssi,", i);
        }
        fprintf(out, "Label\n");

        std::string line;
        // consume first line (header)
        _readLineFromFile(in, line);

        while (_readLineFromFile(in, line)) {
            _splitBySeparator(line, ',');
            if (splittedString.size() != (size_t)(NUMBER_OF_ANCHORS + 1)) {
                continue;
            }
            Label label = static_cast<Label>(std::stoi(splittedString.back()));
            if (validMap[label]) {
                fprintf(out, "%s", line.c_str());
            }
        }
        fclose(in);
        fclose(out);
        remove(getRSSIFilePath().c_str());
        rename(tmpPath.c_str(), getRSSIFilePath().c_str());
        LOG_INFO("FLASH", "RSSI CSV cleaned.");
    }

    header += "Label\n";

    return _createFileWithHeader(filePath, header);
}

//-----------------------------------------------------------------------------
// Private helpers
//-----------------------------------------------------------------------------

static bool _fromCSVStaticRssiToVector(std::string &line) {
    StaticRSSIData row;

    if (splittedString.size() != NUMBER_OF_ANCHORS + 1) {
        return false;
    }

    _splitBySeparator(line, ',');

    for (int i = 0; i < NUMBER_OF_ANCHORS; i++) {
        row.RSSIs[i] = std::stoi(splittedString[i]);
    }

    row.label = static_cast<Label>(std::stoi(splittedString[NUMBER_OF_ANCHORS]));

    staticRSSIDataSet.push_back(row);

    return true;
}

static bool _fromCSVTofToVector(std::string &line) {
    TOFData row;

    if (splittedString.size() != NUMBER_OF_RESPONDERS + 1) {
        return false;
    }

    _splitBySeparator(line, ',');

    for (int i = 0; i < NUMBER_OF_RESPONDERS; i++) {
        row.TOFs[i] = std::stod(splittedString[i]);
    }

    row.label = static_cast<Label>(std::stoi(splittedString[NUMBER_OF_RESPONDERS]));

    tofDataSet.push_back(row);

    return true;
}

static bool __hexaCharToHexaInt(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';          // '0' to '9' -> 0 to 9
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;     // 'A' to 'F' -> 10 to 15
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;     // 'a' to 'f' -> 10 to 15
    }

    return -1;  
}

static bool _fromCSVDynamicRssiToVector(std::string& line) {
    DynamicMacData macAddrData;
    DynamicRSSIData rssiData;

    if (splittedString.size() != NUMBER_OF_DYNAMIC_APS + 1) {
        return false;
    }

    _splitBySeparator(line, ',');

    for (int i = 0 ; i < NUMBER_OF_DYNAMIC_APS ; i++) {
        rssiData.RSSIs[i] = std::stoi(splittedString[i].substr(2 * MAC_ADDRESS_SIZE + 1));

        for (int j = 0, k = 0 ; j < MAC_ADDRESS_SIZE ; j++) {
            macAddrData.macAddresses[i][j + 1] = 0;

            macAddrData.macAddresses[i][j] |= __hexaCharToHexaInt(splittedString[i][3 * j + 1]) |
                                              (__hexaCharToHexaInt(splittedString[i][3 * j]) << 4);
        }
    }

    rssiData.label = static_cast<Label>(std::stoi(splittedString[NUMBER_OF_DYNAMIC_APS]));
    macAddrData.label = rssiData.label;

    dynamicRSSIDataSet.push_back(rssiData);
    dynamicMacDataSet.push_back(macAddrData);

    return true;
}

static bool _readLineFromFile(FILE* file, std::string& outLine) {
    outLine.clear();

    if (!file) {
        return false;
    }

    char buffer[128];

    while (fgets(buffer, sizeof(buffer), file)) {
        outLine += buffer;

        if (strchr(buffer, '\n') != nullptr) {
            break;
        }
    }

    return !outLine.empty();
}

static bool _createFileWithHeader(const std::string& filePath, const std::string& header) {
    FILE *f = fopen(filePath.c_str(), "w");

    if (!f) {
        LOG_ERROR("DATA", "Failed to create TOF CSV.");
        return false;
    }

    fprintf(f, header.c_str());

    return false;
}

static void _splitBySeparator(const std::string &data, char separator) {
    size_t start = 0, pos = 0;

    splittedString.clear();

    while ((pos = data.find(separator, start)) != std::string::npos) {
        splittedString.push_back(data.substr(start, pos - start));
        start = pos + 1;
    }

    splittedString.push_back(data.substr(start));
}

static bool _deleteDirectory(const char* path) {
    DIR* dir = opendir(path);
    struct dirent* entry;

    if (!dir) return false;

    while ((entry = readdir(dir)) != nullptr) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
            continue;
        }
        std::string fullPath = std::string(path) + "/" + entry->d_name;
        if (entry->d_type == DT_DIR) {
            _deleteDirectory(fullPath.c_str());
        } else {
            unlink(fullPath.c_str());
        }
    }

    closedir(dir);
    rmdir(path);

    return true;
}

*/