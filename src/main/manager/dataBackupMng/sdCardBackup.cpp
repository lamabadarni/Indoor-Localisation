
#include <SD.h>
#include <SPI.h>
#include <Arduino.h>
#include <sdCardBackup.h>
#include <userUI.h>


static String CurrCsvPathToOpen = "";
static String metaPathToOpen = "";
static String accuracyPathToOpen = "";
String tmp = ".tmp";
//------------------------------------------------------------------------------
// Forward declarations for local (static) functions
//------------------------------------------------------------------------------
static bool loadFileToDataset(const String& path);
static std::vector<String> splitedString;
static void fromCSVRssiToVector( String line);
static bool deleteInvalidLocations(const String& filePath);
//==================================================================
//          PUBLIC FUNCTIONS
//==================================================================

static void splitByComma(String data, char comma) {
    splitedString.clear();
    int sepIndex;
    while ((sepIndex = data.indexOf(comma)) != -1) {
        splitedString.push_back(data.substring(0, sepIndex));
        data = data.substring(sepIndex + 1);
    }
    splitedString.push_back(data); // last segment
}

bool initSDCard() {
    delay(1000);
    Serial.println("Initializing SD card...");
    int retries = 0;
    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH); // Ensure CS pin is high before initializing
    SD.end(); // Ensure previous SD state is cleared
    SPI.begin(18, 19, 23, 5);
    if (!SD.begin()) {
        delay(500);
        Serial.println("SD init failed. Prompting user...");
        delay(500);
        return promptUserSDCardInitializationApprove();
    }
    File root = SD.open("/");
    if (!root || !root.isDirectory()) {
        Serial.println("file system is not mounted.");
        Serial.println("Failed to open root directory.");
        SD.end(); // Ensure previous SD state is cleared
        return false;
    }
    else {
        Serial.println("SD card mounted successfully.");
    }
    Serial.println("SD card initialized.");
    return true;
}

bool loadLocationDataset() {
    rssiDataSet.clear();
    bool ok = true;
    auto state = currentSystemState;

    // Load RSSI if required

    if (!loadFileToDataset(getRSSIFilePath())) {
            SD.end(); // Ensure previous SD state is cleared
        ok = false;

    }
    if (!ok) Serial.println("Error loading one or more dataset files.");
    return ok;
}

bool deleteInvalidLocations(const String& filePath) {
    if (!SD.exists(filePath)) {
        Serial.println("File not found for filtering: " + filePath);
        return false;
    }

    String tmpPath = filePath + tmp;
    File inputFile = SD.open(filePath, FILE_READ);
    File outputFile = SD.open(tmpPath, FILE_WRITE);
    if (!inputFile || !outputFile) {
        SD.end(); // Ensure previous SD state is cleared
        Serial.println("Error opening files for filtering.");
        if (inputFile) inputFile.close();
        if (outputFile) outputFile.close();
        return false;
    }

    // Copy header
    String header = inputFile.readStringUntil('\n');
    outputFile.println(header);

    // Copy only valid data rows
    while (inputFile.available()) {
        String line = inputFile.readStringUntil('\n');
        line.trim();
        if (line.length() < 3) continue;

        int lastComma = line.lastIndexOf(',');
        String temp = line.substring(lastComma+1);
        temp.trim();
        Label loc = (Label)temp.toInt();
        if (reuseFromSD[loc]) {
            outputFile.println(line);
        }
    }
    inputFile.close();
    outputFile.close();

    SD.remove(filePath);
    SD.rename(tmpPath, filePath);
    Serial.println("Filtered file: " + filePath);
    return true;
}

bool updateCSV() {
    if (deleteInvalidLocations(getRSSIFilePath())) 
      return loadLocationDataset();

    Serial.println("updateCSV failed.");
    return false;
}

void fromCSVRssiToVector( String line){
    RSSIData row{};
     splitByComma(line,',');
     if(splitedString.size() != NUMBER_OF_ANCHORS+1){
        return;
     }

     for(int i=0 ; i<NUMBER_OF_ANCHORS;i++){
     row.RSSIs[i]=splitedString[i].toInt();
     }
     row.label=(Label)splitedString[NUMBER_OF_ANCHORS].toInt();
     rssiDataSet.push_back(row);
     return;
}


//==================================================================
//          PRIVATE (STATIC) FUNCTIONS
//==================================================================

static bool loadFileToDataset(const String& path) {
    File f = SD.open(path, FILE_READ);
    if (!f) {
        Serial.println("Failed to open for reading: " + path);
        return false;
    }
    String header = f.readStringUntil('\n');
    header.trim();
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() < 3) continue;
        fromCSVRssiToVector(line);
    }
    f.close();
    Serial.println("Successfully loaded data from: " + path);
    return true;
}


bool saveRSSIScan(const RSSIData &row) {
    Serial.println(" save rssi scan ");
    // Open (or create) in append mode
    File f = SD.open(getRSSIFilePath(), FILE_APPEND);
    if (!f) {
            SD.end(); // Ensure previous SD state is cleared
        Serial.println("Failed to open for writing: " + getRSSIFilePath());
        return false;
    }

    // If file empty, write header first
    if (f.size() == 0) {
        for (int i = 1; i <= NUMBER_OF_ANCHORS; ++i) {
            if (f.print(String(i) + "_rssi,") == 0) {
                Serial.println("Failed to write header to file.");
                f.close();
                return false;
            }
        }
        f.println("Location");
    }

    // Write the RSSI values
    for (int i = 0; i < NUMBER_OF_ANCHORS; ++i) {
        f.print(String(row.RSSIs[i]));
        f.print(',');
    }
    // Then location label and a timestamp
    f.println(String((int)row.label));
    Serial.println("Saved RSSI values at file: " + getRSSIFilePath());
    f.close();
    return true;
}
