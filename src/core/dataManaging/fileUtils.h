#pragma once 

#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include "core/ui/userUI.h"

static constexpr char META_FILENAME[]        = "meta_";       // This one is okay
static constexpr char RSSI_FILENAME[]        = "rssi_data_";  // Shortened
static constexpr char TOF_FILENAME[]         = "tof_data_";   // Shortened
static constexpr char ACCURACY_FILENAME[]    = "acc_data_";   // Shortened
static constexpr char TMP_SUFFIX[]           = ".tmp";
static constexpr char CSV_SUFFIX[]           = ".csv";
constexpr const char* MOUNT_POINT = "/littlefs";

/**
 * @brief Get the base directory for the current system state.
 */
std::string getBaseDir();

/**
 * @brief Get the full path to the meta file based on current system state.
 */
std::string getMetaFilePath();

/**
 * @brief Get the full path to the RSSI scan data file.
 */
std::string getRSSIFilePath();

/**
 * @brief Get the full path to the TOF scan data file.
 */
std::string getTOFFilePath();

/**
 * @brief Get the full path to the location accuracy file.
 */
std::string getAccuracyFilePath();