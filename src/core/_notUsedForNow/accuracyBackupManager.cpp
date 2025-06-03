/*
#include <Accuracy_data_manger.h>
#include <SD.h>

// Write all accuracyDatas to CSV (overwrites existing)
bool saveAllAccuracyRecords() {
    File f = SD.open(LOCATION_ACCURACY_FILE_NAME.c_str(), FILE_WRITE);
    if (!f) {
        Serial.println("Failed to open location_accuracy file for writing.");
        return false;
    }

    // Header
    f.println("Location,Accuracy");

    // Data rows
    for (const AccuracyData& rec : accuracyDatas) {
        f.print(String((int)rec.location));
        f.print(',');
        f.print(rec.accuracy, 3);
    }

    f.close();
    Serial.println("Accuracy records saved to: " + LOCATION_ACCURACY_FILE_NAME);
    return true;
}

// Load into accuracyDatas (clears existing)
bool loadAllAccuracyRecords() {
    accuracyDatas.clear();
    if (!SD.exists(LOCATION_ACCURACY_FILE_NAME.c_str())) {
        Serial.println("Accuracy file does not exist: " + LOCATION_ACCURACY_FILE_NAME);
        return false;
    }

    File f = SD.open(LOCATION_ACCURACY_FILE_NAME.c_str(), FILE_READ);
    if (!f) {
        Serial.println("Failed to open accuracy file.");
        return false;
    }

    f.readStringUntil('\n'); // skip header
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.isEmpty()) continue;

        int c1 = line.indexOf(',');
        int c2 = line.indexOf(',', c1 + 1);

        AccuracyData rec;
        rec.location  = (LOCATIONS) line.substring(0, c1).toInt();
        rec.accuracy  = line.substring(c1 + 1, c2).toFloat();

        accuracyDatas.push_back(rec);
    }
    f.close();
    Serial.println("Loaded " + String(accuracyDatas.size()) + " accuracy records.");
    return true;
}

// Append one AccuracyData row (with header if needed)
bool saveAccuracyData(const AccuracyData &row) {
    File f = SD.open(getAccuracyFilePath(), FILE_WRITE);
    if (!f) {
        Serial.println("Unable to open accuracy CSV");
        return false;
    }

    if (f.size() == 0) {
        f.println("Location,Accuracy");
    }

    f.print((String(int)row.location));
    f.print(',');
    f.print(row.accuracy, 3);
    f.print(',');

    f.close();
    return true;
}

*/