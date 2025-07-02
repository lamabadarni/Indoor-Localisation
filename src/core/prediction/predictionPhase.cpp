#include "predictionPhase.h"
#include "core/scanning/scanningPhase.h"
#include "core/dataManaging/data.h"
#include "core/utils/utilities.h"

static double _euclidean(const double* a, const double* b, int size);
static void   _preparePointRSSI(double* toBeNormalised, int n);
static void   _preparePointTOF(double TOF[NUMBER_OF_RESPONDERS], double toBeNormalised[NUMBER_OF_RESPONDERS]);
static Label  _predict(std::vector<double>& distances, std::vector<Label>& labels, int k);
static void  clearDataAfterPredectionFailure();
static Label staticRSSIPredict();
static Label dynamicRSSIPredict();
static Label tofPredict();

void runPredictionPhase(void) {
    LOG_INFO("PREDICT", " ");
    LOG_INFO("PREDICT", "=============== Prediction Phase Started ===============");

    int invalidLabels = 0;

    while ( !shouldAbort ) {
        if(invalidLabels >= PREDICTION_MAX_LABEL_FAILURE) {
            LOG_INFO("PREDICT", "Encountered failure at %d different labels", PREDICTION_MAX_LABEL_FAILURE);
            bool clearData = promptUserForClearingDataAfterManyPredectionFailure();
            if(clearData) {
                clearDataAfterPredectionFailure();
            }
            break;
        }

        LOG_INFO("PREDICT", "[PREDICT] >> Press Enter to start predicting...");
        readCharFromUser();

        bool retry = true;
        int count  = 1;

        while(retry) {

            bool success = startLabelPredectionSession();

            if(success) {
                 LOG_INFO("PREDICT", "Prediction Label Success :)");
                break;
            }
            else {

                if(count > PREDICTION_MAX_RETRIES) {
                    LOG_INFO("PREDICT", "Predection failed %d times", count);
                    LOG_INFO("PREDICT", "Move to another label to check overall data validity ...");
                    invalidLabels++;
                    break;
                }
                else {
                    LOG_INFO("PREDICT", "Data seems to be not valid ...");
                    LOG_INFO("PREDICT", "[ADVICE] Make sure anchor placement didn't changes since last data collection");
                    LOG_INFO("PREDICT", "[ADVICE] Make sure responders placement didn't changes since last data collection");
                }
            }

            delay_ms(USER_PROMPTION_DELAY);

            retry = promptUserRetryPrediction();

            if(retry) {
                LOG_INFO("PREDICT", "Retrying label predection after failure ...");
                count++;
            }
            else {
                retry = false;
            }
        }

        promptUserProceedToNextLabel();
        delay_ms(USER_PROMPTION_DELAY);
    }

    LOG_INFO("PREDICT", "Aborting predection phase");
}

bool startLabelPredectionSession() {

    LOG_INFO("PREDICT", "Starting label predection ...");

    Label predictLabel = predict();

    if((int)predictLabel == LABELS_COUNT) {
       LOG_INFO("PREDICT", "Prediction failed ..,");
       return false;
    }

    return true;
}

