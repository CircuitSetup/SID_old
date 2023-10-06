/*
 * -------------------------------------------------------------------
 * CircuitSetup.us Status Indicator Display
 * (C) 2023 Thomas Winischhofer (A10001986)
 * https://github.com/realA10001986/SID
 * https://sid.backtothefutu.re
 *
 * Spectrum Analyzer
 *
 * -------------------------------------------------------------------
 * License: MIT
 * 
 * Permission is hereby granted, free of charge, to any person 
 * obtaining a copy of this software and associated documentation 
 * files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the 
 * Software, and to permit persons to whom the Software is furnished to 
 * do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */ 

#include "sid_global.h"

#include <Arduino.h>

#include "src/arduinoFFT/arduinoFFT.h"

#include <driver/i2s.h>
#include <driver/adc.h>

#include <soc/i2s_reg.h>

#include "sid_main.h"

#define NUMBANDS      11
#define DISPLAYBANDS  10
#define LEDS_PER_BAR  20

#define NUMSAMPLES  1024
#define SAMPLERATE 32000

#define PEAK_HOLD    500   //ms
#define PEAK_FALL    100   //ms

const i2s_port_t I2S_PORT = I2S_NUM_0;

int32_t rawSamples[NUMSAMPLES];
double  vReal[NUMSAMPLES];
double  vImag[NUMSAMPLES];

double freqBands[NUMBANDS + 1] = { 0 };

int minTreshold = 1000;   // below considered noise FIXME

static int freqSteps[NUMBANDS] = {
   20, 70, 150, 400, 650, 1000, 2500, 4000, 6000, 8000, 12000
};

static int     oldHeight[NUMBANDS]  = { 0 };
static double  oldMmax = 0.0;

static uint8_t       peaks[NUMBANDS + 1]  = { 0 };
static unsigned long newPeak[NUMBANDS]    = { 0 };
static unsigned long peakTimer[NUMBANDS]  = { 0 };

bool        saActive = false;
bool        doPeaks = true;
static bool sa_avail = false;
static unsigned long lastTime = 0;
static unsigned long lastCorr = 0;
int         ampFact = 100;

static const i2s_pin_config_t i2sPins = {
    .bck_io_num   = I2S_BCLK_PIN,
    .ws_io_num    = I2S_LRCLK_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = I2S_DIN_PIN
};

static const i2s_config_t i2s_config = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate          = SAMPLERATE,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_32BIT,    // I2S_BITS_PER_SAMPLE_16BIT
    .channel_format       = I2S_CHANNEL_FMT_ONLY_RIGHT,   // I2S_CHANNEL_FMT_ONLY_LEFT,    // I2S_CHANNEL_FMT_RIGHT_LEFT  I2S_CHANNEL_FMT_ONLY_RIGHT
    .communication_format = I2S_COMM_FORMAT_STAND_MSB,    // I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 4,
    .dma_buf_len          = NUMSAMPLES,
    .use_apll             = false,
    .tx_desc_auto_clear   = false,
    .fixed_mclk           = 0
};   

static bool sa_setup()
{
    esp_err_t err;

    if(sa_avail)
        return true;

    err = i2s_driver_install(I2S_PORT, &i2s_config,  0, NULL);
    if(err != ESP_OK) {
        #ifdef SID_DBG
        Serial.printf("sa_setup: Failed to install i2s driver (%d)\n", err);
        #endif
        return false;
    }

    // For SPH0645
    REG_SET_BIT(I2S_TIMING_REG(I2S_PORT), BIT(9));
    REG_SET_BIT(I2S_CONF_REG(I2S_PORT), I2S_RX_MSB_SHIFT);

    i2s_set_pin(I2S_PORT, &i2sPins);

    sa_avail = true;

    return true;
}

void sa_remove()
{
    if(!sa_avail)
        return;
        
    i2s_driver_uninstall(I2S_PORT);
    sa_avail = false;
}

static void sa_resume()
{
    if(!sa_avail)
        sa_setup();
    else 
        i2s_start(I2S_PORT);
}

static void sa_stop()
{
    i2s_stop(I2S_PORT);
}

void sa_activate()
{
    sa_resume();

    lastTime = millis();
    lastCorr = 0;

    if(sa_avail)
        saActive = true;
}

void sa_deactivate()
{
    if(sa_avail)
        sa_stop();

    saActive = false;
}

int sa_setAmpFact(int newAmpFact)
{
    int old = ampFact;

    if(newAmpFact >= 0) {
        ampFact = newAmpFact;
    }

    return old;
}

