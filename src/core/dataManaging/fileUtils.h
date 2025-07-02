#ifndef FILE_UTILITIES_H
#define FILE_UTILITIES_H

#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include "core/ui/userUI.h"

constexpr char* MOUNT_POINT = "/sdcard";

/**
 * @brief Get the base directory for the current system state.
 */
std::string getBaseDir(void);

/**
 * @brief Get the full path to the RSSI scan data file.
 */
std::string getStaticRSSIFilePath(void);

/**
 * @brief Get the full path to the RSSI scan data file.
 */
std::string getDynamicRSSIFilePath(void);

/**
 * @brief Get the full path to the TOF scan data file.
 */
std::string getTOFFilePath(void);

/**
 * @brief Get the full path to the location accuracy file.
 */
std::string getAccuracyFilePath(void);

/**
 * @brief Get the full path to the log file of the current run.
 */
std::string getLogFilePath(void);

#endif