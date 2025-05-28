
#include "data.h"
#include "fileUtils.h"
#include "../ui/logger.h"

void saveData(const RSSIData &scanData) { // Lama: change to const RSSIData& scanData
    rssiDataSet.push_back(scanData);
    LOG_DEBUG("DATA", "Buffered RSSI scan for label %d", scanData.label);
}

void saveData(const TOFData &scanData) {
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

    if (!BufferedData::rssiData && !BufferedData::tofData) {
        LOG_INFO("DATA", "bufferToCSV disabled — data not flushed.");
        return;
    }

    if (BufferedData::rssiData) {
        for (const auto& row : rssiDataSet) {
            if (!saveRSSIScan(row)) LOG_ERROR("DATA", "Failed to write RSSI row.");
        }
    }

    delay_ms(USER_PROMPTION_DELAY);

    if(BufferedData::tofData) {
        for (const auto& row : tofDataSet) {
            if (!saveTOFScan(row)) LOG_ERROR("DATA", "Failed to write TOF row.");
        }
    }

    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO("DATA", "Flush complete");
}

static std::vector<std::string> splitedString;

static void fromCSVRssiToVector(std::string &line) {
    RSSIData row;
    splitBySeparator(line, ',');
    if (splitedString.size() != LABELS_COUNT + 1) return;

    for (int i = 0; i < LABELS_COUNT; i++) {
        row.RSSIs[i] = splitedString[i].toInt();
    }
    row.label = splitedString[LABELS_COUNT].toInt();
    rssiDataSet.push_back(row);
}

static void fromCSVTofToVector(string &line) {
    TOFData row;
    splitBySeparator(line, ',');
    if (splitedString.size() != NUMBER_OF_RESPONDERS + 1) return;

    for (int i = 0; i < NUMBER_OF_RESPONDERS; i++) {
        row.TOFs[i] = splitedString[i].toDouble();
    }
    row.label = splitedString[NUMBER_OF_RESPONDERS].toInt();
    tofDataSet.push_back(row);
}

static void splitBySeparator(string data, char separator) {
    splitedString.clear();
    int sepIndex;
    while ((sepIndex = data.indexOf(comma)) != -1) {
        splitedString.push_back(data.substring(0, sepIndex));
        data = data.substring(sepIndex + 1);
    }
    splitedString.push_back(data);
}

static bool readLineFromFile(FILE* file, std::string& outLine) {
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

bool loadDataset() {
    bool ok = true;

    if (isRSSIActive()) {
        FILE* f = fopen(getRSSIFilePath(), "r");
        if (f) {
            LOG_INFO("FLASH", "Loading RSSI dataset...");
            std::string line;
            while (readLineFromFile(f, line)) {
                fromCSVRssiToVector(line);
            }
            fclose(f);
        } else {
            LOG_WARN("FLASH", "RSSI file not found.");
            ok = false;
        }
    }

    if (isTOFActive(currentSystemState)) {
        FILE* f = fopen(getTOFFilePath(), "r");
        if (f) {
            LOG_INFO("FLASH", "Loading TOF dataset...");
            std::string line;
            while(readLineFromFile(f, line)) {
                fromCSVRssiToVector(line);
            }
            fclose(f);
        } else {
            LOG_WARN("FLASH", "TOF file not found.");
            ok = false;
        }
    }

    return ok;
}

static bool deleteDirectory(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) {
        // nothing to delete or doesn't exist
        return true;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        String name = entry->d_name;
        if (name == "." || name == "..") continue;
        String fullPath = String(path) + "/" + name;

        if (entry->d_type == DT_DIR) {
            // Recurse into sub-directory
            deleteDirectory(fullPath.c_str());
            rmdir(fullPath.c_str());
        } else {
            LittleFS.remove(fullPath);
        }
    }
    closedir(dir);
    // Finally remove the (now empty) directory itself
    return (rmdir(path) == 0);
}


