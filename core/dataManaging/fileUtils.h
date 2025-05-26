#include "../utils/platform.h"
#include "../utils/utilities.h"


/**
 * @brief Get the base directory on the SD card for the current system state.
 */
std::string getSDBaseDir();

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

bool resetStorage();

extern void splitByComma(std::string data, char comma);
extern std::vector<std::string> splitedString;
extern void fromCSVRssiToVector(std::string line);
extern void fromCSVTofToVector(std::string line);
