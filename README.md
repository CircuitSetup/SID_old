# Firmware for Status Indicator Display (SID)

This repository holds the most current firmware for CircuitSetup's magnificent [SID](https://circuitsetup.us).

The hardware is available [here](https://circuitsetup.us).

Features include
- various idle pattern modes
- [Time Travel](#time-travel) function, triggered by button, [Time Circuits Display](https://tcd.backtothefutu.re) or via [MQTT](#home-assistant--mqtt)
- Spectrum Analyzer mode via microphone
- [IR remote controlled](#ir-remote-control); can learn keys from custom remote
- Advanced network-accessible [Config Portal](#the-config-portal) for setup with mDNS support for easy access (http://sid.local, hostname configurable)
- Wireless communication with Time Circuits Display ("[BTTF-Network](#bttf-network-bttfn)"); used for synchonized time travels, alarm, chase speed, night mode, fake power
- [Home Assistant](#home-assistant--mqtt) (MQTT 3.1.1) support
- [*Siddly*](#siddly) and [*Snake*](#snake) games
- [SD card](#sd-card) support

## Installation

There are different alternative ways to install this firmware:

1) If a previous version of the SID firmware was installed on your device, you can upload the provided pre-compiled binary to update to the current version: Enter the [Config Portal](#the-config-portal), click on "Update" and select the pre-compiled binary file provided in this repository ("install/sid-A10001986.ino.nodemcu-32s.bin").

2) Using the Arduino IDE or PlatformIO: Download the sketch source code, all required libraries, compile and upload it. This method is the one for fresh ESP32 boards and/or folks familiar with the programming tool chain. Detailed build information is in [sid-A10001986.ino](https://github.com/realA10001986/SID/blob/main/SID-A10001986/SID-A10001986.ino).

 *Important: After a firmware update, a "wait" symbol (hourglass) might be shown for up to a minute after reboot. Do NOT unplug the device during this time.*

## Short summary of first steps

A good first step would be to establish access to the Config Portal in order to configure your SID.

As long as the device is unconfigured, as is the case with a brand new SID, or later if it for some reason fails to connect to a configured WiFi network, it starts in "access point" mode, i.e. it creates a WiFi network of its own named "SID-AP". This is called "Access Point mode", or "AP-mode".

- Power up the device and wait until the startup sequence has completed.
- Connect your computer or handheld device to the WiFi network "SID-AP".
- Navigate your browser to http://sid.local or http://192.168.4.1 to enter the Config Portal.
 
If you want your SID to connect to your WiFi network, click on "Configure WiFi". The bare minimum is to select an SSID (WiFi network name) and a WiFi password. 

Note that the device requests an IP address via DHCP, unless you entered valid data in the fields for static IP addresses (IP, gateway, netmask, DNS). 

After saving the WiFi network settings, the device reboots and tries to connect to your configured WiFi network. If that fails, it will again start in access point mode.

If the device is inaccessible as a result of wrong static IPs, wait until the SID has completed its startup sequence, then type \*123456OK on the IR remote; static IP data will be deleted and the device will return to DHCP after a reboot.

If you have your SID, along with a Time Circuits Display, mounted in a car, see also [here](#car-setup).

### The Config Portal

The Config Portal is accessible exclusively through WiFi. As outlined above, if the device is not connected to a WiFi network, it creates its own WiFi network (named "SID-AP"), to which your WiFi-enabled hand held device or computer first needs to connect in order to access the Config Portal.

If the operating system on your handheld or computer supports Bonjour (a.k.a. "mDNS"), you can enter the Config Portal by directing your browser to http://sid.local. (mDNS is supported on Windows 10 version TH2 (1511) [other sources say 1703] and later, Android 13 and later, MacOS, iOS)

If that fails, the way to enter the Config Portal depends on whether the device is in access point mode or not. 
- If it is in access point mode (and your handheld/computer is connected to the WiFi network "SID-AP"), navigate your browser to http://192.168.4.1 
- Otherwise type *90 followed by OK on the remote control; the IP address will be shown on the display.

In the main menu, click on "Setup" to configure your SID.

| ![The Config Portal](https://github.com/realA10001986/SID/assets/76924199/e620a93d-ac54-4d4b-9925-cd526ca9d540) |
|:--:| 
| *The Config Portal's Setup page* |

A full reference of the Config Portal is [here](#appendix-a-the-config-portal).

## Basic Operation

By default, it idles and shows an idle pattern. There are alternative idle patterns, selected by *0OK through *4OK on the remote, or via MQTT. If set through the IR remote and if an SD card is inserted, the setting will be persistent accross reboots.

For the options to trigger a time travel, see [here](#time-travel).

The main control device, however, is the IR remote control.

### IR remote control

Your SID might have an IR remote control included. This remote works out-of-the-box and needs no setup. 

| ![Supplied IR remote control](https://github.com/realA10001986/SID/assets/76924199/9637b95b-d3ef-4a12-b72c-215f027e9514) |
|:--:| 
| *The default IR remote control* |

Each time you press a (recognized) key on the remote, an IR feedback LED will briefly light up. This LED is located at the bottom of the board.

### IR learning

You can have your SID learn the codes of another IR remote control. Most remotes with a carrier signal of 38kHz (which most IR remotes use) will work. However, some remote controls, expecially ones for TVs, send keys repeatedly and/or send different codes alternately. If you had the SID learn a remote and the keys are not (always) recognized afterwards or appear to the pressed repeatedly while held, that remote is of that type and cannot be used.

First, go to the Config Portal, uncheck **_TCD connected by wire_** on the Setup page and save. The SID reboots. Afterwards, to start the learning process, hold the Time Travel button for a few seconds, until the displays shows "GO" followed by "0". Then press "0" on your remote, which the SID will visually acknowledge by displaying the next key to press. Then press "1", wait for the acknowledgement, and so on. Enter your keys in the following order:

```0 - 1 - 2 - 3 - 4 - 5 - 6 - 7 - 8 - 9 - * - # - Arrow up - Arrow down - Arrow left - Arrow right - OK``` 

If your remote control lacks the \* (starts command sequence) and \# (aborts command sequence) keys, you can use any other key, of course. \* could be eg. "menu" or "setup", \# could be "exit" or "return".

If no key is pressed for 10 seconds, the learning process aborts, as does briefly pressing the Time Travel button. In thoses cases, the keys already learned are forgotten and nothing is saved.

To make the SID forget a learned IR remote control, type *654321 followed by OK.

### Locking IR control

You can have your SID ignore IR commands by entering *71 followed by OK. After this sequence the SID will ignore all IR commands until *71OK is entered again. The purpose of this function is to enable you to use the same IR control for your SID and other props (such as Flux Capacitor).

Note that the status of the IR lock is saved 10 seconds after its last change, and is persistent accross reboots.

### IR remote reference

<table>
    <tr>
     <td align="center" colspan="3">IR remote reference: Single key actions</td>
    </tr>
    <tr>
     <td align="center">1<br>Games: Quit</td>
     <td align="center">2<br>-</td>
     <td align="center">3<br>Games: New game</a></td>
    </tr>
    <tr>
     <td align="center">4<br>-</td>
     <td align="center">5<br>-</td>
     <td align="center">6<br>-</td>
    </tr>
    <tr>
     <td align="center">7<br>-</td>
     <td align="center">8<br>-</td>
     <td align="center">9<br>Games: Pause</td>
    </tr>
    <tr>
     <td align="center">*<br>Start command sequence</td>
     <td align="center">0<br><a href="#time-travel">Time Travel</a><br>Siddly: Fall down</td>
     <td align="center">#<br>Abort command sequence</td>
    </tr>
    <tr>
     <td align="center"></td>
     <td align="center">&#8593;<br>Increase Brightness<br>Siddly: Rotate<br>Snake: Up</td>
     <td align="center"></td>
    </tr>
    <tr>
     <td align="center">&#8592;<br>Games: Left</td>
     <td align="center">OK<br>Execute command</td>
     <td align="center">&#8594;<br>Games: Right</td>
    </tr>
    <tr>
     <td align="center"></td>
     <td align="center">&#8595;<br>Decrease Brightness<br>Games: Down</td>
     <td align="center"></td>
    </tr>
</table>

<table>
    <tr>
     <td align="center" colspan="2">IR remote reference: Special sequences<br>(&#9166; = OK key)</td>
    </tr>
  <tr>
     <td align="left">*0&#9166;</td>
     <td align="left">Default idle pattern</td>
    </tr>
   <tr>
     <td align="left">*1&#9166;</td>
     <td align="left">Idle pattern 1</td>
    </tr>
   <tr>
     <td align="left">*2&#9166;</td>
     <td align="left">Idle pattern 2</td>
    </tr>
  <tr>
     <td align="left">*3&#9166;</td>
     <td align="left">Idle pattern 3</td>
    </tr>
     <tr>
     <td align="left">*4&#9166;</td>
     <td align="left">Idle pattern 4</td>
    </tr>
   <tr>
     <td align="left">*00&#9166;</td>
     <td align="left">Idle mode</td>
    </tr>
   <tr>
     <td align="left">*01&#9166;</td>
     <td align="left">Start Spectrum Analyzer</td>
    </tr>
   <tr>
     <td align="left">*02&#9166;</td>
     <td align="left">Start Siddly game</td>
    </tr>
    <tr>
     <td align="left">*03&#9166;</td>
     <td align="left">Start Snake game</td>
    </tr>
    <tr>
     <td align="left">*50&#9166;</td>
     <td align="left">Enable/disable peaks in Spectrum Analyzer</td>
    </tr>
    <tr>
     <td align="left">*71&#9166;</td>
     <td align="left"><a href="#locking-ir-control">Disable/Enable</a> IR remote commands</td>
    </tr>
     <tr>
     <td align="left">*90&#9166;</td>
     <td align="left">Display current IP address</td>
    </tr>
    <tr>
     <td align="left">*64738&#9166;</td>
     <td align="left">Reboot the device</td>
    </tr>
    <tr>
     <td align="left">*123456&#9166;</td>
     <td align="left">Delete static IP address<br>and WiFi-AP password</td>
    </tr>
     <tr>
     <td align="left">*654321&#9166;</td>
     <td align="left">Delete learned IR remote control</td>
    </tr>
</table>

[Here](https://github.com/realA10001986/SID/blob/main/CheatSheet.pdf) is a cheat sheet for printing or screen-use. (Note that MacOS' preview application has a bug that scrambles the links in the document. Acrobat Reader does it correctly.)

## Time travel

To travel through time, type "0" on the remote control. The SID will play its time travel sequence.

You can also connect a physical button to your SID; the button must shorten "TT" and "3.3V" on the "Time Travel" connector. Pressing this button briefly will trigger a time travel.

Other ways of triggering a time travel are available if a [Time Circuits Display](#connecting-a-time-circuits-display) is connected.

## Spectrum Analyzer

The spectrum analyzer works through a built-in microphone. Is shows ten frequency bands. Sticky peaks are optional, they can be switched on/off in the Config Portal and by typing *50 followed by OK on the remote.

## Games

### Siddly

Siddly is a simple game where puzzle pieces of various shapes fall down from the top. You can slide them left and right, as well as rotate them while they are falling. When the piece lands at the bottom, a new piece will appear at the top and start falling down. If a line at the bottom is completely filled with fallen pieces or parts thereof, that line will be cleared, and everything piled on top of that line will move down. The target is to keep the pile at the bottom as low as possible; the game ends when the pile is as high as the screen and no new piece has room to appear. I think you get the idea. Note that the red LEDs at the top are not part of the playfield (but show a level-progress bar instead), the field only covers the yellow and green LEDs, and that simularities of Siddly with computer games, especially older ones, exist only in your imagination.

### Snake

Snakes like apples (at least so I have heard). You control a snake that feels a profound urge to eat apples. After each eaten apple, the snake grows, and a new apple appears. Unfortunately, snakes don't like to hit their heads, so you need to watch out that the snake's head doesn't collide with its body.

## SD card

Preface note on SD cards: For unknown reasons, some SD cards simply do not work with this device. For instance, I had no luck with Sandisk Ultra 32GB and  "Intenso" cards. If your SD card is not recognized, check if it is formatted in FAT32 format (not exFAT!). Also, the size must not exceed 32GB (as larger cards cannot be formatted with FAT32). I am currently using Transcend SDHC 4GB cards and those work fine.

The SD card is used for saving secondary settings, in order to avoid flash wear on the SID's ESP32.

Note that the SD card must be inserted before powering up the device. It is not recognized if inserted while the SID is running. Furthermore, do not remove the SD card while the device is powered.

## Connecting a Time Circuits Display

### Connecting TCD by wire

Connect GND and GPIO on the SID's "Time Travel" connector to the TCD like in the table below:

<table>
    <tr>
     <td align="center">SID:<br>"Time Travel" connector</td>
     <td align="center">TCD control board 1.2</td>
     <td align="center">TCD control board 1.3</td>
    </tr>
   <tr>
     <td align="center">GND</td>
     <td align="center">GND of "IO13" connector</td>
     <td align="center">GND on "Time Travel" connector</td>
    </tr>
    <tr>
     <td align="center">GPIO</td>
     <td align="center">IO13 of "IO13" connector</td>
     <td align="center">TT OUT on "Time Travel" connector</td>
    </tr>
</table>

Note that a wired connection only allows for synchronized time travel sequences, no other communication takes place.

### BTTF-Network ("BTTFN")

The TCD can communicate with the SID wirelessly, via WiFi. It can send out information about a time travel and an alarm, and the SID queries the TCD for speed and some other data. Unlike with MQTT, no broker or other third party software is needed.

![BTTFN connection](https://github.com/realA10001986/SID/assets/76924199/60ddeb60-a998-4ad8-8b1c-5a715f850109)

In order to connect your SID to the TCD using BTFFN, just enter the TCD's IP address in the **_IP address of TCD_** field in the SID's Config Portal. On the TCD, no special configuration is required.

Afterwards, the SID and the TCD can communicate wirelessly and 
- play time travel sequences in sync,
- both play an alarm-sequence when the TCD's alarm occurs,
- the SID queries the TCD for GPS speed if desired to adapt chase speed to GPS speed,
- the SID queries the TCD for fake power and night mode, in order to react accordingly if so configured.

You can use BTTF-Network and MQTT at the same time, see immediately below.

### Home Assistant/MQTT

The other way of wireless communication is, of course, [Home Assistant/MQTT](#home-assistant--mqtt).

If both TCD and SID are connected to the same broker, and the option **_Send event notifications_** is checked on the TCD's side, the SID will receive information on time travel and alarm and play their sequences in sync with the TCD. Unlike BTTFN, however, no other communication takes place.

![MQTT connection](https://github.com/realA10001986/SID/assets/76924199/f2838deb-c673-4bfb-9e09-88e26691742f)

MQTT and BTTFN can co-exist. However, the TCD only sends out time travel and alarm notifications through either MQTT or BTTFN, never both. If you have other MQTT-aware devices listening to the TCD's public topic (bttf/tcd/pub) in order to react to time travel or alarm messages, use MQTT (ie check **_Send event notifications_**). If only BTTFN-aware devices are to be used, uncheck this option to use BTTFN as it has less latency.

## Home Assistant / MQTT

The SID supports the MQTT protocol version 3.1.1 for the following features:

### Control the SID via MQTT

The SID can - to a some extent - be controlled through messages sent to topic **bttf/sid/cmd**. Support commands are
- TIMETRAVEL: Start a [time travel](#time-travel)
- IDLE: Select idle mode
- SA: Start spectrum analyzer
- IDLE_0, IDLE_1, IDLE_2, IDLE_3, IDLE_4: Select idle pattern

### Receive commands from Time Circuits Display

The TCD can trigger a time travel and tell the SID about an alarm by sending messages to topic **bttf/tcd/pub**. The SID receives these commands and reacts accordingly.

### Setup

In order to connect to a MQTT network, a "broker" (such as [mosquitto](https://mosquitto.org/), [EMQ X](https://www.emqx.io/), [Cassandana](https://github.com/mtsoleimani/cassandana), [RabbitMQ](https://www.rabbitmq.com/), [Ejjaberd](https://www.ejabberd.im/), [HiveMQ](https://www.hivemq.com/) to name a few) must be present in your network, and its address needs to be configured in the Config Portal. The broker can be specified either by domain or IP (IP preferred, spares us a DNS call). The default port is 1883. If a different port is to be used, append a ":" followed by the port number to the domain/IP, such as "192.168.1.5:1884". 

If your broker does not allow anonymous logins, a username and password can be specified.

Limitations: MQTT Protocol version 3.1.1; TLS/SSL not supported; ".local" domains (MDNS) not supported; server/broker must respond to PING (ICMP) echo requests. For proper operation with low latency, it is recommended that the broker is on your local network. 

### Car setup

If your SID, along with a [Time Circuits Display](https://tcd.backtothefutu.re), is mounted in a car, the following network configuration is recommended:

#### TCD

- Run your TCD in [*car mode*](https://tcd.backtothefutu.re/#car-mode);
- disable WiFi power-saving on the TCD by setting **_WiFi power save timer for AP-mode_** to 0 (zero).

#### SID

Enter the Config Portal on the SID (as described above), click on *Setup* and
  - enter *192.168.4.1* into the field **_IP address of TCD_**
  - check the option **_Follow TCD fake power_** if you have a fake power switch for the TCD (like eg a TFC switch)
  - click on *Save*.

After the SID has restarted, re-enter the SID's Config Portal (while the TCD is powered and in *car mode*) and
  - click on *Configure WiFi*,
  - select the TCD's access point name in the list at the top or enter *TCD-AP* into the *SSID* field; if you password-protected your TCD's AP, enter this password in the *password* field. Leave all other fields empty,
  - click on *Save*.

Using this setup enables the SID to receive notifications about time travel and alarm wirelessly, and to query the TCD for data.

In order to access the SID's Config Portal in your car, connect your hand held or computer to the TCD's WiFi access point ("TCD-AP"), and direct your browser to http://sid.local ; if that does not work, go to the TCD's keypad menu, press ENTER until "BTTFN CLIENTS" is shown, hold ENTER, and look for the SID's IP address there; then direct your browser to that IP by using the URL http://a.b.c.d (a-d being the IP address displayed on the TCD display).

## Flash Wear

Flash memory has a somewhat limited life-time. It can be written to only between 10.000 and 100.000 times before becoming unreliable. The firmware writes to the internal flash memory when saving settings and other data. Every time you change settings, data is written to flash memory.

In order to reduce the number of write operations and thereby prolong the life of your SID, it is recommended to use a good-quality SD card and to check **_["Save secondary settings on SD"](#-save-settings-on-sd)_** in the Config Portal; some settings as well as learned IR codes are then stored on the SD card (which also suffers from wear but is easy to replace). If you want to swap the SD card but preserve your settings, go to the Config Portal while the old SD card is still in place, uncheck the **_Save secondary settings on SD_** option, click on Save and wait until the device has rebooted. You can then power down, swap the SD card and power-up again. Then go to the Config Portal, change the option back on and click on Save. Your settings are now on the new SD card.

## Appendix A: The Config Portal

### Main page

##### &#9654; Configure WiFi

Clicking this leads to the WiFi configuration page. On that page, you can connect your SID to your WiFi network by selecting/entering the SSID (WiFi network name) as well as a password (WPA2). By default, the SID requests an IP address via DHCP. However, you can also configure a static IP for the SID by entering the IP, netmask, gateway and DNS server. All four fields must be filled for a valid static IP configuration. If you want to stick to DHCP, leave those four fields empty.

Note that this page has nothing to do with Access Point mode; it is strictly for connecting your SID to an existing WiFi network as a client.

##### &#9654; Setup

This leads to the [Setup page](#setup-page).

##### &#9654; Restart

This reboots the SID. No confirmation dialog is displayed.

##### &#9654; Update

This leads to the firmware update page. You can select a locally stored firmware image file to upload (such as the ones published here in the install/ folder).

##### &#9654; Erase WiFi Config

Clicking this (and saying "yes" in the confirmation dialog) erases the WiFi configuration (WiFi network and password) and reboots the device; it will restart in "access point" mode. See [here](#short-summary-of-first-steps).

---

### Setup page

#### Basic settings

##### &#9654; Screen saver timer

Enter the number of minutes until the Screen Saver should become active when the SID is idle.

The Screen Saver, when active, disables all LEDs, until 
- a key on the IR remote control is pressed; if IR is [locked](#locking-ir-control), only the # key deactivates the Screen Saver;
- the time travel button is briefly pressed,
- a time travel event is triggered from a connected TCD (wire or wirelessly)

##### &#9654; Show peaks in Spectrum Analyzer

This selects the boot-up setting for showing or not showing the peaks in the Spectrum Analyzer. Can be changed anytime by typing *50 followed by OK on the IR remote control.

#### Hardware configuration settings

##### &#9654; Disable supplied IR remote control

Check this to disable the supplied remote control; the FC will only accept commands from a learned IR remote (if applicable). 

Note that this only disables the supplied remote, unlike [IR locking](#locking-ir-control), where IR commands from any known remote are ignored.

#### Network settings

##### &#9654; Hostname

The device's hostname in the WiFi network. Defaults to 'sid'. This also is the domain name at which the Config Portal is accessible from a browser in the same local network. The URL of the Config Portal then is http://<i>hostname</i>.local (the default is http://sid.local)

If you have more than one SID in your local network, please give them unique hostnames.

##### &#9654; AP Mode: Network name appendix

By default, if the SID creates a WiFi network of its own ("AP-mode"), this network is named "SID-AP". In case you have multiple SIDs in your vicinity, you can have a string appended to create a unique network name. If you, for instance, enter "-ABC" here, the WiFi network name will be "SID-AP-ABC". Characters A-Z, a-z, 0-9 and - are allowed.

##### &#9654; AP Mode: WiFi password

By default, and if this field is empty, the SID's own WiFi network ("AP-mode") will be unprotected. If you want to protect your SID access point, enter your password here. It needs to be 8 characters in length and only characters A-Z, a-z, 0-9 and - are allowed.

If you forget this password and are thereby locked out of your SID, enter *123456 followed by OK on the IR remote control; this deletes the WiFi password. Then power-down and power-up your SID and the access point will start unprotected.

##### &#9654; WiFi connection attempts

Number of times the firmware tries to reconnect to a WiFi network, before falling back to AP-mode. See [here](#short-summary-of-first-steps)

##### &#9654; WiFi connection timeout

Number of seconds before a timeout occurs when connecting to a WiFi network. When a timeout happens, another attempt is made (see immediately above), and if all attempts fail, the device falls back to AP-mode. See [here](#short-summary-of-first-steps)

#### Settings for prop communication/synchronization

##### &#9654; TCD connected by wire

Check this if you have a Time Circuits Display connected by wire. Note that you can only connect *either* a button *or* the TCD to the "time travel" connector on the SID, but not both.

Note that a wired connection only allows for synchronized time travel sequences, no other communication takes place.

Also note that the process of [learning keys from an IR remote control](#ir-remote-control) requires this option to be unchecked. After learning keys is done, you can, of course, check this option again.

##### &#9654; IP address of TCD

If you want to have your SID to communicate with a Time Circuits Display wirelessly ("BTTF-Network"), enter the IP address of the TCD here. Do NOT enter a host name here.

If you connect your SID to the TCD's access point ("TCD-AP"), the TCD's IP address is 192.168.4.1.

##### &#9654; Adapt to GPS speed

If your TCD is equipped with a GPS sensor, the SID can adapt its display to current GPS speed. This option selects if GPS speed should be used.

##### &#9654; Follow TCD night-mode

If this option is checked, and your TCD goes into night mode, the SID will activate the Screen Saver with a very short timeout. 

##### &#9654; Follow TCD fake power

If this option is checked, and your TCD is equipped with a fake power switch, the SID will also fake-power up/down. If fake power is off, no LED is active and the SID will ignore all input from buttons, knobs and the IR control.

#### Home Assistant / MQTT settings

##### &#9654; Use Home Assistant (MQTT 3.1.1)

If checked, the SID will connect to the broker (if configured) and send and receive messages via [MQTT](#home-assistant--mqtt)

##### &#9654; Broker IP[:port] or domain[:port]

The broker server address. Can be a domain (eg. "myhome.me") or an IP address (eg "192.168.1.5"). The default port is 1883. If different port is to be used, it can be specified after the domain/IP and a colon ":", for example: "192.168.1.5:1884". Specifiying the IP address is preferred over a domain since the DNS call adds to the network overhead. Note that ".local" (MDNS) domains are not supported.

##### &#9654; User[:Password]

The username (and optionally the password) to be used when connecting to the broker. Can be left empty if the broker accepts anonymous logins.

#### Other settings

##### &#9654; Save secondary settings on SD

If this is checked, some settings (brightness, idle pattern), as well as learned IR codes are stored on the SD card. This helps to minimize write operations to the internal flash memory and to prolong the lifetime of your SID. See [Flash Wear](#flash-wear).


