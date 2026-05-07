#include "FFT.h"
#include <Arduino.h>

FFT::FFT()
{
    pinMode(MIC_PIN, INPUT);
    fft = new ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQ);
    sampling_period_us = (1.0 / SAMPLING_FREQ) * pow(10.0, 6);

    // adjust reference to get remove background noise
    reference = log10(10000.0);

    // Calculate cuttoff frequencies,meake a logarithmic scale base basePOt
    double basePot = pow(SAMPLING_FREQ / 2.0, 1.0 / FREQUENCY_BANDS);
    cutoffFrequencies[0] = basePot;
    for (int i = 1; i < FREQUENCY_BANDS; i++)
    {
        cutoffFrequencies[i] = basePot * cutoffFrequencies[i - 1];
    }
}

int IRAM_ATTR FFT::local_adc1_read(int channel)
{
    uint16_t adc_value;
    SENS.sar_meas_start1.sar1_en_pad = (1 << channel); // only one channel is selected
    while (SENS.sar_slave_addr1.meas_status != 0);

    SENS.sar_meas_start1.meas1_start_sar = 0;
    SENS.sar_meas_start1.meas1_start_sar = 1;
    while (SENS.sar_meas_start1.meas1_done_sar == 0);
    adc_value = SENS.sar_meas_start1.meas1_data_sar;
    return adc_value;
}

void FFT::takeSamples()
{
    adc1_get_raw(ADC1_CHANNEL_0);
    for (int i = 0; i < SAMPLES; i++)
    {
        unsigned long newTime = micros();
        //int value = analogRead(MIC_PIN);
        int value = local_adc1_read(ADC1_CHANNEL_0);
        vReal[i] = value;
        vImag[i] = 0;
        while (micros() < (newTime + sampling_period_us))
        {
            yield();
        }
    }
}

float FFT::percentage(int value, int numBands)
{
    return ((float)value) / (numBands * MAX_FREQUENCY_VALUE);
}

void FFT::computeFFT()
{
    fft->dcRemoval();
    fft->windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    fft->compute(FFT_FORWARD);
    fft->complexToMagnitude();
}

void FFT::calculateBands()
{
    double max[TOTAL_BANDS];
    int index = 0;
    double hzPerSample = (1.0 * SAMPLING_FREQ) / SAMPLES;
    double hz = 0;

    memset(max, 0, sizeof(max));
    memset(frequencyBandMaxDb, 0, sizeof(frequencyBandMaxDb));

    for (int i = 2; i < (SAMPLES / 2); i++)
    {
        if (vReal[i] > max[index])
            max[index] = vReal[i];
        if (hz > cutoffFrequencies[index])
        {
            index++;
            max[index] = 0;
        }
        hz += hzPerSample;
    }

    // Walk the raw bands in reverse so the highest-frequency band lands at
    // index 0 of the output. Skips the bottom 3 raw bands (~2-9 Hz, sub-audible).
    int frequencyBandIndex = 0;
    for (int i = FREQUENCY_BANDS - 1; i >= 3; i--)
    {
        int frequencyMaxNormalizedDb = 0;
        if (max[i] > 0)
            frequencyMaxNormalizedDb = 20.0 * (log10(max[i]) - reference);

        if (frequencyMaxNormalizedDb < 0)
            frequencyMaxNormalizedDb = 0;
        if (frequencyMaxNormalizedDb >= MAX_FREQUENCY_VALUE)
            frequencyMaxNormalizedDb = MAX_FREQUENCY_VALUE;

        frequencyBandMaxDb[frequencyBandIndex] = frequencyMaxNormalizedDb;
        frequencyBandIndex++;
    }
}

void FFT::calculatePercentages()
{
    takeSamples();
    computeFFT();
    calculateBands();

    // Convert each band's clamped dB integer (in [0, MAX_FREQUENCY_VALUE]) to a
    // raw [0, 1] percentage. Track the loudest band and the bass-band sum
    // (indices 8..10) for AGC and beat detection respectively.
    float rawBands[BAND_COUNT];
    float instantPeak = 0.0f;
    float bassSum = 0.0f;
    for (int i = 0; i < BAND_COUNT; i++)
    {
        rawBands[i] = (float)frequencyBandMaxDb[i] / MAX_FREQUENCY_VALUE;
        if (rawBands[i] > instantPeak) instantPeak = rawBands[i];
        if (i >= 8) bassSum += rawBands[i];
    }
    float rawBass = bassSum / 3.0f;

    // AGC: track the recent peak across all bands and normalize to it. Fast
    // attack (jump up immediately) and slow decay so the visualization
    // auto-calibrates to room volume without pumping on every transient.
    if (instantPeak > agcMax)
        agcMax = instantPeak;
    else
        agcMax = max(agcMax * AGC_DECAY, AGC_FLOOR);

    float gain = 1.0f / agcMax;
    for (int i = 0; i < BAND_COUNT; i++)
        rawBands[i] = min(rawBands[i] * gain, 1.0f);
    rawBass = min(rawBass * gain, 1.0f);

    // Beat detection on the post-AGC bass average, before the envelope smooths
    // anything. A kick shows up as a sudden spike above the slow running
    // average; the refractory counter prevents the same kick from
    // double-counting while it decays.
    beatThisFrame = false;
    if (beatRefractoryLeft > 0)
        beatRefractoryLeft--;
    else if (rawBass > BEAT_MIN_LEVEL && rawBass > bassRunningAvg * BEAT_THRESHOLD)
    {
        beatThisFrame = true;
        beatRefractoryLeft = BEAT_REFRACTORY_FRAMES;
    }
    bassRunningAvg = bassRunningAvg * BEAT_AVG_DECAY + rawBass * (1.0f - BEAT_AVG_DECAY);

    // Per-band envelope follower: peak attack, exponential decay. Keeps
    // transients visible and stops the ring from twitching back to dark
    // between drum hits.
    for (int i = 0; i < BAND_COUNT; i++)
        smoothedBands[i] = max(rawBands[i], smoothedBands[i] * ENVELOPE_DECAY);
}

void FFT::getBands(float out[BAND_COUNT]) const
{
    for (int i = 0; i < BAND_COUNT; i++)
        out[i] = smoothedBands[i];
}