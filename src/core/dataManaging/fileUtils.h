#pragma once 

#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/ui/logger.h"
#include "core/ui/userUI.h"

static constexpr char META_FILENAME[]        = "meta_";
static constexpr char RSSI_FILENAME[]        = "rssi_scan_data_";
static constexpr char TOF_FILENAME[]         = "tof_scan_data_";
static constexpr char ACCURACY_FILENAME[]    = "location_accuracy_";
static constexpr char TMP_SUFFIX[]           = ".tmp";
static constexpr char CSV_SUFFIX[]           = ".csv";

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