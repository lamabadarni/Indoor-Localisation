#include "predictionPhase.h"
#include "core/scanning/scanningPhase.h"
#include "core/dataManaging/data.h"
#include "core/utils/utilities.h"
#include "core/ui/userUISerial.h"

#define K_DYNAMIC_RSSI (12)

static double _euclidean(const double* a, const double* b, int size);
static void   _preparePointRSSI(double* toBeNormalised, int n);
static void   _preparePointTOF(double TOF[NUMBER_OF_RESPONDERS], double toBeNormalised[NUMBER_OF_RESPONDERS]);
static Label  _predict(std::vector<std::pair<double, Label>>& distLabel, int k);
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
        readIntFromUserSerial();

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
            createSingleScan();
            label =  dynamicRSSIPredict();
            break;
        }
        case TOfF: {
            label =  tofPredict();
            break;
        }

        case SYSTEM_PREDICTION_NODES_NUM:
        LOG_ERROR("PREDICT", "should not be here");
            break;
    }

    if(label != LABELS_COUNT) {
        LOG_INFO("PREDICT", " Predicted label: %s", labels[label].c_str());
        delay_ms(USER_PROMPTION_DELAY);
        bool approve = promptUserApprovePrediction();
        if(approve) {
            return label;
        }
    }
    return LABELS_COUNT;
}

static Label staticRSSIPredict() {
    int sizeOfDataSet = staticRSSIDataSet.size();
    double normalisedInput[NUMBER_OF_ANCHORS];
    std::vector<std::pair<double, Label>> distLabel(sizeOfDataSet, {NUMBER_OF_ANCHORS, LABELS_COUNT});

    memcpy(normalisedInput, accumulatedStaticRSSIs, NUMBER_OF_DYNAMIC_APS);
    _preparePointRSSI(normalisedInput, NUMBER_OF_DYNAMIC_APS);

    for (int i = 0; i < sizeOfDataSet; ++i) {
        double normalisedPoint[NUMBER_OF_ANCHORS];

        memcpy(normalisedPoint, staticRSSIDataSet[i].RSSIs, sizeof(normalisedPoint));
        _preparePointRSSI(normalisedPoint, NUMBER_OF_ANCHORS);

        distLabel[i].first = _euclidean(normalisedPoint, normalisedInput, NUMBER_OF_ANCHORS);
        distLabel[i].second = staticRSSIDataSet[i].label;
    }

    return _predict(distLabel, K_RSSI);
}

static int _macAddrCmp(uint8_t mac1[MAC_ADDRESS_SIZE], uint8_t mac2[MAC_ADDRESS_SIZE]) {
    char macStr1[18];
    char macStr2[18];
    sprintf(macStr1, "%02X:%02X:%02X:%02X:%02X:%02X",
                    mac1[0], mac1[1], mac1[2],
                    mac1[3], mac1[4], mac1[5]);
    sprintf(macStr2, "%02X:%02X:%02X:%02X:%02X:%02X",
                mac2[0], mac2[1], mac2[2],
                mac2[3], mac2[4], mac2[5]);

    return strcmp(macStr1, macStr2);
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
            i_2++;
        }
        else if (cmp < 0) {
            i_1++;
        }
        else {
            sum += (rssi1[i_1] - rssi2[i_2])*(rssi1[i_1] - rssi2[i_2]);

            i_1++;
            i_2++;
            exactMatch++;
        }
    }

    sum += NUMBER_OF_DYNAMIC_APS - exactMatch;

    return sqrt(abs(sum));
}

