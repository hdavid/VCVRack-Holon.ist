#include "Ports.hpp"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "OSCServer.hpp"
#include "MdnsServer.hpp"

#ifdef ARCH_WIN
//define usleep for windows
#include <windows.h>
#include <string>



void usleep(int waitTime) {
    __int64 time1 = 0, time2 = 0, freq = 0;
    QueryPerformanceCounter((LARGE_INTEGER *) &time1);
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
    do {
        QueryPerformanceCounter((LARGE_INTEGER *) &time2);
    } while((time2-time1) < waitTime);
}
#endif

//initialise Ports static variables
std::mutex Ports::mutex;
MdnsServer* Ports::mdnsServer = NULL;
OSCServer* Ports::oscServer = NULL;
Ports* Ports::instances[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}; 
long Ports::totalHolonistMessageCount = 0;


Ports::Ports() {
	for (int i = 0; i < PORTS_NUM_CHANNELS; i++) {
		channelValues[i] = 0;
		channelUpdated[i] = false;
		channelModes[i] = 0;
		channelTrigCycles[i] = 0;
		channelSyncTriggerRequested[i] = false;
		channelLFOPhases[i] = 0;
		channelLFOFrequencies[i] = 0;
		channelLFOPWMs[i] = 0.5;
	}
}

Ports::~Ports() {
	stop();
}

void Ports::start() {
	if (!shouldRun){
		shouldRun = true;
		addInstance(this);
	}
}

void Ports::stop() {
	if (shouldRun){
		shouldRun = false;
		removeInstance(this);
	}
}

void Ports::computeChannel(int channel, float deltaTime) {
	// trigger and synched trigger modes
	if (PORTS_OUTPUT_MODE_TRIG == channelModes[channel] ||
			PORTS_OUTPUT_MODE_SYNCTRIG == channelModes[channel]) {
		if (channelSyncTriggerRequested[channel] && clockFrame) {
			channelTrigCycles[channel] = PORTS_TRIGGER_CYCLES; // TODO :
			channelSyncTriggerRequested[channel] = false;
		}
		if (channelTrigCycles[channel] > 0) {
			if (channelTrigCycles[channel] == PORTS_TRIGGER_CYCLES) {
			}
			channelTrigCycles[channel]--;
			if (channelValues[channel] != PORTS_TRIGGER_LEVEL) {
				channelValues[channel] = PORTS_TRIGGER_LEVEL;
				setChannelValue(channel, channelValues[channel]);
			}
		} else {
			if (channelValues[channel] != 0) {
				channelValues[channel] = 0;
				setChannelValue(channel, channelValues[channel]);
			}
		}

		// check if mode is LFO kind
	} else if (channelIsLfo(channel)) {
		if (channelLFOFrequencies[channel] > 0) { // ignore zero frequency
			// compute phase increment
			double lfoPeriod = (1.0 / channelLFOFrequencies[channel]);
			channelLFOPhases[channel] += deltaTime / lfoPeriod;
			// lfo-19 just warped. clock fram !
			if (channelLFOPhases[channel] > 1) {
				channelLFOPhases[channel] -= 1;
				if (channel == 19) {
					clockFrame = true;
				}
			}
		}
		// range and offset
		float lfo_offset = BIPOLAR_POWER ? 0.0 : PORTS_OUTPUT_LEVEL_MAX / 2; // bipolar values are [-1 1], unipolar [0, 1]
		float lfo_range = BIPOLAR_POWER ? PORTS_OUTPUT_LEVEL_MAX / 2 : PORTS_OUTPUT_LEVEL_MAX / 2; // bipolar values are [-0.5 0.5], unipolar [0, 1]

		double phase = channelLFOPhases[channel];
		switch (channelModes[channel]) {
		case PORTS_OUTPUT_MODE_LFO_SINE:
			channelValues[channel] = sin(phase * 2 * M_PI) * lfo_range + lfo_offset;
			break;
		case PORTS_OUTPUT_MODE_LFO_SAW:
			channelValues[channel] = (1 - phase * 2) * lfo_range + lfo_offset;
			break;
		case PORTS_OUTPUT_MODE_LFO_RAMP:
			channelValues[channel] = (phase * 2 - 1) * lfo_range + lfo_offset;
			break;
		case PORTS_OUTPUT_MODE_LFO_TRI:
			channelValues[channel] = ((phase < 0.5 ? (phase * 2) : (2 - phase * 2)) * 2 - 1) * lfo_range + lfo_offset;
			break;
		case PORTS_OUTPUT_MODE_LFO_SQUARE:
			channelValues[channel] =
					(phase < channelLFOPWMs[channel] ? 1 : -1) * lfo_range + lfo_offset;
			break;
		}
	}
}

