#pragma once

#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include "core/ui/userUI.h"
#include "core/dataManaging/fileUtils.h"

/**
 * @brief Mounts the SPIFFS partition. If mount fails, optionally formats and remounts.
 * @return true if SPIFFS is mounted, false otherwise.
 */
bool initInternalFlash();