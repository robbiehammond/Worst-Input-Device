#include <MPU6050.h>
#include <queue>
#include "Action.h"
bool TEST = false; //to check sensor sampling rate

int16_t rawVals[6]; //where raw data is read to
std::array<MPU6050, NUM_SENSORS> sensors;
SensorState curState; //written to over and over again for a copy to get pushed into samples.
std::array<SensorState, MAX_RECORDED_SAMPLES> capturedStates;
int sampleInd = 0;


//read current sensor data, compile it into the SensorSample. After this is called, the curSample has the most up-to-date data.
void updateSensorData() {
    for (int sensorNum = 0; sensorNum < NUM_SENSORS; sensorNum++) {
        auto& sensor = sensors[sensorNum];
        sensor.getMotion6(&rawVals[iAx], &rawVals[iAy], &rawVals[iAz], 
                          &rawVals[iGx], &rawVals[iGy], &rawVals[iGz]);

        curState.set(sensorNum, rawVals);
    };
}

//Give it it's timestamp of when it was taken, and put it into the capturedSamples list.
void recordSensorData() {
    curState.finalizeSample();
    capturedStates[sampleInd] = curState;
    curState.resetSample();
}


/*
Loop through all the sensors and check their most recently recorded y acceleration. If it abruptly changes 
in whatever direction is up, it was probably a keypress.

Probably could just be done immediately in updateSensorData() so we don't need to loop over everything again. Eh it's probably fine.
*/
bool possiblePress() {
    for (int sensorNum = 0; sensorNum < NUM_SENSORS; sensorNum++) {
        int16_t curYAcc = curState.get(sensorNum).ay;
        if (curYAcc >= -1 * THRESHOLD) {
            return true;
        }
    }
    return false;
}


void setup() {
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    Serial.begin(9600);

    for (auto& sensor : sensors) {
        sensor.initialize();
    }
}

void checkSampleRate() {
    long int t1 = millis();
    sensors[0].getMotion6(&rawVals[iAx], &rawVals[iAy], &rawVals[iAz], 
                        &rawVals[iGx], &rawVals[iGy], &rawVals[iGz]);
    Serial.println(millis() - t1);
}

void loop() {
    if (TEST) {
        checkSampleRate();
        return;
    }


    updateSensorData();

    recordSensorData();

    if (possiblePress()) {

        //TODO: This is a bug to fix: If sampleInd is at the beginning and we have an action, it won't be registered.
        //This is because we just look backwards in the sample list to get the last few samples.
        if (sampleInd >= SAMPLES_PER_ACTION) {
            std::array<SensorState, SAMPLES_PER_ACTION> statesToUse;
            for (int i = 0; i < SAMPLES_PER_ACTION; i++) {
                //just so that samplesToUse[0] is oldest, samplesToUse[1] is next oldest, etc
                statesToUse[SAMPLES_PER_ACTION - 1 - i] = capturedStates[sampleInd - i];
            }

            Action action(statesToUse);
            action.writeOut(WriteOption::SERIAL_OUT);

        }
    }


    sampleInd = (sampleInd + 1) % MAX_RECORDED_SAMPLES;
}