void Ports::oscMessage(const char *path, const char* v) {
	int offset = 0;
	if (strncmp(path, "/a/", 3) == 0 || strncmp(path, "/b/", 3) == 0 ||
			strncmp(path, "/c/", 3) == 0 || strncmp(path, "/d/", 3) == 0 ||
			strncmp(path, "/e/", 3) == 0 || strncmp(path, "/f/", 3) == 0 ||
			strncmp(path, "/g/", 3) == 0 || strncmp(path, "/h/", 3) == 0) {

		int bank = 0;

		if (strncmp(path, "/a/", 3) == 0) {
			bank = 0;
		} else if (strncmp(path, "/b/", 3) == 0) {
			bank = 1;
		} else if (strncmp(path, "/c/", 3) == 0) {
			bank = 2;
		} else if (strncmp(path, "/d/", 3) == 0) {
			bank = 3;
		} else if (strncmp(path, "/e/", 3) == 0) {
			bank = 4;
		} else if (strncmp(path, "/f/", 3) == 0) {
			bank = 5;
		} else if (strncmp(path, "/g/", 3) == 0) {
			bank = 6;
		} else if (strncmp(path, "/h/", 3) == 0) {
			bank = 7;
		}
	
		if (bank + 1 > currentBank + numBanks) {
			return;
		}
		if (bank != currentBank) {
			return;
		}

		offset += 3;
		int channel = parseInt(path, offset);
		offset += channel < 10 ? 2 : channel < 100 ? 3 : 4;
		channel -= 1;
		
		if (channel >= 0 && channel <= PORTS_NUM_CHANNELS) {
			if (strncmp(path+offset, "input", 4) == 0) {
				//printf("%s :  %s\n", path, v);
				inputs[channel] = std::string(v);
			} else if (strncmp(path+offset, "name", 4) == 0) {
				//printf("%s :  %s\n", path, v);
				names[channel] = std::string(v);
			}else{
				//printf("XXX: %s :  %s\n", path+offset, v);
			}
		
		}
	}
}
void Ports::oscMessage(const char *path, const float v) {
	float value = v;
	int offset = 0;
	if (strncmp(path, "/a/", 3) == 0 || strncmp(path, "/b/", 3) == 0 ||
			strncmp(path, "/c/", 3) == 0 || strncmp(path, "/d/", 3) == 0 ||
			strncmp(path, "/e/", 3) == 0 || strncmp(path, "/f/", 3) == 0 ||
			strncmp(path, "/g/", 3) == 0 || strncmp(path, "/h/", 3) == 0) {

		int bank = 0;

		if (strncmp(path, "/a/", 3) == 0) {
			bank = 0;
		} else if (strncmp(path, "/b/", 3) == 0) {
			bank = 1;
		} else if (strncmp(path, "/c/", 3) == 0) {
			bank = 2;
		} else if (strncmp(path, "/d/", 3) == 0) {
			bank = 3;
		} else if (strncmp(path, "/e/", 3) == 0) {
			bank = 4;
		} else if (strncmp(path, "/f/", 3) == 0) {
			bank = 5;
		} else if (strncmp(path, "/g/", 3) == 0) {
			bank = 6;
		} else if (strncmp(path, "/h/", 3) == 0) {
			bank = 7;
		}
	
		if (bank + 1 > currentBank + numBanks) {
			return;
		}
		if (bank != currentBank) {
			return;
		}

		offset += 3;
		int channel = parseInt(path, offset);
		offset += channel < 10 ? 2 : channel < 100 ? 3 : 4;
		channel -= 1;
        int mode = parseOutputMode(path, offset);
		updateChannel(channel, mode, value);
	}
}