static bool _createRssiFile(void)
{
    File f = LittleFS.open(rssiPath, "w");
    if (!f) {
        LOG_ERROR("Failed to create RSSI CSV.");
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

static bool _createTofFile(void)
{
    File f = LittleFS.open(tofPath, "w");
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

// Reset storage for the current system state under LittleFS
bool formatStorage() {
    // Build directory name, e.g. "/STATIC_RSSI_TOF"
    String stateName = systemStateToString(SystemSetup::currentSystemState);
    String baseDir = "/" + stateName;

    // 1) Delete any existing folder and its contents
    deleteDirectory(baseDir.c_str());

    // 2) Create a fresh directory
    if (mkdir(baseDir.c_str(), 0777) != 0) {
        LOG_ERROR("Failed to mkdir: %s\n", baseDir.c_str());
        return false;
    }

    LOG_DEBUG("Created directory: %s\n", baseDir.c_str());
    
    switch ((SystemSetup::currentSystemState)){
        case(SystemState::STATIC_RSSI):
        {
            // Create the RSSI CSV file
            if (!_createRssiFile())
            {
                return false;
            }
        }
        break;

        case(SystemState::STATIC_RSSI_TOF):
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

        case (SystemState::STATIC_TOF):
        {    
            // Create the TOF CSV file
            if (!_createTofFile())
            {
                return false;
            }
        }
        break;
    }

    LOG_DEBUG("Storage reset complete for state: %s\n", stateName.c_str());

    return true;
}

bool filterNonValidData(const bool validMap[LABELS_COUNT]) {
    bool success = true;

    if (isRSSIActive()) { // Lama: need to verify if the first line is the header
        FILE* in = fopen(getRSSIFilePath(), "r");
        FILE* out = fopen(getBaseDir() + , "w");
        if (!in || !out) return false;
        // Write header to the temporary file
        for (int i = 1; i <= NUMBER_OF_ANCHORS; ++i) {
            fprintf(out, std::string(i) + "_rssi,");
        }
        fprintf("Label\n");

        std::string line;
        while (readLineFromFile(in, line)) {
            splitBySeparator(line, ',');
            if (splitedString.size() != NUMBER_OF_ANCHORS + 1) continue;

            Label label = static_cast<Label>(std::stoi(splitedString.back()));
            if (validMap[label]) fprintf(out, "%s", line);
        }

        fclose(in);
        fclose(out);
        remove(getRSSIFilePath());
        rename(RSSI_TMP, getRSSIFilePath());
        LOG_INFO("FLASH", "RSSI CSV cleaned.");
    }

    if (isTOFActive(currentSystemState)) {
        FILE* in = fopen(getTOFFilePath(), "r");
        FILE* out = fopen(getBaseDir() + TOF_FILENAME + TMP_SUFFIX, "w");
        if (!in || !out) return false;
        // Write header to the temporary file
        for (int i = 1; i <= NUMBER_OF_RESPONDERS ; ++i) {
            fprintf(out, std::string(i) + "_tof,");
        }
        fprintf("Label\n");
        std::string line;
        while (readLineFromFile(in, line)) {
            std::string str(line);
            splitBySeparator(line, ',');
            if (splitedString.size() != NUMBER_OF_RESPONDERS + 1) continue;

            Label label = static_cast<Label>(std::stoi(splitedString.back()));
            if (validMap[label]) fprintf(out, "%s", line);
        }

        fclose(in);
        fclose(out);
        remove(getTOFFilePath());
        rename(getBaseDir() + TOF_FILENAME + TMP_SUFFIX, getTOFFilePath());
        LOG_INFO("FLASH", "TOF CSV cleaned.");
    }

    return success;
}

bool resetCSV(void) {
    bool success = true;

    if (isRSSIActive()) {
        FILE* f = fopen(getRSSIFilePath(), "w");
        if (!f) return false;
        for (int i = 1; i <= NUMBER_OF_ANCHORS; ++i)
            fprintf(f, "%d_rssi,", i);
        fprintf(f, "Location\n");
        fclose(f);
        LOG_INFO("FLASH", "RSSI CSV reset.");
    }

    if (isTOFActive()) {
        FILE* f = fopen(getTOFFilePath(), "w");
        if (!f) return false;
        for (int i = 1; i <= NUMBER_OF_RESPONDERS; ++i)
            fprintf(f, "%d_tof,", i);
        fprintf(f, "Location\n");
        fclose(f);
        LOG_INFO("FLASH", "TOF CSV reset.");
    }

    return success;
}