Label predict() {

    Label label = LABELS_COUNT;

    if(SystemSetup::currentSystemMode == MODE_PREDICTION_SESSION) {
        if( isStaticRSSIActiveForPrediction() ) {
            SystemSetup::currentSystemScannerMode = STATIC_RSSI;
        }
        if ( isDynamicRSSIActiveForPrediction() ) {
            SystemSetup::currentSystemScannerMode = DYNAMIC_RSSI;
        }
        if ( isTOFActiveForPrediction() ) {
            SystemSetup::currentSystemScannerMode = TOF;
        }
    }

    switch (SystemSetup::currentSystemPredictionMode) {
        case STATICRSSI: {
            label =  staticRSSIPredict();
            break;
        }
        case DYNAMICRSSI: {
            label =  dynamicRSSIPredict();
            break;
        }
        case TOfF: {
            label =  tofPredict();
            break;
        }
        case STATIC_RSSI_DYNAMIC_RSSI: {
            Label staticLabel  = staticRSSIPredict();
            Label dynamicLabel = dynamicRSSIPredict();
            if(staticLabel == dynamicLabel) {
                label = staticLabel;
                break;
            }
            LOG_ERROR("PREDICT", " !! CONFLICT !! Static Anchors RSSI Predict and Dynamic APs RSSI Predict differ: Static RSSI Prediction %s | Dynamic RSSI Prediction %s",
                        labels[staticLabel] , labels[dynamicLabel]);
            if(SystemSetup::logLevel == LOG_LEVEL_DEBUG) {
                Label user = promptUserChooseBetweenPredictions(staticLabel, dynamicLabel);
                if( user == staticLabel )  {
                    LOG_DEBUG("PREDICT", "Static Anchors RSSI Prediction valid");
                }
                if( user == dynamicLabel ) {
                    LOG_DEBUG("PREDICT", "Dynamic APs RSSI Prediction valid");
                }
            }
            break;
        }
        case DYNAMIC_RSSI_TOF: {
            Label tofLabel     = tofPredict();
            Label dynamicLabel = dynamicRSSIPredict();
            if(tofLabel == dynamicLabel) {
                label = tofLabel;
                break;
            }
            LOG_ERROR("PREDICT", " !! CONFLICT !! Time of flight Predict and Dynamic APs RSSI Predict differ: Time of flight Prediction %s | Dynamic RSSI Prediction %s ", 
                labels[tofLabel] , labels[dynamicLabel]);
            if(SystemSetup::logLevel == LOG_LEVEL_DEBUG) {
                Label user = promptUserChooseBetweenPredictions(tofLabel, dynamicLabel);
                if ( user == tofLabel ) {
                    LOG_DEBUG("PREDICT", "Time of flight Prediction valid");
                }
                if ( user == dynamicLabel ) {
                     LOG_DEBUG("PREDICT", "Dynamic APs RSSI Prediction valid");
                }
            }
            break;
        }
        case STATIC_RSSI_TOF: {
            Label staticLabel  = staticRSSIPredict();
            Label tofLabel     = tofPredict();
            if(staticLabel == tofLabel) {
                label = staticLabel;
                break;
            }
            LOG_ERROR("PREDICT", " !! CONFLICT !! Static Anchors RSSI Predict and Time of flight Predict Predict differ: Static RSSI Prediction %s | Time of flight Predict Prediction %s", 
                labels[staticLabel] , labels[tofLabel]);
            if(SystemSetup::logLevel == LOG_LEVEL_DEBUG) {
                Label user = promptUserChooseBetweenPredictions(staticLabel, tofLabel);
                if ( user == staticLabel ) {
                    LOG_DEBUG("PREDICT", "Static Anchors RSSI Prediction valid");
                }
                if ( user == tofLabel ) {
                    LOG_DEBUG("PREDICT", "Time of flight Prediction valid");
                }
            }
            break;
        }

        case STATIC_RSSI_DYNAMIC_RSSI_TOF: {
            Label staticLabel  = staticRSSIPredict();
            Label dynamicLabel = dynamicRSSIPredict();
            Label tofLabel     = tofPredict();

            if (staticLabel == dynamicLabel && dynamicLabel == tofLabel) {
                label = staticLabel;
                break;
            }

            LOG_ERROR("PREDICT", "!! TRIPLE CONFLICT !! Predictions differ:");
            LOG_ERROR("PREDICT", "Static: %s | Dynamic: %s | ToF: %s", labels[staticLabel], labels[dynamicLabel], labels[tofLabel]);

            if(SystemSetup::logLevel == LOG_LEVEL_DEBUG) {
                Label user = promptUserChooseBetweenTriplePredictions(staticLabel, dynamicLabel, tofLabel);
                LOG_DEBUG("PREDICT", "User selected prediction: %s", labels[user]);
                label = user;
            }
            break;
        }

        case SYSTEM_PREDICTION_NODES_NUM:
            break;
    }

    if(label != LABELS_COUNT) {
        LOG_INFO("PREDICT", " Predicted label: %s", labels[label]);
        delay_ms(USER_PROMPTION_DELAY);
        bool approve = promptUserApprovePrediction();
        if(approve) {
            return label;
        }
    }
    return label;
}

static Label staticRSSIPredict() {
    int sizeOfDataSet = staticRSSIDataSet.size();
    double normalisedInput[NUMBER_OF_ANCHORS];
    std::vector<double> distances(sizeOfDataSet, 0);
    std::vector<Label> labels(sizeOfDataSet, LABELS_COUNT);

    memcpy(normalisedInput, accumulatedStaticRSSIs, NUMBER_OF_DYNAMIC_APS);
    _preparePointRSSI(normalisedInput, NUMBER_OF_DYNAMIC_APS);

    for (int i = 0; i < sizeOfDataSet; ++i) {
        double normalisedPoint[NUMBER_OF_ANCHORS];

        memcpy(normalisedPoint, staticRSSIDataSet[i].RSSIs, NUMBER_OF_DYNAMIC_APS);
        _preparePointRSSI(normalisedPoint, NUMBER_OF_DYNAMIC_APS);

        distances[i] = _euclidean(normalisedPoint, normalisedInput, NUMBER_OF_ANCHORS);
        labels[i] = staticRSSIDataSet[i].label;
    }

    return _predict(distances, labels, K_RSSI);
}

