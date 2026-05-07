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
    double frequencyBandMaxDb[TOTAL_BANDS];
    int index = 0;
    double hzPerSample = (1.0 * SAMPLING_FREQ) / SAMPLES; //
    double hz = 0;
    double sum = 0;
    int count = 0;

    memset(max, 0, sizeof(max));
    memset(frequencyBandMaxDb, 0, sizeof(frequencyBandMaxDb));

    for (int i = 2; i < (SAMPLES / 2); i++)
    {
        count++;
        sum += vReal[i];
        if (vReal[i] > max[index])
        {
            max[index] = vReal[i];
        }
        if (hz > cutoffFrequencies[index])
        {
            sum = 0.0;
            count = 0;
            index++;
            max[index] = 0;
        }
        hz += hzPerSample;
    }

    int frequencyBandIndex = 0;

    for (int i = FREQUENCY_BANDS - 1; i >= 3; i--)
    {
        int frequencyMaxNormalizedDb = 0;
        // calculate actual decibels
        if (max[i] > 0)
        {
            frequencyMaxNormalizedDb = 20.0 * (log10(max[i]) - reference);
        }

        // adjust minimum and maximum levels
        if (frequencyMaxNormalizedDb < 0)
        {
            frequencyMaxNormalizedDb = 0;
        }

        if (frequencyMaxNormalizedDb >= MAX_FREQUENCY_VALUE)
        {
            frequencyMaxNormalizedDb = MAX_FREQUENCY_VALUE;
        }
        frequencyBandMaxDb[frequencyBandIndex] = frequencyMaxNormalizedDb;
        frequencyBandIndex++;
    }

    redBandValue = 0;
    greenBandValue = 0;
    blueBandValue = 0;

    for (int i = 0; i <= 4; i++) // 5 bands
        redBandValue += frequencyBandMaxDb[i];

    for (int i = 5; i <= 7; i++) // 3 bands
        greenBandValue += frequencyBandMaxDb[i];

    for (int i = 8; i <= 10; i++) // 3 bands
        blueBandValue += frequencyBandMaxDb[i];
}

void FFT::calculatePercentages(float & redPercentage, float & greenPercentage, float & bluePercentage)
{
    takeSamples();
    computeFFT();
    calculateBands();

    float rawRed = percentage(redBandValue, 5);
    float rawGreen = percentage(greenBandValue, 3);
    float rawBlue = percentage(blueBandValue, 3);

    // AGC: track the recent peak across all bands and normalize to it. Fast attack
    // (jump up immediately) and slow decay so the visualization auto-calibrates
    // to room volume but doesn't pump on every transient.
    float instantPeak = max(max(rawRed, rawGreen), rawBlue);
    if (instantPeak > agcMax)
        agcMax = instantPeak;
    else
        agcMax = max(agcMax * AGC_DECAY, AGC_FLOOR);

    float gain = 1.0f / agcMax;
    rawRed = min(rawRed * gain, 1.0f);
    rawGreen = min(rawGreen * gain, 1.0f);
    rawBlue = min(rawBlue * gain, 1.0f);

    // Envelope follower per band: peak attack, exponential decay. Keeps transients
    // visible and stops the ring from twitching back to dark between drum hits.
    smoothedRed = max(rawRed, smoothedRed * ENVELOPE_DECAY);
    smoothedGreen = max(rawGreen, smoothedGreen * ENVELOPE_DECAY);
    smoothedBlue = max(rawBlue, smoothedBlue * ENVELOPE_DECAY);

    redPercentage = smoothedRed;
    greenPercentage = smoothedGreen;
    bluePercentage = smoothedBlue;
}