void sa_loop()
{
    size_t bytesRead = 0;
    int32_t temp, maxP = 0, maxN = 0;
    unsigned long now = millis();
    
    if(!saActive || !sa_avail)
        return;
    
    if(lastTime && (now - lastTime - lastCorr < (NUMSAMPLES * 1000 / SAMPLERATE)))
        return;      

    // Calculate loop delay correctional value to keep the pace
    lastCorr = now - lastTime;
    if(lastCorr < (NUMSAMPLES * 1000 / SAMPLERATE))
        lastCorr = 0;
    else 
        lastCorr -= (NUMSAMPLES * 1000 / SAMPLERATE);

    lastTime = now;

    // Read i2c data  - do I need a timeout? FIXME
    i2s_read(I2S_PORT, (void *)rawSamples, sizeof(rawSamples), &bytesRead, portMAX_DELAY);

    if(bytesRead != sizeof(rawSamples)) {
        // what now?
        Serial.println("bytesRead != sizeof(rawSamples)");
    }

    // Correct format and clear vImag
    for(int i = 0; i < NUMSAMPLES; i++) {
        temp = rawSamples[i] / 16384; // do NOT shift; result of shifting negative integer is undefined.
        if(temp < maxN) maxN = temp;
        if(temp > maxP) maxP = temp;
        vReal[i] = (double)temp;
        vImag[i] = 0.0;
    }

    // Reset bands
    for(int i = 0; i <= NUMBANDS; i++) {
        freqBands[i] = 0.0;
    }

    #if 0
    for(int i = 0; i < 1; i++) {
        Serial.printf("%d %d %d %d %d ->", 
          rawSamples[(i*5)], rawSamples[(i*5)+1], rawSamples[(i*5)+2], rawSamples[(i*5)+3], rawSamples[(i*5)+4]);
        Serial.printf("%.1f %.1f %.1f %.1f %.1f\n", 
            vReal[(i*5)], vReal[(i*5)+1], vReal[(i*5)+2], vReal[(i*5)+3], vReal[(i*5)+4]);
    }
    #endif

    // Do the FFT
    arduinoFFT FFT = arduinoFFT(vReal, vImag, NUMSAMPLES, SAMPLERATE);

    FFT.DCRemoval();
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);      //FFT.Windowing(vReal, NUMSAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(FFT_FORWARD);                             //FFT.Compute(vReal, vImag, NUMSAMPLES, FFT_FORWARD);
    
    //FFT.ComplexToMagnitude(); // Covers entire array, half would do
    FFT.ComplexToMagnitude(vReal, vImag, NUMSAMPLES/2);

    // Fill frequency bands
    // Max freq = Half of sampling rate => (SAMPLERATE / 2)
    // vReal only filled half because of this => (NUMSAMPLES / 2)
    for(int i = 3; i < NUMSAMPLES / 2; i++) {
        if(vReal[i] > minTreshold) {
            int freq = (i - 2) * (SAMPLERATE / 2) / (NUMSAMPLES / 2);
            int band = 0;
            while(freq >= freqSteps[band]) {
                band++;
                if(band == NUMBANDS) break;
            }
            freqBands[band] += vReal[i];      
        }
    }

    // Find maximum for scaling
    double mmax = 1.0; //, avg = 0.0;
    for(int i = 1; i < NUMBANDS; i++) {
        if(mmax < freqBands[i]) mmax = freqBands[i];
        //avg += freqBands[i];
    }
    //avg /= 10.0;
    
    // Smoothen change of mmax in downward direction
    mmax = max(mmax, (oldMmax + mmax) / 2.0);
    oldMmax = mmax;

    // Convert freqBands to height factors
    for(int i = 1; i < NUMBANDS; i++) {
        freqBands[i] /= mmax;
    }

    now = millis();

    // Calculate bar heights
    for(int i = 0; i < DISPLAYBANDS; i++) {
        int height = (int)(freqBands[i+1] * (double)LEDS_PER_BAR);

        if(ampFact != 100) {
            if(!height) height = 1;
            height = height * ampFact / 100;
        }

        if(height > LEDS_PER_BAR) height = LEDS_PER_BAR;
        if(!height) height = 1;
  
        // Smoothen jumps
        height = (oldHeight[i] + height) / 2;

        // Now do peak
        if(height - 1 > peaks[i]) {
            peaks[i] = min(LEDS_PER_BAR - 1, height - 1);
            newPeak[i] = now;
            peakTimer[i] = PEAK_HOLD;
        }
    
        oldHeight[i] = height;
    }

    // Draw bars & peaks
    for(int i = 0; i < DISPLAYBANDS; i++) {
        sid.drawBarWithHeight(i, oldHeight[i]);
        if(doPeaks && peaks[i] > oldHeight[i] - 1) {
            sid.drawDot(i, peaks[i]);
        }
    }
    sid.show();

    now = millis();

    // Make peaks fall down
    for(int i = 0; i < DISPLAYBANDS; i++) {
        if(newPeak[i] && now - newPeak[i] > peakTimer[i]) {
            if(peaks[i] > 0) {
                peakTimer[i] = PEAK_FALL;
                newPeak[i] = now;
                peaks[i]--;
            } else {
                newPeak[i] = 0;
            }
        }
    }
}
