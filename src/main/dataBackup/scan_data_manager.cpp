// scan_data_manager.cpp
#include "scan_data_manager.h"
#include <SD.h>
#include <SPI.h>

bool createMetaFile(const String &filename, const ScanConfig& cfg) {
    if (SD.exists(filename)) {
        Serial.println("Meta file already exists.");
        return false;
    }

    File f = SD.open(filename, FILE_WRITE);
    if (!f) {
        Serial.println("Failed to create meta file.");
        return false;
    }

    f.print(cfg.IdRound);
    f.print(',');
    f.print(cfg.systemState);
    f.print(',');
    f.print(cfg.Count_Round);
    f.print(',');
    f.print(cfg.RoundTimestamp);
    f.print(',');
    f.print(cfg.RSSINum);
    f.print(',');
    f.print(cfg.TOFNum);

    f.close();
    Serial.println("Meta file created: " + filename);
    return true;
}

int getRSSINumFromMeta(const char* metaPath) {
    File f = SD.open(metaPath, FILE_READ);
    if (!f) {
        Serial.println("Failed to open meta file.");
        return -1;
    }

    String line = f.readStringUntil('\n');
    f.close();

    if (line.length() == 0) {
        Serial.println("Meta file is empty.");
        return -1;
    }

    int commaIndex = -1;
    for (int i = 0, count = 0; i < line.length(); ++i) {
        if (line[i] == ',') ++count;
        if (count == 4) {
            commaIndex = i;
            break;
        }
    }

    if (commaIndex == -1) {
        Serial.println("Could not find RSSINum position.");
        return -1;
    }

    int nextComma = line.indexOf(',', commaIndex + 1);
    String rssinumStr = (nextComma == -1)
                        ? line.substring(commaIndex + 1)
                        : line.substring(commaIndex + 1, nextComma);

    return rssinumStr.toInt();
}

int getTOFNumFromMeta(const char* metaPath) {
    File f = SD.open(metaPath, FILE_READ);
    if (!f) {
        Serial.println("Failed to open meta file.");
        return -1;
    }

    String line = f.readStringUntil('\n');
    f.close();

    if (line.length() == 0) {
        Serial.println("Meta file is empty.");
        return -1;
    }

    int commaIndex = -1;
    for (int i = 0, count = 0; i < line.length(); ++i) {
        if (line[i] == ',') ++count;
        if (count == 5) {
            commaIndex = i;
            break;
        }
    }

    if (commaIndex == -1) {
        Serial.println("Could not find TOFNum position.");
        return -1;
    }

    int nextComma = line.indexOf(',', commaIndex + 1);
    String tofnumStr = (nextComma == -1)
                       ? line.substring(commaIndex + 1)
                       : line.substring(commaIndex + 1, nextComma);

    return tofnumStr.toInt();
}

bool loadMeta(const char* metaPath, ScanConfig& cfg) {
    if (!SD.exists(metaPath)) return false;
    File f = SD.open(metaPath, FILE_READ);
    if (!f) return false;
    String line = f.readStringUntil('\n');
    f.close();

    std::vector<String> toks;
    int start = 0;
    while (true) {
        int c = line.indexOf(',', start);
        if (c < 0) { toks.push_back(line.substring(start)); break; }
        toks.push_back(line.substring(start, c));
        start = c + 1;
    }
    if (toks.size() < 6) return false;

    cfg.IdRound = toks[0].toInt();
    cfg.systemState = (SystemState)toks[1].toInt();
    cfg.Count_Round = toks[2].toInt();
    cfg.RoundTimestamp = toks[3].toInt();
    cfg.RSSINum = toks[4].toInt();
    cfg.TOFNum = toks[5].toInt();

    return true;
}

bool metaMatches(const ScanConfig& stored, const ScanConfig& current) {
    return stored.IdRound == current.IdRound &&
           stored.systemState == current.systemState &&
           stored.Count_Round == current.Count_Round &&
           stored.RoundTimestamp == current.RoundTimestamp &&
           stored.RSSINum == current.RSSINum &&
           stored.TOFNum == current.TOFNum;
}

bool isCSVComplete(const char* csvPath, const ScanConfig& cfg) {
    if (!SD.exists(csvPath)) return false;
    File f = SD.open(csvPath, FILE_READ);
    if (!f) return false;
    f.readStringUntil('\n');

    int count = 0, needed = cfg.expectedSegments * cfg.scansPerSegment;
    while (f.available() && count < needed) {
        String line = f.readStringUntil('\n');
        if (line.length() > 2) ++count;
    }
    f.close();
    return (count >= needed);
}

bool shouldReuseScans(const char* metaPath, const char* csvPath, const ScanConfig& cfg) {
    if (!SD.exists(metaPath)) return false;
    ScanConfig stored;
    if (!loadMeta(metaPath, stored) || !metaMatches(stored, cfg)) return false;
    if (!SD.exists(csvPath) || !verifyCSVFormat(csvPath, cfg)) return false;
    return true;
}

bool verifyCSVFormat(const char* csvPath, const ScanConfig& currentConfig) {
    File f = SD.open(csvPath, FILE_READ);
    if (!f) return false;

    String hdr = f.readStringUntil('\n');
    f.close();
    hdr.trim();

    std::vector<String> expected;
    bool includeTOF = (currentConfig.systemState == SystemState::STATIC_RSSI_TOF ||
                       currentConfig.systemState == SystemState::STATIC_DYNAMIC_RSSI_TOF);

    for (int i = 1; i <= currentConfig.RSSINum; ++i) {
        expected.push_back(String(i) + "_rssi");
        if (includeTOF) {
            expected.push_back(String(i) + "_tof");
        }
    }
    expected.push_back("Location");
    expected.push_back("Timestamp");

    std::vector<String> actual;
    int start = 0;
    while (true) {
        int comma = hdr.indexOf(',', start);
        if (comma < 0) {
            actual.push_back(hdr.substring(start));
            break;
        }
        actual.push_back(hdr.substring(start, comma));
        start = comma + 1;
    }

    if (actual.size() != expected.size()) return false;
    for (size_t i = 0; i < actual.size(); ++i) {
        if (actual[i] != expected[i]) return false;
    }
    return true;
}
