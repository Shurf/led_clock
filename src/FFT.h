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

// Per-frame decay for the visual envelope follower. Larger = slower decay
// (more "languid"), smaller = punchier. Frame rate is ~20-25 fps so 0.85
// gives a ~200 ms half-life.
#define ENVELOPE_DECAY 0.7f
// Per-frame decay for the AGC max tracker. 0.995 over ~22 fps ~= 6 s half-life,
// so the visualization re-calibrates to room volume on that timescale.
#define AGC_DECAY 0.995f
// Lower bound on the AGC normalizer so genuine silence isn't amplified to noise.
#define AGC_FLOOR 0.05f

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

    float smoothedRed = 0.0f;
    float smoothedGreen = 0.0f;
    float smoothedBlue = 0.0f;
    float agcMax = AGC_FLOOR;
};