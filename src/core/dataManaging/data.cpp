#include "data.h"
#include "fileUtils.h"

#include <dirent.h>    // <-- needed for opendir/readdir
#include <unistd.h>    // <-- needed for unlink()/rmdir()
#include "sdCardInit.h"

static std::vector<std::string> splittedString;
static void _splitBySeparator(const std::string &data, char separator);
static bool _deleteDirectory(const char* path);
static bool _createTofFile(void);
static bool _createRssiFile(void);
static bool _readLineFromFile(FILE* file, std::string& outLine);
static void _fromCSVRssiToVector(std::string &line);
static void _fromCSVTofToVector(std::string &line);

//-----------------------------------------------------------------------------
// Public APIs
//-----------------------------------------------------------------------------

bool initDataBackup() {
    return initSDCard();
}
void saveData(const RSSIData &scanData) {
    if ((BufferedData::scanner != STATICRSSI) &&
        (BufferedData::scanner != BOTH)) {
        return;
    } // TODO: change to assert
    rssiDataSet.push_back(scanData);
    LOG_DEBUG("DATA", "Buffered RSSI scan for label %d", scanData.label);
}

void saveData(const TOFData &scanData) {
    if ((BufferedData::scanner != TOF_) &&
        (BufferedData::scanner != BOTH)) {
        return;
    } // TODO: change to assert
    tofDataSet.push_back(scanData);
    LOG_DEBUG("DATA", "Buffered TOF scan for label %d", scanData.label);
}

