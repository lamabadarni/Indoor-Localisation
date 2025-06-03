#include "data.h"
#include "fileUtils.h"
#include "esp_littlefs.h"

static std::vector<std::string> splitedString;
static void _splitBySeparator(const std::string &data, char separator);
static bool _deleteDirectory(const char* path);
static bool _createTofFile(void);
static bool _createRssiFile(void);
static bool _readLineFromFile(FILE* file, std::string& outLine);
static void _fromCSVRssiToVector(std::string &line);
//ward :: check - this funtion is not called at all
static void _fromCSVTofToVector(std::string &line);

void saveData(const RSSIData &scanData) { 
    if((BufferedData::scanner != STATICRSSI) && (BufferedData::scanner != BOTH)) return;
    rssiDataSet.push_back(scanData);
    LOG_DEBUG("DATA", "Buffered RSSI scan for label %d", scanData.label);
}

void saveData(const TOFData &scanData) {
    if(BufferedData::scanner != TOF_ && BufferedData::scanner != BOTH) return;
    tofDataSet.push_back(scanData);
    LOG_DEBUG("DATA", "Buffered TOF scan for label %d", scanData.label);
}

static bool saveRSSIScan(const RSSIData& row) {
    FILE* f = fopen(getRSSIFilePath().c_str(), "a");
    if (!f) {
        LOG_ERROR("FLASH", "Failed to write RSSI row.");
        return false;
    }

    for (int i = 0; i < NUMBER_OF_ANCHORS; ++i)
        fprintf(f, "%d,", row.RSSIs[i]);
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

    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i)
        fprintf(f, "%.2f,", row.TOFs[i]);
    fprintf(f, "%d\n", row.label);
    fclose(f);
    return true;
}

void doneCollectingData() {
    LOG_DEBUG("DATA", "Finishing scan collection. Buffered: RSSI=%d, TOF=%d",
             rssiDataSet.size(), tofDataSet.size());

    if (!BufferedData::scanner == NONE) {
        LOG_INFO("DATA", "bufferToCSV disabled — data not flushed.");
        return;
    }

    if (BufferedData::scanner == STATICRSSI) {
        for (const auto& row : rssiDataSet) {
            if (!saveRSSIScan(row)) LOG_ERROR("DATA", "Failed to write RSSI row.");
        }
    }

    else if(BufferedData::scanner == TOF_) {
        for (const auto& row : tofDataSet) {
            if (!saveTOFScan(row)) LOG_ERROR("DATA", "Failed to write TOF row.");
        }
    }

    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO("DATA", "Flush complete");

    //reset BufferedData
    BufferedData::scanner = NONE;
    BufferedData::lastN   = 0;
}

bool loadDataset() {
    bool ok = true;

    if (isRSSIActive()) {
        FILE* f = fopen(getRSSIFilePath().c_str(), "r");
        if (f) {
            LOG_INFO("FLASH", "Loading RSSI dataset...");
            std::string line;
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
            while(_readLineFromFile(f, line)) {
                _fromCSVRssiToVector(line);
            }
            fclose(f);
        } else {
            LOG_WARN("FLASH", "TOF file not found.");
            ok = false;
        }
    }

    return ok;
}


///Ward :: revisit logic here
// Reset storage for the current system state under LittleFS
bool formatStorage() {
    // Build directory name, e.g. "/STATIC_RSSI_TOF"
    std::string stateName = systemScannerModes[SystemSetup::currentSystemScannerMode];
    std::string baseDir = "/" + stateName;

    // 1) Delete any existing folder and its contents
    _deleteDirectory(baseDir.c_str());

    // 2) Create a fresh directory
    if (mkdir(baseDir.c_str(), 0777) != 0) {
        LOG_ERROR("Failed to mkdir: %s\n", baseDir.c_str());
        return false;
    }

    LOG_DEBUG("Created directory: %s\n", baseDir.c_str());
    
    switch ((SystemSetup::currentSystemScannerMode)){
        case(SystemScannerMode::STATIC_RSSI):
        {
            // Create the RSSI CSV file
            if (!_createRssiFile())
            {
                return false;
            }
        }
        break;

        case(SystemScannerMode::STATIC_RSSI_TOF):
        {
            // Create the RSSI CSV file
            if (!_createRssiFile())
            {
                return false;
            }

            // Create the TOF CSV file
            if (!_createTofFile())
            {
                return false;
            }
        }
        break;

        case (SystemScannerMode::TOF):
        {    
            // Create the TOF CSV file
            if (!_createTofFile())
            {
                return false;
            }
        }
        break;

        default :
        break;
    }

    LOG_DEBUG("Storage reset complete for state: %s\n", stateName.c_str());

    return true;
}

