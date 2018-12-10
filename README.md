# Holonic Systems - VCV Rack Free Plugins

##### Table of Contents  
- [Holon.ist Receiver for VCV Rack](##Holon.ist Receiver)
- [Past - Dual CV/Gate Recorder/looper](##Past)
- [Swiss Cheese Knife - Quad Utility with a twist](##Swiss Cheese Knife)
- [Gaps - Quad Utility with a twist](##Gaps)
- [Junctions - dual switch](##Junctions)


## Holon.ist Receiver

![Holon.ist Receiver](https://raw.githubusercontent.com/hdavid/VCVRack-Holon.ist/master/screencaps/Holon.ist-Receiver.png)

Holon.ist from http://holonic.systems can communicate with various virtual and physical studio gear using MIDI and OSC protocols.

Holon.ist Receiver VCV Rack Module integrates with Holon.ist iOS app and outputs CV control signals in VCV Rack world.
- 8 channels of CV control from Holon.ist to your VCV rack
- Per channel activity indicator, attenuverter and low pass filter
- Multiple instances of the module can run at the same time. Each instance can be set to receive on its own bank, from A to H, allowing for a total of 64 channels of CV control.


### Installing
- Last version of Holon.ist is installed on your iPhone or iPad : http://holonic.systems
- Last version of VCV rack installed : https://vcvrack.com/
- Install Holon.ist Receiver plugin from VCV Rack plugin manager and : https://vcvrack.com/plugins.html
- follow the instruction below how to get Holon.ist and the receiver to talk to each other.

### OSC Communication
mDNS/bonjour is used for autodiscovery.

- On MacOS Holon.ist will automatically detect VCV Rack if Holon.ist receiver plugin is loaded !
- On Windows you need to install Bonjour SDK from Apple and check that the Bonjour Service is running. Then run the script `scripts\Holon.ist receiver bonjour.bat`. This will publish Holon.ist Receiver on the network and allow autodiscovery from Holon.ist App.
- On Linux you need to have Avahi running (usually it is the case). Then run the script `scripts\Holon.ist_receiver_avahi.sh`. This will publish Holon.ist Receiver on the network and allow autodiscovery

Tips
- Make sure firewall like little snitch or other are not blocking communication. For instance, on MasOS, with Little Snitch firewall, incoming communication must be specifically enabled for Rack.
- If you cannot get auto discovery to work for some reason, you can always create manually an OSC output in Holon.ist and input your computer IP and port 9000
- Be sure to Check Holon.ist manual http://holon.ist/manual/

### Receiving Bank
The Receiving bank knob selects on which bank the module is receiving signals from Holon.ist. This allows to use more than one instance of the module in your patch, allowing for up to 64 channels of cv control in total!

### Activity LEDs
Activity leds for each channel indicate when Holon.ist Receiver is receiving OSC message for this channel and receiving bank.

Note that output jacks will also show output value graphically, but for this to happen, a cable must be plugged in the jack.

### Attenuverters
Attenuverter on each channels allow to scale and invert CV values to suit your needs.  

### Low Pass Filters
In Order to avoid stair cases in the signal, and to bring more organic CV changes, you can adjust the slews

### Outputs
VCV rack expects values between 0v and +10v for unipolar and between -5v and +5v for bipolar signals.

Holon.ist receivers clips signals to -10v, +10v.
  
Make sure to scale values properly in the scaling section in Holon.ist

## Past

![Past](https://raw.githubusercontent.com/hdavid/VCVRack-Holon.ist/master/screencaps/Past.png)

Past is a Dual CV/Gate Recorder/Looper.

Features :
- Dual Channel 
- CV and Gate
- Record: full loop length when CV rised or button is pressed
- Overdub: record or record while CV/button is held
- Length from 1 to 32 with CV
- Shifting with CV control


## Swiss Cheese Knife

![SwissCheeseKnife](https://raw.githubusercontent.com/hdavid/VCVRack-Holon.ist/master/screencaps/SwissCheeseKnife.png)

SwissCheeseKnife is a Quad utility module with a twist.

### Inverter
Inverts input signal.

### Attenuator 
Attenuator with range [0, 1] ratios.

### Sample and Hold
Classic Trigger and Hold.

### VCA
VCA with response shape from linear to exponential.

### Low Pass Filter
Simple low pass filter

### Slew Limiter
Slew limiter

### DC Offset Remover
While in AC mode, any DC offset is removed. 

### Offset
Offset signal

### Mixer
Last on the signal path, but not least. SwissCheeseKnife can also mix 4 signals as its outputs are normaled.


## Gaps

![Gaps](https://raw.githubusercontent.com/hdavid/VCVRack-Holon.ist/master/screencaps/Gaps.png)

Gaps is a clock divider that supports multiple modes 

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
