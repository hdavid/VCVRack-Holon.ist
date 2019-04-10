# Holonic Systems - VCV Rack Free Plugins

Holon.ist from http://holonic.systems can communicate with various virtual and physical studio gear using MIDI and OSC protocols. Download the latest beta version from: https://itunes.apple.com/us/app/holon-ist/id1359763686?mt=8

##### Table of Contents  
- [Holonic Source for VCV Rack](#holonic-source)
- [Dumbwaiter - Sequencer/Switch](#Dumbwaiter)
- [Lazy Susan - Quad Quantiser](#Lazy-Susan)
- [Pantry - Dual CV/Gate Recorder/Looper](#pantry)
- [Swiss Cheese Knife - Quad Utility with a twist](#swiss-cheese-knife)
- [Gaps - Multimode Clock Divider](#gaps)
- [Junctions - simple dual switch](#Junctions)



## Holonic Source

![Holonic Source](https://raw.githubusercontent.com/hdavid/VCVRack-Holon.ist/master/screencaps/HolonicSource.png)    [![Holon.ist Receiver Demo](http://img.youtube.com/vi/eBonU85BfDM/0.jpg)](https://www.youtube.com/watch?v=eBonU85BfDM "Holon.ist Receiver Demo")

Holonic Source module for VCV Rack integrates with the Holon.ist iOS app and outputs CV control signals to VCV Rack.
- 8 channels of CV control from Holon.ist to VCV Rack.
- Per channel activity indicator, attenuator and low pass filter.
- Multiple Holonic Source modules can run at the same time. Each instance can be set to receive on its own bus, from A to H, allowing for a total of 64 channels of CV.

### Demo Patches
We've put together some VCV demo patches that use the Holonic Source. They use the default mappings in the Holon.ist app.

Our demos are packaged with the plugins, they are located in: `c:\Users\<you>\Documents\Rack\Plugins\HolonicSystems-Free\demos` (Windows) `/Users/<you>/Documents/Rack/plugins/HolonicSystems-Free/demos` (MacOS) 

The demo patches require the following VCV Rack plugins to be installed: 

- Amalgamated Harmonics
- Audible Instruments
- Befaco
- Holonic Systems - Free
- JW Modules
- ML Modules
- MSM
- Valley
- Vult Modules - Free
- Stellare Link

See also http://holon.ist/demos/

### Installing
Please check the following installation steps below to avoid common pitfalls, such as firewalls, and to ensure full functionality of the autodetection feature.

- Install the latest version of Holon.ist on your iPhone or iPad from: http://holonic.systems or https://testflight.apple.com/join/mBx4PTxL
- Install the latest version of VCV rack: https://vcvrack.com/
- Install the Holonic Systems plugin suite from the VCV Rack plugin manager: https://vcvrack.com/plugins.html#holonic
- Follow the instruction below  to establish communication between Holon.ist and Holonic Source.

### OSC Communication

mDNS/bonjour is used for autodiscovery.

- MacOS 
  - Holon.ist automatically detects VCV Rack when the Holonic Source plugin is loaded.
- Windows 
  - requires Bonjour SDK from Apple to be installed: https://developer.apple.com/bonjour/  
  - Check that the Bonjour Service is running.
  - Run the script `Holonic_Source_windows.bat`. This script can be found in the plugin folder `c:\Users\<you>\Documents\Rack\Plugins\HolonicSystems-Free\` The script publishes Holonic Source on the network to allow autodiscovery from Holon.ist app.
  - The DOS window needs to remain open for the duration of the VCV session.
- Linux 
  - Ensure Avahi is running.
  - Run the script. `Holonic_Source_linux.sh`. This publishes Holonic Source on the network to allow autodiscovery.
  - The shell window needs to remain open for the duration of the VCV session.

Tips
- Ensure that firewalls, such as Little Snitch, are not blocking communication. For instance, on MacOS, with Little Snitch firewall, incoming communication must be specifically enabled for Rack.
- If you cannot get autodiscovery to work for some reason, you can always manually create an OSC output in Holon.ist and input your computer IP and port 9000.
- Please read the Holon.ist manual http://holon.ist/manual/

#### OSC Message Format

The Holonic Source OSC message format is used to ensure close integration with our Holon.ist app. In order to use Holonic Source with any other OSC compatible application, please select port 9000.

The message OSC path is `/<bus>/<channel>/<mode>` where:
- `<bus>` is one of `a`/`b`/`c`/`d`/`e`/`f`/`g`/`h`
- `<channel>` is one of `1`/`2`/`3`/`4`/`5`/`6`/`7`/`8`
- `<mode>` is one of `cv` for unipolar CV, `cvbi` for bipolar CV.
- For instance `/a/2/cv` will address channel 2 of bus a in CV mode. 
- Argument value should be a float. Holonic Source accepts values in [-1,1] range or in [-10,10] range depending on the switch configuration in the GUI. When [-1,1] is selected, values will effectively be multiplied by 10.


### Receiving Bus
The Receiving bus selector determines from which bus the module receives signals from Holon.ist. This allows use of more than one instance of the module in the patch, providing up to 64 channels of voltage control in total.

### Activity LEDs
Activity LEDs for each channel indicate when Holonic Source receives OSC messages for the particular channel and receiving bus.

### Attenuators
Attenuators on each channel let one scale CV values according to need.  

### Low Pass Filters
An adjustable slew is included to smooth signals as desired.

### Outputs
VCV rack expects values between 0v and +10v for unipolar and between -5v and +5v for bipolar signals.

Holonic Source clips signals to -10v, +10v.
  
Ensure that values are properly scaled in the Holon.ist scaling section.


## Dumbwaiter

![Dumbwaiter](https://raw.githubusercontent.com/hdavid/VCVRack-Holon.ist/master/screencaps/Dumbwaiter.png).

Dumbwaiter is a addressable 8 step sequencer and switch. One could see it as the lower row of the Doepfer A-155 with a large helping of A-154 functionality.

## Lazy Susan

![Lazy Susan](https://raw.githubusercontent.com/hdavid/VCVRack-Holon.ist/master/screencaps/LazySusan.png)

Lazy Susan is a quad quantiser with user editable scales.

For your convenience, your custom scales are saved with each patch.

By default, the scales are the 7 standard modes: Ionian, Dorian, Phrygian, Lydian, Mixolydian, Aeolian, Locrian.

Serving suggestion: The Doepfer A-156 Quantiser minor mode is a melodic minor scale (CmMaj7). Choose the Dorian scale and sharpen the 7th to harmonize with your Eurorack.

## Pantry

![Pantry](https://raw.githubusercontent.com/hdavid/VCVRack-Holon.ist/master/screencaps/Pantry.png)

Pantry is a Dual CV/Gate Recorder/Looper.

Features:
- Dual Channel. 
- CV and Gate Recording.
- Record: Full loop length recording via cv or button
- Overdub: Record while CV/button is held.
- Pattern Length from 1 to 32 using CV.
- CV control of Pattern Shift.


## Swiss Cheese Knife

![SwissCheeseKnife](https://raw.githubusercontent.com/hdavid/VCVRack-Holon.ist/master/screencaps/SwissCheeseKnife.png)

SwissCheeseKnife is a fully featured quad utility module.

### Inverter
Inverts input signal.

### Attenuator 
Signal Attenuator.

### Sample and Hold
Classic Sample and Hold.

### VCA
VCA with variable response shape ranging from linear to exponential.

### Low Pass Filter
Simple low pass filter.

### Slew Limiter
Slew limiter with independently adjustable slope shape and Rise and Fall times.

### DC Offset Remover
While in AC mode, any DC offset is removed. 

### Offset
Signal offset.

### Mixer
Last on the signal path, but not least. Swiss Cheese Knife can also mix four signals, as its outputs are normalled.

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

A simple dual switch with two inputs and one output.