void Ports::updateChannel(const int channel, const int mode, const float v) {
    float value = v;
    
	if (channel >= 0 && channel <= PORTS_NUM_CHANNELS) {
		// update channel last update;
		channelUpdated[channel] = true;

		
		bool force = false;
		if (channelModes[channel] != mode) {
			channelModes[channel] = mode;
			force = true;
		}
		bool isBipolar = channelIsBipolar(channel);
		if (channelIsLfo(channel)) {
			// lfo frequency clipping
			if (value < PORTS_LFO_FREQUENCY_MIN) {
				value = PORTS_LFO_FREQUENCY_MIN;
			}
			if (value > PORTS_LFO_FREQUENCY_MAX) {
				value = PORTS_LFO_FREQUENCY_MAX;
			}
			channelLFOFrequencies[channel] = value;
			setChannelMode(channel, false, isBipolar, force);
		} else {
			// value scaling
			if (value > PORTS_OUTPUT_LEVEL_MAX) {
				value = PORTS_OUTPUT_LEVEL_MAX;
			}
			if (isBipolar) {
				if (value <= PORTS_OUTPUT_LEVEL_MIN) {
					value = PORTS_OUTPUT_LEVEL_MIN;
				}
			} else {
				if (value < PORTS_OUTPUT_LEVEL_ZERO) {
					value = PORTS_OUTPUT_LEVEL_ZERO;
				}
			}
			setChannelMode(channel, false, isBipolar, force);
			setChannelValue(channel, value);
			if (PORTS_OUTPUT_MODE_TRIG == channelModes[channel]) {
				channelTrigCycles[channel] = PORTS_TRIGGER_CYCLES;
			} else if (PORTS_OUTPUT_MODE_SYNCTRIG == channelModes[channel]) {
				channelSyncTriggerRequested[channel] = true;
			}
		}
		//} else {
		// std::cout << "invalid channel : " << path << "\n";
	}

}


void Ports::setBank(int bank) {
	if (bank != currentBank){
		currentBank = bank;
		for (int i=0;i<PORTS_NUM_CHANNELS;i++){
			names[i] = std::string();
			inputs[i] = std::string();
		}
 	}
}

int Ports::getBank() {
	return currentBank;
}

bool Ports::channelIsInput(int channel) { return channelModes[channel] >= 100; }

bool Ports::channelIsLfo(int channel) {
	return channelModes[channel] > 70 && channelModes[channel] < 100;
}

bool Ports::channelIsBipolar(int channel) {
	int mode = channelModes[channel];
	return BIPOLAR_POWER && ((mode >= 50 && mode < 100) || mode == 150);
}

void Ports::setChannelMode(int channel, bool mode, bool bipolar, bool force) {}

void Ports::setChannelValue(int channel, float value) {
	channelValues[channel] = value;
}

int Ports::parseInt(const char *a, int offset) {
	int sign, n, c;
	c = 0;
	n = 0;
	if (a[offset] == '-') { // Handle negative integers
		sign = -1;
		offset++;
	} else {
		sign = 1;
	}
	while (a[offset] != '/' && offset < (int)strlen(a)) {
		n = n * 10 + a[offset] - '0';
		offset++;
		c++;
	}
	if (sign == -1) {
		n = -n;
	}
	return n;
}

int Ports::parseOutputMode(const char *str, int offset) {
	if (strncmp(str + offset, "gate", 4) == 0) {
		return PORTS_OUTPUT_MODE_GATE;
	} else if (strncmp(str + offset, "trig", 4) == 0) {
		return PORTS_OUTPUT_MODE_TRIG;
	} else if (strncmp(str + offset, "synctrig", 8) == 0) {
		return PORTS_OUTPUT_MODE_SYNCTRIG;
	} else if (strncmp(str + offset, "flipflop", 8) == 0) {
		return PORTS_OUTPUT_MODE_FLIPFLOP;
	} else if (strncmp(str + offset, "cvbi", 4) == 0) {
		return PORTS_OUTPUT_MODE_CVBI;
	} else if (strncmp(str + offset, "cvuni", 5) == 0 || strncmp(str + offset, "cv", 2) == 0) {
		return PORTS_OUTPUT_MODE_CVUNI;
	} else if (strncmp(str + offset, "sh", 2) == 0) {
		return PORTS_OUTPUT_MODE_RANDOM_SH;
	} else if (strncmp(str + offset, "lfosaw", 6) == 0) {
		return PORTS_OUTPUT_MODE_LFO_SAW;
	} else if (strncmp(str + offset, "lforamp", 7) == 0) {
		return PORTS_OUTPUT_MODE_LFO_RAMP;
	} else if (strncmp(str + offset, "lfotri", 6) == 0) {
		return PORTS_OUTPUT_MODE_LFO_TRI;
	} else if (strncmp(str + offset, "lfosquare", 9) == 0) {
		return PORTS_OUTPUT_MODE_LFO_SQUARE;
	} else if (strncmp(str + offset, "lfosine", 7) == 0 || strncmp(str + offset, "lfo", 3) == 0) {
		return PORTS_OUTPUT_MODE_LFO_SINE;
	}
	return -1;
}

