#pragma once

#include "arduinoFFT.h"
#include <driver/adc.h>
#include <soc/sens_reg.h>
#include <soc/sens_struct.h>

// arbitraty value, tune as needed
#define MAX_FREQUENCY_VALUE 30

#define SAMPLES 1024 // power of 2
#define SAMPLING_FREQ 40000 // 12 kHz Fmax = sampleF /2 
#define AMPLITUDE 100 // sensitivity
#define FREQUENCY_BANDS 14
#define TOTAL_BANDS 20

#define MIC_PIN A0

class FFT {
public:
    FFT();
    void calculatePercentages(float & redPercentage, float & greenPercentage, float & bluePercentage);
private:

    int IRAM_ATTR local_adc1_read(int channel);
    void takeSamples();    
    float percentage(int value, int numBands);
    void computeFFT();
    void calculateBands();

    double vImag[SAMPLES];
    double vReal[SAMPLES];
    unsigned long sampling_period_us;

    double cutoffFrequencies[FREQUENCY_BANDS];
    ArduinoFFT<double>* fft;
    float reference;

    int redBandValue = 0;
    int greenBandValue = 0;
    int blueBandValue = 0;

    int maxRed = 0;
    int maxGreen = 0;
    int maxBlue = 0;
};