static Label dynamicRSSIPredict() {
    double normalisedInput[NUMBER_OF_DYNAMIC_APS];
    std::vector<std::pair<double, Label>> distLabel(K_DYNAMIC_RSSI, {4, LABELS_COUNT});

    memcpy(normalisedInput, accumulatedDynamicRSSIs, NUMBER_OF_DYNAMIC_APS*sizeof(double));
    _preparePointRSSI(normalisedInput, NUMBER_OF_DYNAMIC_APS);

    while (true) {
        bool res = loadBatch();
        int sizeOfDataSet = dynamicRSSIDataSet.size();

        for (int i = sizeOfDataSet - 1 ; i >= 0 ; i--) {
            double normalisedPoint[NUMBER_OF_DYNAMIC_APS];

            for (int j = 0 ; j < NUMBER_OF_DYNAMIC_APS ; j++) {
                normalisedPoint[j] = (double)dynamicRSSIDataSet[i].RSSIs[j];
            }

            _preparePointRSSI(normalisedPoint, NUMBER_OF_DYNAMIC_APS);

            double distance = _dynamicRSSIeuclidean(normalisedInput, normalisedPoint,
                                                    accumulatedMacAddresses,
                                                    dynamicMacDataSet[i].macAddresses
                                                );

            if (distance < distLabel[distLabel.size() - 1].first) {
                distLabel[distLabel.size() - 1].first = distance;
                distLabel[distLabel.size() - 1].second = dynamicRSSIDataSet[i].label;
            }

            int maxDistLabelIndex = 0;
            // find pair with max distance and put at the end of array
            for (int  c = 1; c < distLabel.size() ; c++) {
                const auto& distLabelPair = distLabel[c];

                if (distLabel[maxDistLabelIndex].first < distLabelPair.first) {
                    maxDistLabelIndex = c;
                }
            }

            std::swap(distLabel[maxDistLabelIndex], distLabel[distLabel.size() - 1]);

        }

        dynamicRSSIDataSet.clear();
        dynamicMacDataSet.clear();

        if (!res) break;
    }

    // when we reach this point distLabel should have the top 7 distances relative to the dataset
    int labelVotes[LABELS_COUNT] = {0};
    Label labelWithMaxVotes = (Label)0;

    LOG_DEBUG("PREDICT", "TOP %d DISTANCES", distLabel.size());

    for (int i = 0 ; i < distLabel.size() ; i++) {
        LOG_DEBUG("PREDICT", "Distance = %f, Label = %s", distLabel[i].first, labels[distLabel[i].second].c_str());
        labelVotes[distLabel[i].second]++;

        if (labelVotes[labelWithMaxVotes] < labelVotes[distLabel[i].second]) {
            labelWithMaxVotes = distLabel[i].second;
        }
    }

    return labelWithMaxVotes;
}

static Label tofPredict() {
    int sizeOfDataSet = tofDataSet.size();
    double normalisedInput[NUMBER_OF_RESPONDERS];
    std::vector<std::pair<double, Label>> distLabel(sizeOfDataSet, {4, LABELS_COUNT});

    _preparePointTOF(accumulatedTOFs, normalisedInput);

    for (int i = 0; i < sizeOfDataSet; ++i) {
        double normalisedPoint[NUMBER_OF_RESPONDERS];
        _preparePointTOF(tofDataSet[i].TOFs, normalisedPoint);
        distLabel[i].first = _euclidean(normalisedPoint, normalisedInput, NUMBER_OF_RESPONDERS);
        distLabel[i].second = tofDataSet[i].label;
    }

    return _predict(distLabel, K_TOF);
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
        toBeNormalised[i] = ((double)(toBeNormalised[i] + 100.0)) / 100.0;
    }
}

static void _preparePointTOF(double TOF[NUMBER_OF_RESPONDERS], double toBeNormalised[NUMBER_OF_RESPONDERS]) {
    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        toBeNormalised[i] = TOF[i] / 2000;
    }
}

static Label _predict(std::vector<std::pair<double, Label>>& distLabel, int k) {
    int closestLabel[LABELS_COUNT] = {0};
    Label labelWithMaxVotes = (Label)0;

    std::sort(distLabel.begin(), distLabel.end(),
              [](const std::pair<double, Label>& a, const std::pair<double, Label>& b) {
              return a.first < b.first;
            });

    for (int i = 0; i < k; ++i) {
        closestLabel[distLabel[i].second]++;
        labelWithMaxVotes = closestLabel[distLabel[i].second] > closestLabel[labelWithMaxVotes] ?
                            distLabel[i].second :
                            labelWithMaxVotes;
    }

    return labelWithMaxVotes;
}