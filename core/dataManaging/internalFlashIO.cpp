
#include "internalFlashIO.h"
#include "../utils/platform.h"
#include "../ui/logger.h"
#include "../utils/utilities.h"
#include "../ui/userUI.h"

#define RSSI_FILE "/spiffs/rssi_scan_data_.csv"
#define TOF_FILE  "/spiffs/tof_scan_data_.csv"
#define RSSI_TMP  "/spiffs/rssi_scan_data_.csv.tmp"
#define TOF_TMP   "/spiffs/tof_scan_data_.csv.tmp"

// ====================== Dependencies ======================

extern void splitByComma(std::string data, char comma);
extern std::vector<std::string> splitedString;
extern void fromCSVRssiToVector(std::string line);
extern void fromCSVTofToVector(std::string line);

// ====================== Initialization ======================

bool initInternalFlash() {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false
    };

    esp_err_t err = esp_vfs_spiffs_register(&conf);
    if (err == ESP_OK) {
        size_t total, used;
        esp_spiffs_info(NULL, &total, &used);
        LOG_INFO("FLASH", "SPIFFS mounted. Total: %d, Used: %d", total, used);
        return true;
    }

    LOG_WARN("FLASH", "SPIFFS mount failed: %s", esp_err_to_name(err));

    if (!promptUserFlashRecoveryApprove()) {
        LOG_ERROR("FLASH", "SPIFFS recovery declined by user.");
        return false;
    }

    LOG_INFO("FLASH", "Formatting SPIFFS...");
    if (esp_spiffs_format(NULL) != ESP_OK) {
        LOG_ERROR("FLASH", "SPIFFS format failed.");
        return false;
    }

    if (esp_vfs_spiffs_register(&conf) != ESP_OK) {
        LOG_ERROR("FLASH", "SPIFFS re-mount failed after format.");
        return false;
    }

    LOG_INFO("FLASH", "SPIFFS recovery successful.");
    return true;
}

// ====================== Dataset Loading ======================

bool loadLocationDataset() {
    bool ok = true;

    if (isRSSIActive(currentSystemState)) {
        FILE* f = fopen(RSSI_FILE, "r");
        if (f) {
            LOG_INFO("FLASH", "Loading RSSI dataset...");
            char line[256];
            while (fgets(line, sizeof(line), f)) {
                fromCSVRssiToVector(std::string(line));
            }
            fclose(f);
        } else {
            LOG_WARN("FLASH", "RSSI file not found.");
            ok = false;
        }
    }

    if (isTOFActive(currentSystemState)) {
        FILE* f = fopen(TOF_FILE, "r");
        if (f) {
            LOG_INFO("FLASH", "Loading TOF dataset...");
            char line[256];
            while (fgets(line, sizeof(line), f)) {
                fromCSVTofToVector(std::string(line));
            }
            fclose(f);
        } else {
            LOG_WARN("FLASH", "TOF file not found.");
            ok = false;
        }
    }

    return ok;
}

// ====================== Reset ======================

bool resetCSV() {
    bool success = true;

    if (isRSSIActive(currentSystemState)) {
        FILE* f = fopen(RSSI_FILE, "w");
        if (!f) return false;
        for (int i = 1; i <= NUMBER_OF_ANCHORS; ++i)
            fprintf(f, "%d_rssi,", i);
        fprintf(f, "Location\n");
        fclose(f);
        LOG_INFO("FLASH", "RSSI CSV reset.");
    }

    if (isTOFActive(currentSystemState)) {
        FILE* f = fopen(TOF_FILE, "w");
        if (!f) return false;
        for (int i = 1; i <= NUMBER_OF_RESPONDERS; ++i)
            fprintf(f, "%d_tof,", i);
        fprintf(f, "Location\n");
        fclose(f);
        LOG_INFO("FLASH", "TOF CSV reset.");
    }

    return success;
}

// ====================== Cleanup ======================

bool cleanupCSV(const bool validMap[NUMBER_OF_LABELS]) {
    bool success = true;

    if (isRSSIActive(currentSystemState)) {
        FILE* in = fopen(RSSI_FILE, "r");
        FILE* out = fopen(RSSI_TMP, "w");
        if (!in || !out) return false;

        char line[256];
        while (fgets(line, sizeof(line), in)) {
            std::string str(line);
            splitByComma(str, ',');
            if (splitedString.size() != NUMBER_OF_ANCHORS + 1) continue;

            Label label = static_cast<Label>(std::stoi(splitedString.back()));
            if (validMap[label]) fprintf(out, "%s", line);
        }

        fclose(in);
        fclose(out);
        remove(RSSI_FILE);
        rename(RSSI_TMP, RSSI_FILE);
        LOG_INFO("FLASH", "RSSI CSV cleaned.");
    }

    if (isTOFActive(currentSystemState)) {
        FILE* in = fopen(TOF_FILE, "r");
        FILE* out = fopen(TOF_TMP, "w");
        if (!in || !out) return false;

        char line[256];
        while (fgets(line, sizeof(line), in)) {
            std::string str(line);
            splitByComma(str, ',');
            if (splitedString.size() != NUMBER_OF_RESPONDERS + 1) continue;

            Label label = static_cast<Label>(std::stoi(splitedString.back()));
            if (validMap[label]) fprintf(out, "%s", line);
        }

        fclose(in);
        fclose(out);
        remove(TOF_FILE);
        rename(TOF_TMP, TOF_FILE);
        LOG_INFO("FLASH", "TOF CSV cleaned.");
    }

    return success;
}

// ====================== Scan Row Writing ======================

bool saveRSSIScan(const RSSIData& row) {
    FILE* f = fopen(RSSI_FILE, "a");
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

bool saveTOFScan(const TOFData& row) {
    FILE* f = fopen(TOF_FILE, "a");
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
