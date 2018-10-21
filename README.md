# Holon.ist Receiver for VCV Rack

Holon.ist from http://holonic.systems can communicate with various virtual and physical studio gear using MIDI and OSC protocols.

Holon.ist Receiver VCV Rack Module integrates with Holon.ist iOS app and outputs CV control signals in VCV Rack world.
- 8 channels of CV control from Holon.ist to your VCV rack
- Per channel activity indicator, attenuverter and low pass filter
- Multiple instances of the module can run at the same time. Each instance can be set to receive on its own bank, from A to H, allowing for a total of 64 channels of CV control.


## Installing
- Last version of Holon.ist is installed on your iPhone or iPad : http://holonic.systems
- Last version of VCV rack installed : https://vcvrack.com/
- Install Holon.ist Receiver plugin from VCV Rack plugin manager and : https://vcvrack.com/plugins.html

## OSC Communication
- If VCV Rack and Holon.ist Receiver are running MacOS, Holon.ist will automatically detect VCV Rack !
- On Windows and Linux (for now) you will have have get your ip (https://www.digitalcitizen.life/find-ip-address-windows ), and manually setup an OSC destination in Holon.ist. Holon.ist Receiver listens on port 9000
- Make sure firewall like little snitch or other are not blocking communication. For instance, on MasOS, with Little Snitch firewall, incoming communication must be specifically enabled for Rack.
- Make sure to check Holon.ist manual http://holon.ist/manual/

## Receiving Bank
The Receiving bank knob selects on which bank the module is receiving signals from Holon.ist. This allows to use more than one instance of the module in your patch, allowing for up to 64 channels of cv control in total!

## Activity LEDs
Activity leds for each channel indicate when Holon.ist Receiver is receiving OSC message for this channel and receiving bank.

Note that output jacks will also show output value graphically, but for this to happen, a cable must be plugged in the jack.

## Attenuverters
Attenuverter on each channels allow to scale and invert CV values to suit your needs.  

## Low Pass Filters
In Order to avoid stair cases in the signal, and to bring more organic CV changes, you can adjust the slews

## Outputs
VCV rack expects values between 0v and +10v for unipolar and between -5v and +5v for bipolar signals.

Holon.ist receivers clips signals to -10v, +10v.
  
Make sure to scale values properly in the scaling section in Holon.ist
