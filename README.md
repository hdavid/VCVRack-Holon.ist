# Holonic Systems - VCV Rack Free Plugins

Holon.ist from http://holonic.systems can communicate with various virtual and physical studio gear using MIDI and OSC protocols. Download the latest beta version from: https://testflight.apple.com/join/mBx4PTxL

##### Table of Contents  
- [Holon.ist Receiver for VCV Rack](#holonist-receiver)
- [Pantry - Dual CV/Gate Recorder/Looper](#pantry)
- [Swiss Cheese Knife - Quad Utility with a twist](#swiss-cheese-knife)
- [Gaps - Multimode Clock Divider](#gaps)
- [Junctions - Dual Switch](#junctions)


## Holon.ist Receiver

![Holon.ist Receiver](https://raw.githubusercontent.com/hdavid/VCVRack-Holon.ist/master/screencaps/Holon.ist-Receiver.png)    [![Holon.ist Receiver Demo](http://img.youtube.com/vi/eBonU85BfDM/0.jpg)](https://www.youtube.com/watch?v=eBonU85BfDM "Holon.ist Receiver Demo")

Holon.ist Receiver VCV Rack Module integrates with the Holon.ist iOS app and outputs CV control signals to VCV Rack.
- 8 channels of CV control from Holon.ist to VCV Rack.
- Per channel activity indicator, attenuverter and low pass filter.
- Multiple Holon.ist receiver modules can run at the same time. Each instance can be set to receive on its own bank, from A to H, allowing for a total of 64 channels of CV.

### Demos
We've put together some VCV demo patches that use the Holon.ist Receiver. They use the default mappings in the Holon.ist app.

Our demos are packaged with the plugins, they are located in: `c:\Users\<you>\Documents\Rack\Plugins\HolonicSystems-Free\demos` (Windows) `/Users/<you>/Documents/Rack/plugins/HolonicSystems-Free/demos` (MacOS) 

The demo patches require the following VCV Rack plugins to be installed: 

Amalgamated Harmonics
Audible Instruments
Befaco
Holonic Systems - Free
JW Modules
ML Modules
Valley
Vult Modules - Free 

### Installing
Please check the following installation steps below to avoid common pitfalls, such as firewalls, and to ensure full functionality of the autodetection feature.

- Install the latest version of Holon.ist on your iPhone or iPad from: http://holonic.systems or https://testflight.apple.com/join/mBx4PTxL
- Install the latest version of VCV rack: https://vcvrack.com/
- Install the Holon.ist Receiver plugin from the VCV Rack plugin manager: https://vcvrack.com/plugins.html#holonic
- Follow the instruction below  to establish communication between Holon.ist and Receiver.

### OSC Communication
mDNS/bonjour is used for autodiscovery.

- On MacOS Holon.ist automatically detects VCV Rack when Holon.ist Receiver plugin is loaded.
- Windows requires Bonjour SDK from Apple to be installed: https://developer.apple.com/bonjour/  
- Check that the Bonjour Service is running
- Run the script `scripts\Holon.ist receiver bonjour.bat`. This script can be found in the plugin folder `c:\Users\<you>\Documents\Rack\Plugins\HolonicSystems-Free\` The script publishes Holon.ist Receiver on the network to allow autodiscovery from Holon.ist app.
- On Linux you need to have Avahi running (it is usually the case), then run the script. `scripts\Holon.ist_receiver_avahi.sh`. This publishes Holon.ist Receiver on the network to allow autodiscovery.

Tips
- Ensure that firewalls, such as Little Snitch, are not blocking communication. For instance, on MacOS, with Little Snitch firewall, incoming communication must be specifically enabled for Rack.
- If you cannot get auto discovery to work for some reason, you can always manually create an OSC output in Holon.ist and input your computer IP and port 9000.
- Please read the Holon.ist manual http://holon.ist/manual/


### Receiving Bank
The Receiving bank pot selects from which bank the module receives signals from Holon.ist. This allows use of more than one instance of the module in the patch, providing up to 64 channels of voltage control in total.

### Activity LEDs
Activity LEDs for each channel indicate when Holon.ist Receiver receives OSC messages for the particular channel and receiving bank.

Note that output jacks will also show output values visually, but for this to happen, a cable needs to be plugged in the jack.

### Attenuators
Attenuators on each channel let one scale and invert CV values according to need.  

### Low Pass Filters
The slew is adjustable, in order to avoid stepped signals, and to make more natural CV changes.

### Outputs
VCV rack expects values between 0v and +10v for unipolar and between -5v and +5v for bipolar signals.

Holon.ist receivers clips signals to -10v, +10v.
  
Ensure that values are properly scaled in the Holon.ist scaling section.

## Pantry

![Pantry](https://raw.githubusercontent.com/hdavid/VCVRack-Holon.ist/master/screencaps/Pantry.png)

Pantry is a Dual CV/Gate Recorder/Looper.

Features:
- Dual Channel. 
- CV and Gate Recording.
- Record: Full loop length triggered or the button is pressed.
- Overdub: Record while CV/button is held.
- Pattern Length from 1 to 32 using CV.
- CV control of Pattern Shift.


## Swiss Cheese Knife

![SwissCheeseKnife](https://raw.githubusercontent.com/hdavid/VCVRack-Holon.ist/master/screencaps/SwissCheeseKnife.png)

SwissCheeseKnife is a quad utility module with a twist.

### Inverter
Inverts input signal.

### Attenuator 
Signal attenuator.

### Sample and Hold
Classic Trigger and Hold.

### VCA
VCA with response shape ranging from linear to exponential.

### Low Pass Filter
Simple low pass filter.

### Slew Limiter
Slew limiter.

### DC Offset Remover
While in AC mode, any DC offset is removed. 

### Offset
Signal offset.

### Mixer
Last on the signal path, but not least. SwissCheeseKnife can also mix 4 signals, as its outputs are normalled.


## Gaps

![Gaps](https://raw.githubusercontent.com/hdavid/VCVRack-Holon.ist/master/screencaps/Gaps.png)

Gaps is a clock divider that supports multiple modes. 

- Integers: 2,3,4,5,6,7,8,9
- Even : 2,4,6,8,10,12,14,16
- Odd: 3,5,7,9,11,13,15,17
- Prime: 2,3,5,7,11,13,17,19
- Binary: 2,4,8,16,32,64,128,256
- Random: random probabilities 1/2, 1/3, 1/4, 1/5, 1/6, 1/7, 1/8, 1/9
- 8 step clock Sequencer


## Junctions

![Junctions](https://raw.githubusercontent.com/hdavid/VCVRack-Holon.ist/master/screencaps/Junctions.png)

A simple dual switch. Two inputs, one output.
