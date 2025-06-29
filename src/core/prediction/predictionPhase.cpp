#include "predictionPhase.h"
#include "core/scanning/scanningPhase.h"
#include "core/dataManaging/data.h"

static double _euclidean(const double* a, const double* b, int size);
static void _preparePointRSSI(double toBeNormalised[NUMBER_OF_ANCHORS]);
static void _preparePointTOF(double TOF[NUMBER_OF_RESPONDERS], double toBeNormalised[NUMBER_OF_RESPONDERS]);
static Label _predict(std::vector<double>& distances, std::vector<Label>& labels, int k);
static Label rssiPredict();
static Label tofPredict();

static ScannerFlag flag = NONE;
#define accuracy_ (flag == STATICRSSI ? rssiAccuracy : tofAccuracy)

void runPredictionPhase(void) {
    LOG_INFO("PREDICT", " ");
    LOG_INFO("PREDICT", "=============== Prediction Phase Started ===============");

    int invalidLabels = 0;
    bool cont = true;

    while (!shouldAbort && cont) {
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
                break;
            }
        }

        cont = promptUserProceedToNextLabel();
        LOG_INFO("PREDICT", "Move to next label to start predecting ...");
        doneCollectingData();
        delay_ms(USER_PROMPTION_DELAY);
    }

    //TODO: should print predection summary
    LOG_INFO("PREDICT", "Aborting predection phase");
}

// Ward Lama :: need implementation -- should update validForPredection and make user choose only from them
// update data
bool clearDataAfterPredectionFailure() {
    return true;
}

bool startLabelPredectionSession() {

    LOG_INFO("PREDICT", "Starting label predection ...");

    createSingleScan();

    Label predictLabel = predict();

    if((int)predictLabel == LABELS_COUNT) {
       LOG_INFO("PREDICT", "Prediction failed ..,");
       if(flag == NONE) {
        LOG_DEBUG("PREDICT", "Both rssi prediction and tof prediction failed");
       }
       return false;
    }
    else if(SystemSetup::currentSystemScannerMode == STATIC_RSSI_TOF) {
        BufferedData::scanner = flag;
        BufferedData::lastN++;

        RSSIData rssiData;
        rssiData.label = predictLabel;
        for(int i = 0; i < NUMBER_OF_ANCHORS; i++) {
            rssiData.RSSIs[i] = accumulatedRSSIs[i];
        }

        TOFData tofData;
        tofData.label = predictLabel;
        for (int i =0; i > NUMBER_OF_RESPONDERS; i++) {
            tofData.TOFs[i] = accumulatedTOFs[i];
        }

        saveData(rssiData);
        saveData(tofData);
    }
 
    return true;
}

Label predict() {

    Label label = LABELS_COUNT;

    switch (SystemSetup::currentSystemScannerMode) {
        case STATIC_RSSI: {
            flag  = STATICRSSI;
            label =  createSamplePredict();
            break;
        }

        case TOF: {
            flag  = TOF_;
            label =  createSamplePredict();
            break;
        }

        case STATIC_RSSI_TOF: {
            flag = STATICRSSI;
            Label rssiLabel = createSamplePredict();
            flag = TOF_;
            Label tofLabel  = createSamplePredict();

            if(rssiLabel == tofLabel) {
                label = rssiLabel;
                flag = BOTH;
                break;
            }
        
            // Ward :: here , should we add weighting ?? 
            LOG_ERROR("PREDICT", " !! CONFLICT !! RSSI and TOF predictions differ:");
            LOG_ERROR("PREDICT", " ! RSSI Prediction: %s, with accuracy: %d", labels[rssiLabel], rssiAccuracy[rssiLabel]);
            LOG_ERROR("PREDICT", " ! TOF  Prediction: %s, with accuracy: %d", labels[tofLabel],  tofAccuracy[tofLabel]);
                
            Label user = promptUserChooseBetweenPredictions(rssiLabel, tofLabel);
            if(user == rssiLabel)      { flag = STATICRSSI; tofAccuracy[tofLabel]   = 0;}
            else if(user == tofLabel)  { flag = TOF_;        rssiAccuracy[rssiLabel] = 0;}
            else { flag = NONE; tofAccuracy[tofLabel] = 0; rssiAccuracy[rssiLabel] = 0;}
            label = user;
            return label;
        }

        case SYSTEM_SCANNER_MODES_NUM:
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

Label createSamplePredict() {
    Label samples[PREDICTION_SAMPLES];
    int c = 1;
    for(int i = 0; i < PREDICTION_SAMPLES; i++) {
        samples[i] = flag == STATICRSSI ? rssiPredict() : tofPredict() ;
        if(i > 0 && samples[i] == samples[i-1]) c++;
        if(c > PREDICTION_SAMPLES_THRESHOLD) return samples[i];
    }

    int most[LABELS_COUNT] = {0};
    for(int i = 0; i < PREDICTION_SAMPLES; i++) {
        most[samples[i]]++;
    }

    int maxIndex = 0;
    for (int i = 1; i < LABELS_COUNT; ++i) {
        if (most[i] > most[maxIndex]) {
            maxIndex = i;
        }
    }

    int labelNumHaveMax = 0;
    for (int i = maxIndex; i < LABELS_COUNT; ++i) {
        if(most[maxIndex] == most[i]) {
            labelNumHaveMax++;
        }
    }

    if(labelNumHaveMax > 1) {
        return LABELS_COUNT;
    }

    if(flag == STATIC_RSSI) {
        rssiAccuracy[maxIndex] = most[maxIndex] / PREDICTION_SAMPLES;
    }
    else if (flag == TOF_) {
        tofAccuracy[maxIndex] = most[maxIndex] / PREDICTION_SAMPLES;
    }

    return (Label)maxIndex;
}

static Label rssiPredict() {
    int sizeOfDataSet = rssiDataSet.size();
    double normalisedInput[NUMBER_OF_ANCHORS];
    std::vector<double> distances(sizeOfDataSet, 0);
    std::vector<Label> labels(sizeOfDataSet, LABELS_COUNT);

    _preparePointRSSI(normalisedInput);
    for (int i = 0; i < sizeOfDataSet; ++i) {
        double normalisedPoint[NUMBER_OF_ANCHORS];
        _preparePointRSSI(normalisedPoint);
        distances[i] = _euclidean(normalisedPoint, normalisedInput, NUMBER_OF_ANCHORS);
        labels[i] = rssiDataSet[i].label;
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


static double _euclidean(const double* a, const double* b, int size) {
    double sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sqrt(sum);
}

static void _preparePointRSSI(double toBeNormalised[NUMBER_OF_ANCHORS]) {
    for (int i = 0; i < NUMBER_OF_ANCHORS; ++i) {
        toBeNormalised[i] = ((double)(accumulatedRSSIs[i] + 100)) / 100.0;
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