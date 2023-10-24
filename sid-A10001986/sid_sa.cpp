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

#ifdef SID_DBG
//#define SA_DBG_WRITEOUT
#endif

#include <Arduino.h>
#include "src/arduinoFFT/arduinoFFT.h"
#include <driver/i2s.h>
#include <driver/adc.h>
#include <soc/i2s_reg.h>

#include "sid_main.h"
#ifdef SA_DBG_WRITEOUT
#include "sid_settings.h"
#endif

#define NUMBANDS      11
#define DISPLAYBANDS  10
#define LEDS_PER_BAR  20

#define NUMSAMPLES  1024
#define SAMPLERATE 32000

#define PEAK_HOLD    500   // ms
#define PEAK_FALL    100   // ms

static const i2s_port_t I2S_PORT = I2S_NUM_0;

static int32_t rawSamples[NUMSAMPLES];
static double  vReal[NUMSAMPLES];
static double  vImag[NUMSAMPLES];

static double freqBands[NUMBANDS] = { 0 };

// The frequency bands
// First one is "garbage bin", not used for display
static int freqSteps[NUMBANDS] = {
   //110,  200,  400,  600,  800, 1000, 2500, 3500, 5000, 7000, 9000
   // 60,  150,  250,  400,  650, 1000, 1600, 2500, 4000, 6250, 10000
      80,  100,  150,  250,  430,  650, 1000, 2000, 4000, 7000, 10000
};

static int minTreshold[NUMBANDS] = {
       0, 5000, 5000, 5000, 3000, 1000, 1000, 1000, 1000, 1000, 1000
};

static int     oldHeight[NUMBANDS]  = { 0 };

static uint8_t       peaks[NUMBANDS + 1]  = { 0 };
static unsigned long newPeak[NUMBANDS]    = { 0 };
static unsigned long peakTimer[NUMBANDS]  = { 0 };

bool        saActive = false;
bool        doPeaks = false;
static bool sa_avail = false;
static unsigned long lastTime = 0;
int         ampFact = 100;

#ifdef SA_DBG_WRITEOUT
#include <SD.h>
#include <FS.h>
static File outFile;
static bool outFileOpen = false;
#endif

static const i2s_pin_config_t i2sPins = {
    .bck_io_num   = I2S_BCLK_PIN,
    .ws_io_num    = I2S_LRCLK_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = I2S_DIN_PIN
};

static const i2s_config_t i2s_config = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate          = SAMPLERATE,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format       = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_STAND_MSB,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 4,
    .dma_buf_len          = 1024,
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

    #ifdef SA_DBG_WRITEOUT
    if(haveSD) {
        outFile = SD.open("/sidsa.pcm", FILE_WRITE);
        outFileOpen = true;
    }
    #endif

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

    if(sa_avail)
        saActive = true;
}

void sa_deactivate()
{
    if(sa_avail)
        sa_stop();

    saActive = false;

    #ifdef SA_DBG_WRITEOUT
    outFile.close();
    outFileOpen = false;
    #endif
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
    unsigned long now = millis();
    int mmaxi = 0, band = 0;
    
    if(!saActive || !sa_avail)
        return;
    
    if(lastTime && (now - lastTime < (NUMSAMPLES * 1000 / SAMPLERATE)))
        return;

    lastTime = now;

    // Read i2c data  - do I need a timeout? FIXME
    i2s_read(I2S_PORT, (void *)rawSamples, sizeof(rawSamples), &bytesRead, portMAX_DELAY);

    if(bytesRead != sizeof(rawSamples)) {
        // what now?
        #ifdef SID_DBG
        Serial.println("bytesRead != sizeof(rawSamples)");
        #endif
    }

    #ifdef SA_DBG_WRITEOUT
    
    if(outFileOpen) {
        outFile.write((uint8_t *)&rawSamples[0], NUMSAMPLES * 4);
    }
    
    #else

    // Convert; clear vImag
    for(int i = 0; i < NUMSAMPLES; i++) {
        vReal[i] = (double)(rawSamples[i] / 16384); // do NOT shift; result of shifting negative integer is undefined.;
        vImag[i] = 0.0;
    }

    // Do the FFT
    arduinoFFT FFT = arduinoFFT(vReal, vImag, NUMSAMPLES, SAMPLERATE);

    // Remove hum and dc offset
    FFT.DCRemoval();

    // Windowing: "Rectangle" does fine for our purpose
    //FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Windowing(FFT_WIN_TYP_RECTANGLE, FFT_FORWARD);
    
    FFT.Compute(FFT_FORWARD);
    
    //FFT.ComplexToMagnitude(); // Covers entire array, half would do
    FFT.ComplexToMagnitude(vReal, vImag, NUMSAMPLES/2);

    // Fill frequency bands
    // Max freq = Half of sampling rate => (SAMPLERATE / 2)
    // vReal only filled half because of this => (NUMSAMPLES / 2)
    band = 0;
    for(int i = 3; i < NUMSAMPLES / 2; i++) {
        int freq = (i - 2) * (SAMPLERATE / 2) / (NUMSAMPLES / 2);
        if(freq >= freqSteps[band]) {
            band++;
            if(band == NUMBANDS) break;
            else freqBands[band] = 0.0;
        }
        if(band && (vReal[i] > minTreshold[band])) {
            freqBands[band] += vReal[i];      
        }
    }

    // Find maximum for scaling
    double mmax = 1.0;
    for(int i = 1; i < NUMBANDS; i++) {
        if(mmax < freqBands[i]) mmax = freqBands[i];
    }

    // No point in smoothening mmax changes; is relative
    // to amplitude of sample block (32ms @ 1024 @ 32kHz)
    // and therefore not in any way valid over more than one
    // sample block.

    // Convert freqBands to height factors
    for(int i = 1; i < NUMBANDS; i++) {
        freqBands[i] /= mmax;
    }

    now = millis();

    // Calculate bar heights
    for(int i = 0; i < DISPLAYBANDS; i++) {
        int height = (int)(freqBands[i+1] * (double)(LEDS_PER_BAR - 1));

        if(ampFact != 100) {
            if(!height) height = 1;
            height = height * ampFact / 100;
        }

        if(height > LEDS_PER_BAR) height = LEDS_PER_BAR;
        if(!height) height = 1;
  
        // Smoothen jumps in downward direction
        if(height < oldHeight[i]) {
            if(oldHeight[i] - height > 10) height = (oldHeight[i] + height) / 2;
            else                           height = oldHeight[i] - 1;
        }

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

    #endif
}
