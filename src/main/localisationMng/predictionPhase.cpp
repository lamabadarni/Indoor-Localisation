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
    vector<LOCATIONS> locations(sizeOfDataSet, NOT_ACCURATE);

    // Calculate distances and store corresponding location labels
    for (int i = 0 ; i < NUMBER_OF_SCANS ; ++i) {
        distances[i] = euclidean(dataSet[i].RSSIs, input, NUMBER_OF_ANCHORS);
        locations[i] = dataSet[i].location;
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

                // Swap corresponding location labels
                LOCATIONS tempLocation = locations[j];
                locations[j] = locations[j + 1];
                locations[j + 1] = tempLocation;
            }
        }
    }

    // Count votes for the K closest locations
    int closestLocation[NUMBER_OF_LOCATIONS] = {0};

    for (int i = 0; i < K; ++i)
    {
        closestLocation[locations[sizeOfDataSet - i - 1]]++;
    }

    // Find the location with the maximum votes
    int maxVotes = 0;
    // Default value
    LOCATIONS locationWithMaxVotes = (LOCATIONS) 0;

    for (int i = 0; i < NUMBER_OF_LOCATIONS; ++i)
    {
        if (closestLocation[i] > maxVotes)
        {
            maxVotes = closestLocation[i];
            locationWithMaxVotes = (LOCATIONS)i;
        }
    }

    return locationWithMaxVotes;
}


void preparePoint(double RSSIs[NUMBER_OF_ANCHORS])
{
    for (int i = 0; i < NUMBER_OF_ANCHORS; ++i)
    {
        // Normalize the RSSI values using min-max scaling
        RSSIs[i] = (RSSIs[i] + 100) / 100.0;
    }

LOCATIONS validateScanningPhasePerLabel(std::vector<Data> scanResultSamepleForValidationPerLabel) {
}
