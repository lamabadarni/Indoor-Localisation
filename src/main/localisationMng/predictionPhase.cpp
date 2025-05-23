#include "predictionPhase.h"

static double euclidean(const double* a, const double* b, int size)
{
    int sum = 0;
    for (int i = 0; i < size; ++i)
    {
      sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sqrt(sum);
}

LOCATIONS knnPredict(const double input[NUMBER_OF_ANCHORS])
{
    int sizeOfDataSet = dataSet.size();
    vector<double> distances(sizeOfDataSet, 0);
    vector<LOCATIONS> labels(sizeOfDataSet, NOT_ACCURATE);

    // Calculate distances and store corresponding labels
    for (int i = 0 ; i < sizeOfDataSet ; ++i) {
        distances[i] = euclidean(dataSet[i].RSSIs, input, sizeOfDataSet);
        labels[i] = dataSet[i].label;
    }

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


void preparePoint(double RSSIs[NUMBER_OF_ANCHORS])
{
    for (int i = 0; i < NUMBER_OF_ANCHORS; ++i)
    {
        // Normalize the RSSI values using min-max scaling
        RSSIs[i] = (RSSIs[i] + 100) / 100.0;
    }
}

LOCATIONS validateScanningPhasePerLabel(std::vector<Data> scanResultSamepleForValidationPerLabel) {
}
