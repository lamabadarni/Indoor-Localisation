/**
 * @file predictionPhase.cpp
 * @brief Executes real-time predictions using KNN over RSSI dataset with user trust resolution for conflicting results.
 * @author Ward Iroq
 */

#include <predictionPhase.h>
#include <rssiScanner.h>
#include <userUI.h>
#include <vector>
#include <utilities.h>

#define BACKUB_ACCURACY_THRESHOLD (0.8)
#define TRAIN_TEST_SPLIT (0.8)

void runPredictionPhase(void) {
    Serial.println("\n=============== Prediction Phase Started ===============");

    if (currentSystemState == OFFLINE) {
        Serial.println("[PREDICT] System state is OFFLINE. Cannot proceed.");
        return;
    }
    
    Serial.println("[PREDICT] System state = " + String(systemStateToString(currentSystemState)));

    while(true) {
        Serial.println("[PREDICT] >> Press Enter to start predicting...");
        while (!Serial.available()) delay(50);

        Serial.read();  // consume newline
        Serial.println("[PREDICT] Collecting RSSI scan...");
        createRSSIScanToMakePrediction();

        Label rssiLabel = rssiPredict();

        Serial.println("[PREDICT] Do you want to continue predicting? (y/n)");
        while (!Serial.available()) delay(50);
        char response = Serial.read();
        if (response != 'y' && response != 'Y') {
            Serial.println("[PREDICT] User chose to exit prediction loop.");
            break;
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

static Label _predict(std::vector<double> &distances, std::vector<Label> &labels, int k) {
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

    for (int i = 0; i < k; ++i)
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
    std::vector<double> distances(sizeOfDataSet, 0);
    std::vector<Label> labels(sizeOfDataSet, NOT_ACCURATE);

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
