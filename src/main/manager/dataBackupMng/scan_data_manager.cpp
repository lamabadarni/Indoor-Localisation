// scan_data_manager.cpp
/*#include <scan_data_manager.h>
#include <SD.h>
#include <SPI.h>

bool createMetaFile(const String &filename) {
    if (SD.exists(filename)) {
        Serial.println("Meta file already exists.");
        return false;
    }

    File f = SD.open(filename, FILE_WRITE);
    if (!f) {
        Serial.println("Failed to create meta file.");
        return false;
    }


    f.print(currentConfig.systemState);
    f.print(',');
    f.print(currentConfig.RSSINum);
    f.print(',');
    f.print(currentConfig.TOFNum);

    f.close();
    Serial.println("Meta file created: " + filename);
    return true;
}
*/
