/**
 * @file predictionPhase.cpp
 * @brief Executes real-time predictions using KNN over RSSI and TOF datasets with user trust resolution for conflicting results.
 * @author Ward Iroq
 */

#include "../utils/platform.h"
#include "../utils/utilities.h"  
#include "predictionPhase.h"
#include "../scanning/rssiScanner.h"
#include "../scanning/tofScanner.h"
#include "../ui/userUI.h"

#define BACKUB_ACCURACY_THRESHOLD (0.8)
#define TRAIN_TEST_SPLIT (0.8)

void runPredictionPhase(void) {
    LOG_INFO("UI", " ");
    LOG_INFO("UI", "=============== Prediction Phase Started ===============");

    if (currentSystemState == OFFLINE) {
        LOG_ERROR("PREDICT", "System state is OFFLINE. Cannot proceed.");
        return;
    }

    LOG_INFO("PREDICT", "System state = %s", systemStateToString());

    while (true) {
        LOG_INFO("UI", "[PREDICT] >> Press Enter to start predicting...");
        readCharFromUser();

        bool hasTOF = currentSystemState == STATIC_RSSI_TOF || currentSystemState == STATIC_DYNAMIC_RSSI_TOF;

        LOG_INFO("PREDICT", "Collecting RSSI scan...");
        createRSSIScanToMakePrediction();

        if (hasTOF) {
            LOG_INFO("PREDICT", "Collecting TOF scan...");
            createTOFScanToMakePrediction();
        }

        Label rssiLabel = rssiPredict();
        Label tofLabel = tofPredict();

        if (hasTOF && rssiLabel != tofLabel) {
            LOG_WARN("PREDICT", "RSSI and TOF predictions differ:");
            LOG_INFO("PREDICT", "  RSSI Prediction: %s", labelToString[rssiLabel]);
            LOG_INFO("PREDICT", "  TOF  Prediction: %s", labelToString[tofLabel]);

            int userChoice = promptUserPreferredPrediction();

            if (userChoice == 1) {
                LOG_INFO("PREDICT", "User chose RSSI prediction.");
                RSSIData scanData;
                scanData.label = rssiLabel;
                for (int j = 0; j < NUMBER_OF_ANCHORS; ++j) {
                    scanData.RSSIs[j] = accumulatedRSSIs[j];
                }
                rssiDataSet.push_back(scanData);
            } else if (userChoice == 2) {
                LOG_INFO("PREDICT", "User chose TOF prediction.");
                TOFData scanData;
                scanData.label = tofLabel;
                for (int j = 0; j < NUMBER_OF_RESPONDERS; ++j) {
                    scanData.TOFs[j] = accumulatedTOFs[j];
                }
                tofDataSet.push_back(scanData);
            }
        } else {
            LOG_INFO("PREDICT", "Final prediction = %s", labelToString[rssiLabel]);
        }

        //ask user to proceed or not

        if (response != 'y' && response != 'Y') {
            LOG_INFO("PREDICT", "User chose to exit prediction loop.");
            break;
        }
    }
}

static double euclidean(const double* a, const double* b, int size) {
    double sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sqrt(sum);
}

static void preparePointRSSI(int RSSIs[NUMBER_OF_ANCHORS], double toBeNormalised[NUMBER_OF_ANCHORS]) {
    for (int i = 0; i < NUMBER_OF_ANCHORS; ++i) {
        toBeNormalised[i] = ((double)(RSSIs[i] + 100)) / 100.0;
    }
}

static void preparePointTOF(double TOF[NUMBER_OF_RESPONDERS], double toBeNormalised[NUMBER_OF_RESPONDERS]) {
    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        toBeNormalised[i] = TOF[i] / 2000;
    }
}

static Label _predict(std::vector<double>& distances, std::vector<Label>& labels, int k) {
    int sizeOfDataSet = distances.size();

    for (int i = 0; i < k; ++i) {
        for (int j = 0; j < sizeOfDataSet - i - 1; ++j) {
            if (distances[j] < distances[j + 1]) {
                std::swap(distances[j], distances[j + 1]);
                std::swap(labels[j], labels[j + 1]);
            }
        }
    }

    int closestLabel[LABELS_COUNT] = {0};
    for (int i = 0; i < k; ++i) {
        closestLabel[labels[sizeOfDataSet - i - 1]]++;
    }

    int maxVotes = 0;
    Label labelWithMaxVotes = (Label)0;

    for (int i = 0; i < LABELS_COUNT; ++i) {
        if (closestLabel[i] > maxVotes) {
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
    for (int i = 0; i < sizeOfDataSet; ++i) {
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
    std::vector<double> distances(sizeOfDataSet, 0);
    std::vector<Label> labels(sizeOfDataSet, NOT_ACCURATE);

    preparePointTOF(accumulatedTOFs, normalisedInput);
    for (int i = 0; i < sizeOfDataSet; ++i) {
        double normalisedPoint[NUMBER_OF_RESPONDERS];
        preparePointTOF(tofDataSet[i].TOFs, normalisedPoint);
        distances[i] = euclidean(normalisedPoint, normalisedInput, NUMBER_OF_RESPONDERS);
        labels[i] = tofDataSet[i].label;
    }

    return _predict(distances, labels, K_TOF);
}
