/*
 * -------------------------------------------------------------------
 * CircuitSetup.us Status Indicator Display
 * (C) 2023 Thomas Winischhofer (A10001986)
 * https://github.com/realA10001986/SID
 * https://sid.backtothefutu.re
 *
 * SIDDisplay Class: Handles the SID LEDs
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

#ifndef _SIDDISPLAY_H
#define _SIDDISPLAY_H

#define SD_BUF_SIZE   16  // Buffer size in words (16bit)

class sidDisplay {

    public:

        sidDisplay(uint8_t address1, uint8_t address2);
        void begin();
        void on();
        void off();

        void lampTest();

        void clearBuf();

        uint8_t setBrightness(uint8_t level, bool setInitial = false);
        void    resetBrightness();
        uint8_t setBrightnessDirect(uint8_t level);
        uint8_t getBrightness();
        
        void show();

        void clearDisplayDirect();

        void drawBar(uint8_t bar, uint8_t bottom, uint8_t top);
        void drawBarWithHeight(uint8_t bar, uint8_t height);
        void clearBar(uint8_t bar);
        void drawDot(uint8_t bar, uint8_t dot_y);

        void drawFieldAndShow(uint8_t *fieldData);

        void drawLetterAndShow(char alpha, int x = 0, int y = 8);
        void drawLetterMask(char alpha, int x, int y);

    private:
        void directCmd(uint8_t val);
        
        uint8_t _address[2] = { 0, 0 };

        uint8_t _brightness = 15;     // current display brightness
        uint8_t _origBrightness = 15; // value from settings
        
        uint16_t _displayBuffer[SD_BUF_SIZE];

};

#endif