bool filterNonValidData(const bool validMap[LABELS_COUNT]) {
    bool success = true;

    if (isRSSIActive()) { // Lama: need to verify if the first line is the header
        FILE* in = fopen(getRSSIFilePath().c_str(), "r");
        //ward revisit here
        FILE* out = fopen(getBaseDir().c_str()  , "w");
        if (!in || !out) return false;
        // Write header to the temporary file
        for (int i = 1; i <= NUMBER_OF_ANCHORS; ++i) {
            fprintf(out, std::string(i) + "_rssi,");
        }
        fprintf("Label\n");

        std::string line;
        while (_readLineFromFile(in, line)) {
            _splitBySeparator(line, ',');
            if (splitedString.size() != NUMBER_OF_ANCHORS + 1) continue;

            Label label = static_cast<Label>(std::stoi(splitedString.back()));
            if (validMap[label]) fprintf(out, "%s", line);
        }

        fclose(in);
        fclose(out);
        remove(getRSSIFilePath().c_str());
        rename(RSSI_TMP, getRSSIFilePath());
        LOG_INFO("FLASH", "RSSI CSV cleaned.");
    }

    if (isTOFActive()) {
        FILE* in = fopen(getTOFFilePath().c_str(), "r");
        FILE* out = fopen(getBaseDir().c_str() + TOF_FILENAME + TMP_SUFFIX, "w");
        if (!in || !out) return false;
        // Write header to the temporary file
        for (int i = 1; i <= NUMBER_OF_RESPONDERS ; ++i) {
            fprintf(out, std::string(i) + "_tof,");
        }
        fprintf("Label\n");
        std::string line;
        while (_readLineFromFile(in, line)) {
            std::string str(line);
            _splitBySeparator(line, ',');
            if (splitedString.size() != NUMBER_OF_RESPONDERS + 1) continue;

            Label label = static_cast<Label>(std::stoi(splitedString.back()));
            if (validMap[label]) fprintf(out, "%s", line);
        }

        fclose(in);
        fclose(out);
        remove(getTOFFilePath().c_str());
        rename(getBaseDir().c_str() + TOF_FILENAME + TMP_SUFFIX, getTOFFilePath().c_str());
        LOG_INFO("FLASH", "TOF CSV cleaned.");
    }

    return success;
}

bool resetCSV(void) {
    bool success = true;

    if (isRSSIActive()) {
        FILE* f = fopen(getRSSIFilePath().c_str(), "w");
        if (!f) return false;
        for (int i = 1; i <= NUMBER_OF_ANCHORS; ++i)
            fprintf(f, "%d_rssi,", i);
        fprintf(f, "Location\n");
        fclose(f);
        LOG_INFO("FLASH", "RSSI CSV reset.");
    }

    if (isTOFActive()) {
        FILE* f = fopen(getTOFFilePath().c_str(), "w");
        if (!f) return false;
        for (int i = 1; i <= NUMBER_OF_RESPONDERS; ++i)
            fprintf(f, "%d_tof,", i);
        fprintf(f, "Location\n");
        fclose(f);
        LOG_INFO("FLASH", "TOF CSV reset.");
    }

    return success;
}

static void _fromCSVRssiToVector(std::string &line) {
    RSSIData row;
    _splitBySeparator(line, ',');
    if (splitedString.size() != LABELS_COUNT + 1) return;

    for (int i = 0; i < LABELS_COUNT; i++) {
        row.RSSIs[i] = std::stoi(splitedString[i]);
    }
    row.label = (Label)std::stoi(splitedString[LABELS_COUNT]);
    rssiDataSet.push_back(row);
}

static void _fromCSVTofToVector(std::string &line) {
    TOFData row;
    _splitBySeparator(line, ',');
    if (splitedString.size() != NUMBER_OF_RESPONDERS + 1) return;

    for (int i = 0; i < NUMBER_OF_RESPONDERS; i++) {
        row.TOFs[i] = std::stod(splitedString[i]);
    }
    row.label = Label(std::stoi(splitedString[NUMBER_OF_RESPONDERS]));
    tofDataSet.push_back(row);
}

static bool _readLineFromFile(FILE* file, std::string& outLine) {
    outLine.clear();
    if (!file) return false;

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), file)) {
        outLine += buffer;
        // If newline was found, line is complete
        if(strchr(buffer, '\n') != nullptr) break;
    }

    return !outLine.empty();
}

static bool _createRssiFile(void) {
    File f = fopen(rssiPath, "w");
    if (!f) {
        LOG_ERROR("DATA", "Failed to create RSSI CSV.");
        return false;
    }
    for (int i = 1; i <= NUMBER_OF_ANCHORS; ++i) {
        f.print(i);
        f.print("_rssi,");
    }
    f.println("Label");
    f.close();

    return true;
}

static bool _createTofFile(void) {
    File f = fopen(tofPath, "w");
    if (!f) {
        LOG_ERROR("Failed to create TOF CSV.");
        return false;
    }
    for (int j = 1; j <= NUMBER_OF_RESPONDERS; ++j) {
        f.print(j);
        f.print("_tof,");
    }
    f.println("Label");
    f.close();

    return true;
}

static void _splitBySeparator(const std::string &data, char separator) {
    splitedString.clear();            // assume splitedString is std::vector<std::string>
    size_t start = 0, pos = 0;
    while ((pos = data.find(separator, start)) != std::string::npos) {
        splitedString.push_back(data.substr(start, pos - start));
        start = pos + 1;
    }
    // final segment (or entire string if no separator found)
    splitedString.push_back(data.substr(start));
}

// Ward :: check function ( instead of remove(...))
static bool _deleteDirectory(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) return false;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // skip "." and ".."
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
            continue;
        }
        std::string fullPath = std::string(path) + "/" + entry->d_name;
        if (entry->d_type == DT_DIR) {
            // recurse into subdirectory
            deleteDirectory(fullPath.c_str());
        } else {
            // remove file
            unlink(fullPath.c_str());
        }
    }
    closedir(dir);
    // now remove the (now-empty) directory itself
    rmdir(path);
    return true;
}
