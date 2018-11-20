@ECHO OFF
title mDNS for VCV Rack Holon.ist Receiver

ECHO Registers VCV Rack Holon.ist Receiver using Bonjour-SDK
ECHO install from https://www.softpedia.com/get/Programming/SDK-DDK/Bonjour-SDK.shtml
ECHO or https://developer.apple.com/bonjour/
ECHO registering OSC Service...
ECHO don't close this window if you want your service to be registered on the network
dns-sd -R vcvrack-%COMPUTERNAME% _osc._udp local 9000