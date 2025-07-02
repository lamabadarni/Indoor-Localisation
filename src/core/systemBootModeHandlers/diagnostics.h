
#ifndef VEREFIER_H
#define VEREFIER_H

#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include "core/ui/userUI.h"
#include "core/ui/userUIOled.h"

typedef struct {
    int    seen    = 0;
    double average = 0;
} Coverage;

bool interactiveScanCoverage();

void performTOFScanCoverage();

void performRSSIScanCoverage();

Coverage scanRSSIForCoverage();

Coverage scanTOFForCoverage();

#endif // VEREFIER_H