static bool saveRSSIScan(const RSSIData& row) {
    FILE* f = fopen(getRSSIFilePath().c_str(), "a");
    if (!f) {
        LOG_ERROR("FLASH", "Failed to write RSSI row.");
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
             (int)rssiDataSet.size(), (int)tofDataSet.size());

    if (BufferedData::scanner == NONE) {
        LOG_INFO("DATA", "bufferToCSV disabled — data not flushed.");
        return;
    } // TODO: change to assert

    if (BufferedData::scanner == STATICRSSI) {
        int datasetSize = rssiDataSet.size();
        
        for (int row = datasetSize - 1; row >= datasetSize - BufferedData::lastN ; row--) {
            if (!saveRSSIScan(rssiDataSet[row])) {
                LOG_ERROR("DATA", "Failed to write RSSI row.");
            }
        }
    }
    else if (BufferedData::scanner == TOF_) {
        int datasetSize = tofDataSet.size();

        for (int row = datasetSize - 1 ; row >= datasetSize - BufferedData::lastN ; row--) {
            if (!saveTOFScan(tofDataSet[row])) {
                LOG_ERROR("DATA", "Failed to write TOF row.");
            }
        }
    }

    LOG_INFO("DATA", "Flush complete");

    // reset BufferedData
    BufferedData::scanner = NONE;
    BufferedData::lastN   = 0;
}
bool loadDataset() {
    bool ok = true;

    // This logic is based on the version you provided in the previous turn
    if (isRSSIActive()) {
        FILE* f = fopen(getRSSIFilePath().c_str(), "r");
        if (f) {
            LOG_INFO("FLASH", "Loading RSSI dataset...");
            std::string line;

            // ---- FIX: Read and discard the header line first ----
            _readLineFromFile(f, line);

            // Now, loop through the rest of the file (the actual data)
            while (_readLineFromFile(f, line)) {
                _fromCSVRssiToVector(line);
            }
            fclose(f);
        } else {
            LOG_WARN("FLASH", "RSSI file not found.");
            ok = false;
        }
    }

    if (isTOFActive()) {
        FILE* f = fopen(getTOFFilePath().c_str(), "r");
        if (f) {
            LOG_INFO("FLASH", "Loading TOF dataset...");
            std::string line;

            // ---- FIX: Read and discard the header line first ----
            _readLineFromFile(f, line);

            // Now, loop through the rest of the file (the actual data)
            while (_readLineFromFile(f, line)) {
                _fromCSVTofToVector(line);
            }
            fclose(f);
        } else {
            LOG_WARN("FLASH", "TOF file not found.");
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

    // The rest of the function remains the same
    switch (SystemSetup::currentSystemScannerMode) {
        case SystemScannerMode::STATIC_RSSI:
            if (!_createRssiFile()) {
                return false;
            }
            break;
        case SystemScannerMode::STATIC_RSSI_TOF:
            if (!_createRssiFile() || !_createTofFile()) {
                return false;
            }
            break;
        case SystemScannerMode::TOF:
            if (!_createTofFile()) {
                return false;
            }
            break;
        default:
            break;
    }
    LOG_DEBUG("FLASH", "Storage reset complete for state: %s", systemScannerModes[SystemSetup::currentSystemScannerMode].c_str());
    return true;
}

bool filterNonValidData(const bool validMap[LABELS_COUNT]) {
    bool success = true;

    if (isRSSIActive()) {
        FILE* in = fopen(getRSSIFilePath().c_str(), "r");
        if (!in) return false;
        std::string tmpPath = getBaseDir() + RSSI_FILENAME + TMP_SUFFIX;
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

    if (isTOFActive()) {
        FILE* in = fopen(getTOFFilePath().c_str(), "r");
        if (!in) return false;
        std::string tmpPath = getBaseDir() + TOF_FILENAME + TMP_SUFFIX;
        FILE* out = fopen(tmpPath.c_str(), "w");
        if (!out) {
            fclose(in);
            return false;
        }

        // Write header
        for (int i = 1; i <= NUMBER_OF_RESPONDERS; ++i) {
            fprintf(out, "%d_tof,", i);
        }
        fprintf(out, "Label\n");

        std::string line;
        // first line = header
        _readLineFromFile(in, line);

        while (_readLineFromFile(in, line)) {
            _splitBySeparator(line, ',');
            if (splittedString.size() != (size_t)(NUMBER_OF_RESPONDERS + 1)) {
                continue;
            }
            Label label = static_cast<Label>(std::stoi(splittedString.back()));
            if (validMap[label]) {
                fprintf(out, "%s", line.c_str());
            }
        }
        fclose(in);
        fclose(out);
        remove(getTOFFilePath().c_str());
        rename(tmpPath.c_str(), getTOFFilePath().c_str());
        LOG_INFO("FLASH", "TOF CSV cleaned.");
    }

    return success;
}

bool resetCSV(void) {
    bool success = true;

    if (isRSSIActive()) {
        FILE* f = fopen(getRSSIFilePath().c_str(), "w");
        if (!f) return false;
        for (int i = 1; i <= NUMBER_OF_ANCHORS; ++i) {
            fprintf(f, "%d_rssi,", i);
        }
        fprintf(f, "Label\n");
        fclose(f);
        LOG_INFO("FLASH", "RSSI CSV reset.");
    }

    if (isTOFActive()) {
        FILE* f = fopen(getTOFFilePath().c_str(), "w");
        if (!f) return false;
        for (int i = 1; i <= NUMBER_OF_RESPONDERS; ++i) {
            fprintf(f, "%d_tof,", i);
        }
        fprintf(f, "Label\n");
        fclose(f);
        LOG_INFO("FLASH", "TOF CSV reset.");
    }

    return success;
}

//-----------------------------------------------------------------------------
// Private helpers
//-----------------------------------------------------------------------------

static void _fromCSVRssiToVector(std::string &line) {
    RSSIData row;
    _splitBySeparator(line, ',');
    if (splittedString.size() != NUMBER_OF_ANCHORS  + 1) {
        return;
    }
    for (int i = 0; i < NUMBER_OF_ANCHORS; i++) {
        row.RSSIs[i] = std::stoi(splittedString[i]);
    }
    row.label = static_cast<Label>(std::stoi(splittedString[NUMBER_OF_ANCHORS]));
    rssiDataSet.push_back(row);
}

static void _fromCSVTofToVector(std::string &line) {
    TOFData row;
    _splitBySeparator(line, ',');
    if (splittedString.size() != NUMBER_OF_RESPONDERS + 1) {
        return;
    }
    for (int i = 0; i < NUMBER_OF_RESPONDERS; i++) {
        row.TOFs[i] = std::stod(splittedString[i]);
    }
    row.label = static_cast<Label>(std::stoi(splittedString[NUMBER_OF_RESPONDERS]));
    tofDataSet.push_back(row);
}

static bool _readLineFromFile(FILE* file, std::string& outLine) {
    outLine.clear();
    if (!file) return false;

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), file)) {
        outLine += buffer;
        if (strchr(buffer, '\n') != nullptr) {
            break;
        }
    }
    return !outLine.empty();
}

static bool _createRssiFile(void) {
    FILE *f = fopen(getRSSIFilePath().c_str(), "w");
    if (!f) {
        LOG_ERROR("DATA", "Failed to create RSSI CSV.");
        return false;
    }
    for (int i = 1; i <= NUMBER_OF_ANCHORS; ++i) {
        fprintf(f, "%d_rssi,", i);
    }
    fprintf(f, "Label\n");
    fclose(f);
    return true;
}

static bool _createTofFile(void) {
    FILE *f = fopen(getTOFFilePath().c_str(), "w");
    if (!f) {
        LOG_ERROR("DATA", "Failed to create TOF CSV.");
        return false;
    }
    for (int j = 1; j <= NUMBER_OF_RESPONDERS; ++j) {
        fprintf(f, "%d_tof,", j);
    }
    fprintf(f, "Label\n");
    fclose(f);
    return true;
}

static void _splitBySeparator(const std::string &data, char separator) {
    splittedString.clear();
    size_t start = 0, pos = 0;
    while ((pos = data.find(separator, start)) != std::string::npos) {
        splittedString.push_back(data.substr(start, pos - start));
        start = pos + 1;
    }
    splittedString.push_back(data.substr(start));
}

static bool _deleteDirectory(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) return false;

    struct dirent* entry;
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