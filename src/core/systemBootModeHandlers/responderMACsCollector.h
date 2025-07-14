#ifndef RESPONDER_MACS_COLLECTOR_H
#define RESPONDER_MACS_COLLECTOR_H

#include "core/utils/utilities.h"
#include "core/utils/platform.h"
#include "core/utils/logger.h"
#include "core/ui/userUI.h"

/**
 * @brief Prompt the user to collect FTM responder MAC addresses interactively.
 */
void collectResponderMACs();

/**
 * @brief Check if a MAC address is all zeros.
 */
bool isMACUnset(const uint8_t* mac);

/**
 * @brief Clear all stored responder MACs.
 */
void clearResponderMacs();

/**
 * @brief Convert a MAC address into a formatted string.
 */
std::string formatMac(const uint8_t* mac);

#endif // RESPONDER_MACS_COLLECTOR_H
