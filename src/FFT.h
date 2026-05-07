#pragma once

#include "arduinoFFT.h"
#include <driver/adc.h>
#include <soc/sens_reg.h>
#include <soc/sens_struct.h>

// arbitraty value, tune as needed
#define MAX_FREQUENCY_VALUE 30

#define SAMPLES 512 // power of 2
#define SAMPLING_FREQ 40000 // 12 kHz Fmax = sampleF /2
#define AMPLITUDE 100 // sensitivity
#define FREQUENCY_BANDS 14
#define TOTAL_BANDS 20
// Number of valid bands after `calculateBands` skips the lowest 3 sub-audible
// raw bands and reverses the order. Indices 0..4 are highs, 5..7 mids, 8..10 bass.
#define BAND_COUNT 11

// Per-frame decay for the visual envelope follower. Larger = slower decay
// (more "languid"), smaller = punchier. Frame rate is ~20-25 fps so 0.85
// gives a ~200 ms half-life.
#define ENVELOPE_DECAY 0.7f
// Per-frame decay for the AGC max tracker. 0.995 over ~22 fps ~= 6 s half-life,
// so the visualization re-calibrates to room volume on that timescale.
#define AGC_DECAY 0.995f
// Lower bound on the AGC normalizer so genuine silence isn't amplified to noise.
#define AGC_FLOOR 0.05f

// Beat detector. A beat fires when the post-AGC bass exceeds a slowly-tracked
// running average by BEAT_THRESHOLD, and only if it's also above BEAT_MIN_LEVEL
// (so quiet passages don't produce phantom beats from tiny fluctuations).
// BEAT_REFRACTORY_FRAMES suppresses re-triggering immediately after a beat —
// at ~22 fps, 5 frames is roughly 230 ms.
#define BEAT_THRESHOLD 1.5f
#define BEAT_MIN_LEVEL 0.25f
#define BEAT_REFRACTORY_FRAMES 5
// How fast the bass running-average tracks the input. 0.95 ≈ ~1 s memory,
// so a steady beat establishes the baseline but a single kick still spikes above it.
#define BEAT_AVG_DECAY 0.95f

#define MIC_PIN A0

class FFT {
public:
    FFT();
    void calculatePercentages();
    void getBands(float out[BAND_COUNT]) const;
    bool beatDetected() const { return beatThisFrame; }
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

    // Per-band normalized dB values in [0, MAX_FREQUENCY_VALUE], filled by calculateBands.
    int frequencyBandMaxDb[BAND_COUNT] = {0};

    // Per-band post-AGC, post-envelope-followed values in [0, 1]. Read by getBands.
    float smoothedBands[BAND_COUNT] = {0};
    float agcMax = AGC_FLOOR;

    float bassRunningAvg = 0.0f;
    int beatRefractoryLeft = 0;
    bool beatThisFrame = false;
};