void Ports::holonistMessage(const int bus, const int channel, const int mode, const  int subMode, const float value){
	mutex.lock();
    totalHolonistMessageCount++;
    if (totalHolonistMessageCount%100==0){
        //printf("totalHolonistMessageCount:%ld\n", totalHolonistMessageCount);
    }
    if (bus>=0 && bus < 64) {
        int newMode=0;
        if (mode==0) {
            if (subMode==0) {
                newMode=PORTS_OUTPUT_MODE_CVUNI;
            } else {
                newMode=PORTS_OUTPUT_MODE_CVBI;
            }
        } else if (mode==1 ||mode==2) {
            if (subMode==0) {
                 newMode=PORTS_OUTPUT_MODE_LFO_SINE;
            } else if (subMode==0) {
                 newMode=PORTS_OUTPUT_MODE_LFO_SAW;
            } else if (subMode==1) {
                 newMode=PORTS_OUTPUT_MODE_LFO_SAW;
            } else if (subMode==2) {
                 newMode=PORTS_OUTPUT_MODE_LFO_RAMP;
            } else if (subMode==3) {
                 newMode=PORTS_OUTPUT_MODE_LFO_TRI;
            } else if (subMode==4) {
                 newMode=PORTS_OUTPUT_MODE_LFO_SQUARE;
            }
        } else if (mode==3) {
            newMode=PORTS_OUTPUT_MODE_TRIG;
        } else if (mode == 4) {
           newMode=PORTS_OUTPUT_MODE_GATE;
        }
        if (newMode==0){
            printf("unknown mode: bus:%d\tchannel:%d\tmode:%d\tsubMode:%d\tnewMode:%d\tvalue:%f\n", bus, channel, mode, subMode, newMode, value);
        }
    	for (int i = 0; i < PORTS_MAX_INSTANCE_COUNT; i++) {
    		if (instances[i] != NULL && instances[i]->getBank() == bus) {
                instances[i]->updateChannel(channel, newMode, value);            
    		}
    	}
	}
	mutex.unlock();
}


void Ports::oscMessageCallback(const char *path, const float value){
	mutex.lock();
	for (int i = 0; i < PORTS_MAX_INSTANCE_COUNT; i++) {
		if (instances[i] != NULL) {
			instances[i]->oscMessage(path, value);
		}
	}
	mutex.unlock();
}

void Ports::oscMessageStringCallback(const char *path, const char* value){
	mutex.lock();
	for (int i = 0; i < PORTS_MAX_INSTANCE_COUNT; i++) {
		if (instances[i] != NULL) {
			instances[i]->oscMessage(path, value);
		}
	}
	mutex.unlock();
}

int Ports::instanceCount() {
	int count = 0;
	mutex.lock();
	for (int i = 0; i < PORTS_MAX_INSTANCE_COUNT; i++) {
		if (instances[i] != NULL) {
			count++;
		}
	}
	mutex.lock();
	return count;
}

void Ports::addInstance(Ports* instance) {
	mutex.lock();
	int count = 0;
	for (int i = 0; i < PORTS_MAX_INSTANCE_COUNT; i++) {
		if (instances[i] != NULL) {
			count++;
		}
	}
	if (count==0){
		if (oscServer == NULL){
			oscServer = new OSCServer(9000);
			oscServer->setCallback(Ports::oscMessageCallback);
			oscServer->setStringCallback(Ports::oscMessageStringCallback);
		}
		if (mdnsServer == NULL){
			mdnsServer = new MdnsServer(9000);
		}
	}
	for (int i = 0; i < PORTS_MAX_INSTANCE_COUNT; i++) {
		if (instances[i] == NULL) {
			instances[i] = instance;
			break;
		}
	}
	mutex.unlock();
}

void Ports::removeInstance(Ports* instance) {
	mutex.lock();
	int count = 0;
	for (int i = 0; i < PORTS_MAX_INSTANCE_COUNT; i++) {
		if (instances[i] == instance) {
			instances[i] = NULL;
		}
		if (instances[i] != NULL) {
			count++;
		}
	}
	if (count==0){
		if (oscServer != NULL){
			oscServer->stop();
			oscServer->setCallback(NULL);
			oscServer->setStringCallback(NULL);
		}
		if (mdnsServer != NULL){
			mdnsServer->stop();
		}
		if (oscServer != NULL && mdnsServer != NULL) {
			int i = 0;
			int wait = 50;
			while ((oscServer->running || mdnsServer->running) && i * wait < 2000){
				//wait for threads to complete but not longer than 2secs.
				usleep(1000 * wait);
				i++;
			}
			// add 100ms just in case.
			usleep(1000 * 100);
		}
		if (oscServer != NULL){
			delete oscServer;
			oscServer = NULL;
		}
		if (mdnsServer != NULL){
			delete mdnsServer;
			mdnsServer = NULL;
		}
	}
	mutex.unlock();
}
