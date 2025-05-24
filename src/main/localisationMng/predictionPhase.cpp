#include "predictionPhase.h"
#include "rssiScanner.h"
#include "tofScanner.h"
#include "utillities.h"

#define BACKUB_ACCURACY_THRESHOLD (0.8)
#define TRAIN_TEST_SPLIT (0.8)

void runPredictionPhase(void) {
    Serial.println("\n=============== Prediction Phase Started ===============");

    if (currentSystemState == OFFLINE) {
        Serial.println("Prediction Phase: System state is OFFLINE. Cannot proceed.");
        return;
    }

    Serial.println("Prediction Phase: System state = " + String(systemStateToString(currentSystemState)));

    while(true) {

    Serial.println(">> Press Enter to start predecting...");
        while (!Serial.available()) delay(50);
        Serial.read();  // consume newline

    Label finalPrediction = NOT_ACCURATE;

    bool hasTOF = currentSystemState == STATIC_RSSI_TOF || currentSystemState == STATIC_DYNAMIC_RSSI_TOF;

    Serial.println("Prediction Phase: Collecting RSSI scan...");
    createRSSIScanToMakePrediction();

    if(hasTOF) {
        Serial.println("Prediction Phase: Collecting TOF scan...");
        createTOFScanToMakePrediction();
    }

    Label rssiLabel = rssiPredict();
    Label tofLabel  = tofPredict();

    if (hasTOF && rssiLabel != tofLabel) {
        Serial.println("Prediction Phase: RSSI and TOF predictions differ:");
        Serial.println("  RSSI Prediction: " + String(labelToString(tofLabel)));
        Serial.println("  TOF  Prediction: " + String(labelToString(tofLabel)));

        int userChoice = promptUserPreferredPrediction();

        if (userChoice == 1) {
            Serial.println("UserUI: User chose RSSI prediction.");
             RSSIData scanData;
             scanData.label = rssiLabel;
             for (int j = 0; j < NUMBER_OF_ANCHORS; ++j) {
                scanData.RSSIs[j] = accumulatedRSSIs[j];
             }
             rssiDataSet.push_back(scanData);
        } 
        else if (userChoice == 2) {
            Serial.println("UserUI: User chose RSSI prediction.");
             TOFData scanData;
             scanData.label = tofLabel;
             for (int j = 0; j < NUMBER_OF_RESPONDERS; ++j) {
                scanData.TOFs[j] = accumulatedTOFs[j];
             }
             rssiDataSet.push_back(scanData);
        }
    } else {
        Serial.println("Prediction Phase: Final prediction = " + String(labelToString(rssiLabel)));
    }
}
}

static double euclidean(const double* a, const double* b, int size) {
    double sum = 0;
    for (int i = 0; i < size; ++i)
    {
      sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sqrt(sum);
}


static void preparePointRSSI(int RSSIs[NUMBER_OF_ANCHORS], double toBeNormalised[NUMBER_OF_ANCHORS]) {
    // Normalize the RSSI values using min-max scaling
    for (int i = 0; i < NUMBER_OF_ANCHORS; ++i)
    {
        toBeNormalised[i] = ((double)(RSSIs[i] + 100)) / 100.0;
    }
}

static void preparePointTOF(double TOF[NUMBER_OF_RESPONDERS], double toBeNormalised[NUMBER_OF_RESPONDERS]) {
    // Normalize the TOF values using min-max scaling
    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i)
    {
        toBeNormalised[i] = TOF[i] / 2000;
    }
}

static Label _predict(vector<double> &distances, vector<Label> &labels, int k) {
    int sizeOfDataSet = distances.size();

    // Find the minimum K distances using bubble sort
    for (int i = 0; i < k; ++i)
    {
        for (int j = 0; j < sizeOfDataSet - i - 1; ++j)
        {
            if (distances[j] < distances[j + 1])
            {
                // Swap distances
                double tempDist = distances[j];
                distances[j] = distances[j + 1];
                distances[j + 1] = tempDist;

                // Swap corresponding labels
                Label tempLabel = labels[j];
                labels[j] = labels[j + 1];
                labels[j + 1] = tempLabel;
            }
        }
    }

    // Count votes for the K closest labels
    int closestLabel[NUMBER_OF_LABELS] = {0};

    for (int i = 0; i < K; ++i)
    {
        closestLabel[labels[sizeOfDataSet - i - 1]]++;
    }

    // Find the label with the maximum votes
    int maxVotes = 0;
    // Default value
    Label labelWithMaxVotes = (Label) 0;

    for (int i = 0; i < NUMBER_OF_LABELS; ++i)
    {
        if (closestLabel[i] > maxVotes)
        {
            maxVotes = closestLabel[i];
            labelWithMaxVotes = (Label)i;
        }
    }

    return labelWithMaxVotes;
}

Label rssiPredict() {
    int sizeOfDataSet = rssiDataSet.size();
    double normalisedInput[NUMBER_OF_ANCHORS];
    vector<double> distances(sizeOfDataSet, 0);
    vector<Label> labels(sizeOfDataSet, NOT_ACCURATE);

    preparePointRSSI(accumulatedRSSIs, normalisedInput);
    // Calculate distances and store corresponding labels
    for (int i = 0 ; i < sizeOfDataSet ; ++i) {
        double normalisedPoint[NUMBER_OF_ANCHORS];

        preparePointRSSI(rssiDataSet[i].RSSIs, normalisedPoint);
        distances[i] = euclidean(normalisedPoint, normalisedInput, NUMBER_OF_ANCHORS);
        labels[i] = rssiDataSet[i].label;
    }

    return _predict(distances, labels, K_RSSI);
}

Label tofPredict() {
    int sizeOfDataSet = tofDataSet.size();
    double normalisedInput[NUMBER_OF_RESPONDERS];
    vector<double> distances(sizeOfDataSet, 0);
    vector<Label> labels(sizeOfDataSet, NOT_ACCURATE);

    preparePointTOF(accumulatedTOFs, normalisedInput);
    // Calculate distances and store corresponding labels
    for (int i = 0 ; i < sizeOfDataSet ; ++i) {
        double normalisedPoint[NUMBER_OF_RESPONDERS];

        preparePointTOF(tofDataSet[i].TOFs, normalisedPoint);
        distances[i] = euclidean(normalisedPoint, normalisedInput, NUMBER_OF_RESPONDERS);
        labels[i] = tofDataSet[i].label;
    }

    return _predict(distances, labels, K_TOF);
}
