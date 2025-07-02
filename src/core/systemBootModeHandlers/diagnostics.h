#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include "core/utils/utilities.h"

/**
 * @brief Runs interactive diagnostic flow based on boot mode
 */
void interactiveScanCoverage();

/**
 * @brief Perform STATIC RSSI diagnostic
 */
void performRSSIScanCoverage();

/**
 * @brief Perform DYNAMIC RSSI diagnostic
 */
void performDynamicScanCoverage();

/**
 * @brief Perform TOF diagnostic
 */
void performTOFScanCoverage();

/**
 * @brief Compute STATIC RSSI coverage result
 */
Coverage scanStaticForCoverage();

/**
 * @brief Compute DYNAMIC RSSI coverage result
 */
Coverage scanDynamicForCoverage();

/**
 * @brief Compute TOF coverage result
 */
Coverage scanTOFForCoverage();

#endif // DIAGNOSTICS_H
