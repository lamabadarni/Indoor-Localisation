#pragma once 

#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include "core/ui/userUI.h"

static constexpr char STATIC_RSSI_FILENAME[] = "static_rssi_data";
static constexpr char DYNAMIC_RSSI_FILENAME[] = "dynamic_rssi_data";
static constexpr char TOF_FILENAME[]         = "tof_data";
static constexpr char ACCURACY_FILENAME[]    = "acc_data";
static constexpr char TMP_SUFFIX[]           = ".tmp";
static constexpr char CSV_SUFFIX[]           = ".csv";
constexpr const char* MOUNT_POINT            = "/indoor-localisation-data";

/**
 * @brief Get the base directory for the current system state.
 */
std::string getBaseDir();

/**
 * @brief Get the full path to the RSSI scan data file.
 */
std::string getStaticRSSIFilePath();

/**
 * @brief Get the full path to the RSSI scan data file.
 */
std::string getDynamicRSSIFilePath();

/**
 * @brief Get the full path to the TOF scan data file.
 */
std::string getTOFFilePath();

/**
 * @brief Get the full path to the location accuracy file.
 */
std::string getAccuracyFilePath();
