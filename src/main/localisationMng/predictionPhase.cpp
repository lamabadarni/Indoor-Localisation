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


static void preparePointRSSI(double RSSIs[NUMBER_OF_ANCHORS])
{
    for (int i = 0; i < NUMBER_OF_ANCHORS; ++i)
    {
        // Normalize the RSSI values using min-max scaling
        RSSIs[i] = (RSSIs[i] + 100) / 100.0;
    }
}

static void preparePointTOF(double TOF[NUMBER_OF_RESPONDERS])
{
    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i)
    {
        // Normalize the TOF values using min-max scaling
        TOF[i] = TOF[i] / 2000;
    }
}

static Label _predict(const vector<double> &distances, const vector<Label> &labels)
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

Label rssiPredict(double input[NUMBER_OF_ANCHORS])
{
    int sizeOfDataSet = rssiDataSet.size();
    vector<double> distances(sizeOfDataSet, 0);
    vector<Label> labels(sizeOfDataSet, NOT_ACCURATE);

    // Calculate distances and store corresponding labels
    for (int i = 0 ; i < sizeOfDataSet ; ++i) {
        distances[i] = euclidean(preparePoint(rssiDataSet[i].RSSIs), input, sizeOfInput);
        labels[i] = dataSet[i].label;
    }

    return _predict(distances, labels);
}

Label tofPredict(double input[NUMBER_OF_RESPONDERS])
{
    int sizeOfDataSet = tofDataSet.size();
    vector<double> distances(sizeOfDataSet, 0);
    vector<Label> labels(sizeOfDataSet, NOT_ACCURATE);

    // Calculate distances and store corresponding labels
    for (int i = 0 ; i < sizeOfDataSet ; ++i) {
        distances[i] = euclidean(tofDataSet[i].TOFs, input, sizeOfInput);
        labels[i] = dataSet[i].label;
    }

    return _predict(distances, labels);
}

bool isBackupDataSetRelevant(void)
{
    if (currentSystemState == SystemState::STATIC_RSSI_TOF && tofDataSet.size() < 10 * K ||
        rssiDataSet.size() < 10 * K)
    {
        return false; // Not enough data for KNN
    }

    unsigned int numOfLabelInRSSI[NUMBER_OF_LABELS] = {0};
    unsigned int numOfLabelInTOF[NUMBER_OF_LABELS] = {0};
    bool isDataSetValid = false;
    int sizeOfDataSet = dataSet.size();

    for (const RSSIData &data : rssiDataSet)
    {
        numOfLabelInRSSI[data.label]++;
    }

    for (const TOFData &data : tofDataSet)
    {
        numOfLabelInTOF[data.label]++;
    }

    for (int i = 0; i < NUMBER_OF_LABELS; ++i)
    {
        currentScanningLabel = (Label)i;
        if (numOfLabelInRSSI[i] >= 3 * K && numOfLabelInTOF[i] >= 3 * K && validateScanAccuracy())
        {
            reuseFromSD[i] = true;
            isDataSetValid = true;
        }
        else
        {
            Serial.println("Backup dataset is not relevant for location: " + labelToString(i));
        }
    }

    return isDataSetValid;
}

/**
 * @brief Validates the scan results by checking how many predictions match the label.
 *        Uses both RSSI and TOF modules if enabled. If failed, offers fallback.
 */
bool validateScanAccuracy() {
    int matchesRSSI = 0;
    int matchesTOF = 0;
    bool combinedOK = false;

    // Run RSSI and/or TOF predictions depending on system state
    switch (Enablements::currentSystemState) {
        case STATIC_RSSI:
            matchesRSSI = computeRSSIPredictionMatches();
            break;

        case STATIC_RSSI_TOF:
            matchesRSSI = computeRSSIPredictionMatches();
            matchesTOF = computeTOFPredictionMatches();
            break;

        case STATIC_DYNAMIC_RSSI:
            matchesRSSI = computeRSSIPredictionMatches();
            break;

        case STATIC_DYNAMIC_RSSI_TOF:
            matchesRSSI = computeRSSIPredictionMatches();
            matchesTOF = computeTOFPredictionMatches();
            break;

        default:
            break;
    }

    int totalMatches = matchesRSSI + matchesTOF;
    int totalAttempts = (matchesTOF > 0) ? 2 * SCAN_VALIDATION_SAMPLE_SIZE : SCAN_VALIDATION_SAMPLE_SIZE;
    scanAccuracy = (100 * totalMatches) / totalAttempts;

    Serial.printf("Scanning Phase: combined accuracy = %d%% (%d/%d correct predictions)\n",
                  scanAccuracy, totalMatches, totalAttempts);

    combinedOK = (totalMatches >= VALIDATION_PASS_THRESHOLD);

    if (combinedOK && promptUserAccuracyApprove()) return true;

    // Combined validation failed — fallback prompt
    int choice = promptRetryValidationWithSingleMethod();

    if (choice == 1) {
        Serial.println("Retrying validation with RSSI only...");
        matchesRSSI = computeRSSIPredictionMatches();
        scanAccuracy = (100 * matchesRSSI) / SCAN_VALIDATION_SAMPLE_SIZE;
        Serial.printf("[RSSI] Retry accuracy = %d%%\n", scanAccuracy);
        return (matchesRSSI >= VALIDATION_PASS_THRESHOLD) && promptUserAccuracyApprove();
    }

    if (choice == 2) {
        Serial.println("Retrying validation with TOF only...");
        matchesTOF = computeTOFPredictionMatches();
        scanAccuracy = (100 * matchesTOF) / SCAN_VALIDATION_SAMPLE_SIZE;
        Serial.printf("[TOF] Retry accuracy = %d%%\n", scanAccuracy);
        return (matchesTOF >= VALIDATION_PASS_THRESHOLD) && promptUserAccuracyApprove();
    }

    Serial.println("User aborted validation.");
    return false;
}