static int _macAddrCmp(uint8_t mac1[MAC_ADDRESS_SIZE], uint8_t mac2[MAC_ADDRESS_SIZE]) {
    int cmp = 0;

    for (int i = 0 ; i < MAC_ADDRESS_SIZE ; i++) {
        cmp = mac1[i] - mac2[i];

        if (cmp) {
            return cmp;
        }
    }

    return cmp;
}

static double _dynamicRSSIeuclidean(double rssi1[NUMBER_OF_DYNAMIC_APS],
                            double rssi2[NUMBER_OF_DYNAMIC_APS],
                            uint8_t mac1[NUMBER_OF_DYNAMIC_APS][MAC_ADDRESS_SIZE],
                            uint8_t mac2[NUMBER_OF_DYNAMIC_APS][MAC_ADDRESS_SIZE]) {
    int i_1 = 0, i_2 = 0, exactMatch = 0;
    double sum = 0;

    while (i_1 < NUMBER_OF_DYNAMIC_APS && i_2 < NUMBER_OF_DYNAMIC_APS) {
        int cmp = _macAddrCmp(mac1[i_1], mac2[i_2]);

        if (cmp > 0) {
            i_1++;
        }
        else if (cmp < 0) {
            i_2++;
        }
        else {
            sum += (rssi1[i_1] - rssi2[i_2])*(rssi1[i_1] - rssi2[i_2]);

            i_1++;
            i_2++;
            exactMatch++;
        }
    }

    sum += NUMBER_OF_DYNAMIC_APS - exactMatch;

    return sqrt(sum);
}

static Label dynamicRSSIPredict() {
    int sizeOfDataSet = dynamicRSSIDataSet.size();
    double normalisedInput[NUMBER_OF_DYNAMIC_APS];
    std::vector<double> distances(sizeOfDataSet, 0);
    std::vector<Label> labels(sizeOfDataSet, LABELS_COUNT);

    memcpy(normalisedInput, accumulatedDynamicRSSIs, NUMBER_OF_DYNAMIC_APS);
    _preparePointRSSI(normalisedInput, NUMBER_OF_DYNAMIC_APS);

    for (int i = 0 ; i < sizeOfDataSet ; i++) {
        double normalisedPoint[NUMBER_OF_DYNAMIC_APS];

        memcpy(normalisedPoint, dynamicRSSIDataSet[i].RSSIs, NUMBER_OF_DYNAMIC_APS);
        _preparePointRSSI(normalisedPoint, NUMBER_OF_DYNAMIC_APS);
        distances[i] = _dynamicRSSIeuclidean(normalisedInput, normalisedPoint,
                       accumulatedMacAddresses, dynamicMacDataSet[i].macAddresses);
        labels[i] = staticRSSIDataSet[i].label;
    }

    return _predict(distances, labels, K_RSSI);
}

static Label tofPredict() {
    int sizeOfDataSet = tofDataSet.size();
    double normalisedInput[NUMBER_OF_RESPONDERS];
    std::vector<double> distances(sizeOfDataSet, 0);
    std::vector<Label> labels(sizeOfDataSet, LABELS_COUNT);

    _preparePointTOF(accumulatedTOFs, normalisedInput);

    for (int i = 0; i < sizeOfDataSet; ++i) {
        double normalisedPoint[NUMBER_OF_RESPONDERS];
        _preparePointTOF(tofDataSet[i].TOFs, normalisedPoint);
        distances[i] = _euclidean(normalisedPoint, normalisedInput, NUMBER_OF_RESPONDERS);
        labels[i] = tofDataSet[i].label;
    }

    return _predict(distances, labels, K_TOF);
}

static void clearDataAfterPredectionFailure() {
    DeleteBufferedData::scanner = SystemSetup::currentSystemScannerMode;
}

static double _euclidean(const double* a, const double* b, int size) {
    double sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sqrt(sum);
}

static void _preparePointRSSI(double* toBeNormalised, int n) {
    for (int i = 0 ; i < n ; ++i) {
        toBeNormalised[i] = ((double)(toBeNormalised[i] + 100)) / 100.0;
    }
}

static void _preparePointTOF(double TOF[NUMBER_OF_RESPONDERS], double toBeNormalised[NUMBER_OF_RESPONDERS]) {
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