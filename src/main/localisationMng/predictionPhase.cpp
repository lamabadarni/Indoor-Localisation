#include "predictionPhase.h"

#define BACKUB_ACCURACY_THRESHOLD (0.8)
#define TRAIN_TEST_SPLIT (0.8)

static double euclidean(const double* a, const double* b, int size)
{
    int sum = 0;
    for (int i = 0; i < size; ++i)
    {
      sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sqrt(sum);
}


static LOCATIONS _predict(const vector<double> &distances, const vector<LOCATIONS> &labels)
{
    // Find the minimum K distances using bubble sort
    for (int i = 0; i < K; ++i)
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
                LOCATIONS tempLabel = labels[j];
                labels[j] = labels[j + 1];
                labels[j + 1] = tempLabel;
            }
        }
    }

    // Count votes for the K closest labels
    int closestLabel[NUMBER_OF_LOCATIONS] = {0};

    for (int i = 0; i < K; ++i)
    {
        closestLabel[labels[sizeOfDataSet - i - 1]]++;
    }

    // Find the label with the maximum votes
    int maxVotes = 0;
    // Default value
    LOCATIONS labelWithMaxVotes = (LOCATIONS) 0;

    for (int i = 0; i < NUMBER_OF_LOCATIONS; ++i)
    {
        if (closestLabel[i] > maxVotes)
        {
            maxVotes = closestLabel[i];
            labelWithMaxVotes = (LOCATIONS)i;
        }
    }

    return labelWithMaxVotes;
}

LOCATIONS rssiPredict(double input[NUMBER_OF_ANCHORS])
{
    int sizeOfDataSet = rssiDataSet.size();
    vector<double> distances(sizeOfDataSet, 0);
    vector<LOCATIONS> labels(sizeOfDataSet, NOT_ACCURATE);

    // Calculate distances and store corresponding labels
    for (int i = 0 ; i < sizeOfDataSet ; ++i) {
        distances[i] = euclidean(rssiDataSet[i].RSSIs, input, sizeOfInput);
        labels[i] = dataSet[i].label;
    }

    return _predict(distances, labels);
}

LOCATIONS tofPredict(double input[NUMBER_OF_RESPONDERS])
{
    int sizeOfDataSet = tofDataSet.size();
    vector<double> distances(sizeOfDataSet, 0);
    vector<LOCATIONS> labels(sizeOfDataSet, NOT_ACCURATE);

    // Calculate distances and store corresponding labels
    for (int i = 0 ; i < sizeOfDataSet ; ++i) {
        distances[i] = euclidean(tofDataSet[i].TOFs, input, sizeOfInput);
        labels[i] = dataSet[i].label;
    }

    return _predict(distances, labels);
}

void preparePoint(double RSSIs[NUMBER_OF_ANCHORS])
{
    for (int i = 0; i < NUMBER_OF_ANCHORS; ++i)
    {
        // Normalize the RSSI values using min-max scaling
        RSSIs[i] = (RSSIs[i] + 100) / 100.0;
    }
}

bool isBackupDataSetRelevant(void)
{
    if (dataSet.empty() < K)
    {
        return false; // Not enough data for KNN
    }

    int sizeOfDataSet = dataSet.size();
    int numTrainSamples = (int)(sizeOfDataSet * TRAIN_TEST_SPLIT);
    int numTestSamples = sizeOfDataSet - numTrainSamples;

    // Split the dataset into training and testing sets
    std::vector<Data> trainSet(dataSet.begin(), dataSet.begin() + numTrainSamples);
    std::vector<Data> testSet(dataSet.begin() + numTrainSamples, dataSet.end());

    // Calculate the accuracy of the backup dataset
    int correctPredictions = 0;
    for (const auto& sample : testSet)
    {
        LOCATIONS predictedLabel = knnPredict(sample.RSSIs[i]);
        if (predictedLabel == sample.label)
        {
            correctPredictions++;
        }
    }

    double accuracy = static_cast<double>(correctPredictions) / numTestSamples;

    return accuracy >= BACKUB_ACCURACY_THRESHOLD;
}

LOCATIONS validateScanningPhasePerLabel(std::vector<Data> scanResultSamepleForValidationPerLabel) {
}
