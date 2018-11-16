#ifndef PORTS_HPP
#define PORTS_HPP

#include <stdlib.h>
#include <sys/time.h> 
#include <mutex>
#include <thread>

#include "OSCServer.hpp"
#include "MdnsServer.hpp"

#define PORTS_NUM_CHANNELS 8
#define PORTS_MAX_INSTANCE_COUNT 10
#define PORTS_OSC_PORT 9000

#define PORTS_TRIGGER_CYCLES 2000 /// PORTS_TIMER_PERIOD

#define PORTS_OUTPUT_LEVEL_ZERO 0
#define PORTS_OUTPUT_LEVEL_MAX 10
#define PORTS_OUTPUT_LEVEL_MIN -10
#define PORTS_TRIGGER_LEVEL PORTS_OUTPUT_LEVEL_MAX / 2
#define PORTS_LFO_FREQUENCY_MIN 0
#define PORTS_LFO_FREQUENCY_MAX 1000
#define BIPOLAR_POWER true

// Ports Modes
#define PORTS_OUTPUT_MODE_GATE 1
#define PORTS_OUTPUT_MODE_TRIG 2
#define PORTS_OUTPUT_MODE_SYNCTRIG 3
#define PORTS_OUTPUT_MODE_CVUNI 4

#define PORTS_OUTPUT_MODE_FLIPFLOP 10

#define PORTS_OUTPUT_MODE_RANDOM_SH 40

#define PORTS_OUTPUT_MODE_CVBI 50

#define PORTS_OUTPUT_MODE_LFO_SINE 71
#define PORTS_OUTPUT_MODE_LFO_SAW 81
#define PORTS_OUTPUT_MODE_LFO_RAMP 82
#define PORTS_OUTPUT_MODE_LFO_TRI 83
#define PORTS_OUTPUT_MODE_LFO_SQUARE 91

#define PORTS_INPUT_MODE_GATE 100
#define PORTS_INPUT_MODE_TRIG 101
#define PORTS_INPUT_MODE_CVUNI 102

#define PORTS_INPUT_MODE_CVBI 150




class Ports {
public:
	Ports();
	~Ports();
	void start();
	void stop();
	void computeChannel(int channel, float deltaTime);
	void setBank(int bank);
	void oscMessage(const char *path, const float value);
	static void oscMessageCallback(const char *path, const float value);
	double channelValues[PORTS_NUM_CHANNELS];
	bool channelUpdated[PORTS_NUM_CHANNELS];
	int channelModes[PORTS_NUM_CHANNELS];
private:
	int currentBank = 0;
	int numBanks = PORTS_NUM_CHANNELS / 8;
	
	int channelTrigCycles[PORTS_NUM_CHANNELS];
	bool channelSyncTriggerRequested[PORTS_NUM_CHANNELS];
	double channelLFOPhases[PORTS_NUM_CHANNELS];
	double channelLFOFrequencies[PORTS_NUM_CHANNELS];
	double channelLFOPWMs[PORTS_NUM_CHANNELS];
	bool clockFrame = false;
	struct timeval lastTimer;
	struct timeval lastReset;
	struct timeval now;
	struct timeval elapsed;
	struct timeval started;
	volatile bool shouldRun = false;
	
	void setChannelMode(int channel, bool mode, bool bipolar, bool force);
	void setChannelValue(int channel, float value);
	int parseChannel(const char *path, int offset);
	int parseOutputMode(const char *path, int offset);
	int parseInputMode(const char *path, int offset);
	bool channelIsInput(int channel);
	bool channelIsLfo(int channel);
	bool channelIsBipolar(int channel);
	int parseInt(const char *a, int offset);
	static void addInstance(Ports* instance);
	static void removeInstance(Ports* instance);
	static int instanceCount();
	static Ports* instances[PORTS_MAX_INSTANCE_COUNT];
	static std::mutex mutex;
	static MdnsServer* mdnsServer;
	static OSCServer* oscServer;

};


#endif // PORTS_HPP
