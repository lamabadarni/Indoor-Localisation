#include "data.h"
#include "internalFlashIO.h"
#include "../utils/logger.h"

void saveData(RSSIData scanData) {
    rssiDataSet.push_back(scanData);
    LOG_DEBUG("DATA", "Buffered RSSI scan for label %d", scanData.label);
}

void saveData(TOFData scanData) {
    tofDataSet.push_back(scanData);
    LOG_DEBUG("DATA", "Buffered TOF scan for label %d", scanData.label);
}

void doneCollectingData() {
    LOG_DEBUG("DATA", "Finishing scan collection. Buffered: RSSI=%d, TOF=%d",
             rssiDataSet.size(), tofDataSet.size());

    if (!BufferedData::rssiData & !BufferedData::tofData) {
        LOG_INFO("DATA", "bufferToCSV disabled — data not flushed.");
        return;
    }

    if (BufferedData::rssiData) {
        for (const auto& row : rssiDataSet) {
            if (saveRSSIScan(row)) savedRSSI++;
            else LOG_ERROR("DATA", "Failed to write RSSI row.");
        }
    }

    delay_ms(USER_PROMPTION_DELAY);

    if(BufferedData::tofData) {
        for (const auto& row : tofDataSet) {
            if (saveTOFScan(row)) savedTOF++;
            else LOG_ERROR("DATA", "Failed to write TOF row.");
        }
    }

    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO("DATA", "Flush complete");
}