/*
 * -------------------------------------------------------------------
 * CircuitSetup.us Status Indicator Display
 * (C) 2023 Thomas Winischhofer (A10001986)
 * https://github.com/realA10001986/SID
 * http://sid.backtothefutu.re
 *
 * Settings handling
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

#ifndef _SID_SETTINGS_H
#define _SID_SETTINGS_H

extern bool haveSD;
extern bool FlashROMode;

extern uint8_t musFolderNum;

#define MS(s) XMS(s)
#define XMS(s) #s

// Default settings - change settings in the web interface 192.168.4.1

#define DEF_SS_TIMER        0     // "Screen saver" timeout in minutes; 0 = ss off
#define DEF_SA_PEAKS        1     // 1: Show peaks in SA, 0: don't
#define DEF_TCD_PRES        0     // 0: No TCD connected, 1: connected via GPIO
#define DEF_DISDIR          0     // 0: Do not disable default IR remote control; 1: do
#define DEF_HOSTNAME        "sid"
#define DEF_WIFI_RETRY      3     // 1-10; Default: 3 retries
#define DEF_WIFI_TIMEOUT    7     // 7-25; Default: 7 seconds
#define DEF_TCD_IP          ""    // TCD ip address for networked polling
#define DEF_WAIT_FOR_TCD    0     // 0: Boot normally  1: Delay WiFi setup for 30 seconds (to wait for TCD if powered up simultaniously)
#define DEF_USE_GPSS        0     // 0: Ignore GPS speed; 1: Use it for chase speed
#define DEF_USE_NM          0     // 0: Ignore TCD night mode; 1: Follow TCD night mode
#define DEF_USE_FPO         0     // 0: Ignore TCD fake power; 1: Follow TCD fake power
#define DEF_WAIT_FPO        1     // 0: Don't wait for fake power on during boot, 1: Do
#define DEF_CFG_ON_SD       1     // Default: Save vol/spd/IR settings on SD card
#define DEF_SD_FREQ         0     // SD/SPI frequency: Default 16MHz

struct Settings {
    char ssTimer[6]         = MS(DEF_SS_TIMER);
    char SApeaks[4]         = MS(DEF_SA_PEAKS);
    char TCDpresent[4]      = MS(DEF_TCD_PRES);
    char disDIR[4]          = MS(DEF_DISDIR);
    
    char hostName[32]       = DEF_HOSTNAME;
    char systemID[8]        = "";
    char appw[10]           = "";
    char wifiConRetries[4]  = MS(DEF_WIFI_RETRY);
    char wifiConTimeout[4]  = MS(DEF_WIFI_TIMEOUT);

    char tcdIP[32]          = DEF_TCD_IP;
    //char wait4TCD[4]        = MS(DEF_WAIT_FOR_TCD);
    char useGPSS[4]         = MS(DEF_USE_GPSS);
    char useNM[4]           = MS(DEF_USE_NM);
    char useFPO[4]          = MS(DEF_USE_FPO);
    char wait4FPOn[4]       = MS(DEF_WAIT_FPO);

    char CfgOnSD[4]         = MS(DEF_CFG_ON_SD);
    char sdFreq[4]          = MS(DEF_SD_FREQ);
    
#ifdef SID_HAVEMQTT  
    char useMQTT[4]         = "0";
    char mqttServer[80]     = "";  // ip or domain [:port]  
    char mqttUser[128]      = "";  // user[:pass] (UTF8)
#endif    
};

struct IPSettings {
    char ip[20]       = "";
    char gateway[20]  = "";
    char netmask[20]  = "";
    char dns[20]      = "";
};

extern struct Settings settings;
extern struct IPSettings ipsettings;

void settings_setup();
void write_settings();
bool checkConfigExists();

void copySettings();

bool loadBrightness();
void saveBrightness(bool useCache = true);

bool loadIdlePat();
void saveIdlePat(bool useCache = true);

bool loadIRLock();
void saveIRLock(bool useCache = true);

bool saveIRKeys();
void deleteIRKeys();

bool loadIpSettings();
void writeIpSettings();
void deleteIpSettings();

void formatFlashFS();

#endif
