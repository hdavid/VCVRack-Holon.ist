#include "HolonicSystems-Free.hpp"
#include "dsp/digital.hpp"



struct HolonicSystemsPantryModule : Module {

	enum ParamIds {
		PARAM_RECORD_1,
		PARAM_RECORD_2,
		PARAM_OVERDUB_1,
		PARAM_OVERDUB_2,
		PARAM_CLEAR_1,
		PARAM_CLEAR_2,
		PARAM_LENGTH_1,
		PARAM_LENGTH_2,
		PARAM_SHIFT_1,
		PARAM_SHIFT_2,
		NUM_PARAMS
	};

	enum InputIds {
		INPUT_CLOCK,
		INPUT_RESET,
		INPUT_RECORD_1,
		INPUT_RECORD_2,
		INPUT_OVERDUB_1,
		INPUT_OVERDUB_2,
		INPUT_CLEAR_1,
		INPUT_CLEAR_2,
		INPUT_SHIFT_1,
		INPUT_SHIFT_2,
		INPUT_LENGTH_1,
		INPUT_LENGTH_2,
		INPUT_CV_1,
		INPUT_CV_2,
		INPUT_GATE_1,
		INPUT_GATE_2,
		NUM_INPUTS
	};

	enum OutputIds {
		OUTPUT_CV_1,
		OUTPUT_CV_2,
		OUTPUT_GATE_1,
		OUTPUT_GATE_2,
		NUM_OUTPUTS
	};
	
	enum LightIds {
		LIGHT_RECORDING_1,
		LIGHT_RECORDING_2,
		NUM_LIGHTS
	};	
	
	
	LooseSchmittTrigger clockTrigger;
	LooseSchmittTrigger resetTrigger;

	long counters[2] = {0,0};
	int recordingSteps[2] = {-1,-1};

	LooseSchmittTrigger recordTrigger[2];
	LooseSchmittTrigger overdubTrigger[2];
	LooseSchmittTrigger clearTrigger[2];

	std::vector<float> cvs[2*16] = {std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64)};
	std::vector<float> gates[2*16]= {std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64)};


	HolonicSystemsPantryModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PARAM_RECORD_1,0.f, 1.f, 0.f, "Record 1");
		configParam(PARAM_OVERDUB_1,0.f, 1.f, 0.0f, "Overdub 1");
		configParam(PARAM_CLEAR_1,0.f, 1.f, 0.0f, "Clear 1");
		configParam(PARAM_LENGTH_1,0.f, 31.f, 16.f, "Lenth 1");
		configParam(PARAM_SHIFT_1,0.f, 31.f, 0.f, "Shift 1");
		configParam(PARAM_RECORD_2,0.f, 1.f, 0.f, "Record 2");
		configParam(PARAM_OVERDUB_2,0.f, 1.f, 0.0f, "Overdub 2");
		configParam(PARAM_CLEAR_2,0.f, 1.f, 0.0f, "Clear 2");
		configParam(PARAM_LENGTH_2,0.f, 31.f, 16.f, "Lenth 2");
		configParam(PARAM_SHIFT_2,0.f, 31.f, 0.f, "Shift 2");
		onReset();
	}


	~HolonicSystemsPantryModule() {
	}

	void onReset() override {
		for (int i=0; i<2; i++) {
			counters[i] = 0;
			for (int channel=0; channel<16; channel++){
				for (int j=0;j<64;j++){
					cvs[i+channel*2][j] = 0;
					gates[i+channel*2][j]= 0;
				}
			}
		}
	}


	void process(const ProcessArgs &args) override {
	
		bool clock = clockTrigger.process(inputs[INPUT_CLOCK].value);
		bool reset = resetTrigger.process(inputs[INPUT_RESET].value);
		
		for (int i = 0; i<2;i++){
			
			int channels = fmax(inputs[INPUT_CV_1+i].getChannels(), inputs[INPUT_GATE_1+i].getChannels());
			
			//triggers must process at each step
			bool rec = recordTrigger[i].process(inputs[INPUT_RECORD_1+i].value) || params[PARAM_RECORD_1+i].value>0;
			bool x = overdubTrigger[i].process(inputs[INPUT_OVERDUB_1+i].value);
			x = x && rec;//silence warning
			bool overdub =  overdubTrigger[i].isHigh() || params[PARAM_OVERDUB_1+i].value>0;
			bool clear = clearTrigger[i].process(inputs[INPUT_CLEAR_1+i].value) || params[PARAM_CLEAR_1+i].value>0;
			
			//clear at any time, not on clock
			if (clear){
				for (int channel=0; channel<16; channel++){
					for (int step=0; step<32; step++){
						cvs[i+channel*2][step] = 0.0f;
						gates[i+channel*2][step] = 0.0f;
					}
				}
			}
			
			if (reset){
				counters[i] = 0;
			}
			
			//recording
			bool recording = overdub;
			if (recordingSteps[i]==-1 && rec) {
				//trigger new full length recording only if not already recording
				recordingSteps[i]=0;
				recording = true;
			}
			
			if (clock) {
				
				int length = clamp( ((int)inputs[INPUT_LENGTH_1+i].value) + ((int)params[PARAM_LENGTH_1+i].value), 1.0, 32.0);
				int shift = ((int)inputs[INPUT_SHIFT_1+i].value) + ((int)params[PARAM_SHIFT_1+i].value);
				
				//warp count
				counters[i] = (counters[i]) % length;
				int step = (counters[i]+shift) % 32;//TODO: wrap within loop or within all 32 steps ?
				
				//recording
				if (recordingSteps[i]!= -1){
					recordingSteps[i]++;
					if (recordingSteps[i]<=length){
						recording = true;
					} else {
						//recording ended
						recordingSteps[i] = -1;
					}
				}
				
				lights[LIGHT_RECORDING_1+i].setBrightness(recording?10:0);
				
				//recording
				if (recording){
					for (int channel=0; channel<channels; channel++){
						cvs[i+channel*2][step]=inputs[INPUT_CV_1+i].getVoltage(inputs[INPUT_CV_1+i].getChannels()>channel?channel:0);
						gates[i+channel*2][step]=inputs[INPUT_GATE_1+i].getVoltage(inputs[INPUT_GATE_1+i].getChannels()>channel?channel:0);
					}
				}
				
				//output
				outputs[OUTPUT_CV_1+i].setChannels(channels);
				outputs[OUTPUT_GATE_1+i].setChannels(channels);
				for (int channel=0; channel<channels; channel++){
					outputs[OUTPUT_CV_1+i].setVoltage(cvs[i+channel*2][step], channel);
					outputs[OUTPUT_GATE_1+i].setVoltage(gates[i+channel*2][step], channel);
				}
				
				//step
				counters[i]++;
			}
		}
	}
	 
};





struct HolonicSystemsPantryWidget : ModuleWidget {
	HolonicSystemsPantryWidget(HolonicSystemsPantryModule *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HolonicSystems-Pantry.svg")));
		
		//screws
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
 
		//IN
		addInput(createInput<PJ301MPort>(Vec(10, 20), module, HolonicSystemsPantryModule::INPUT_CLOCK));
		addInput(createInput<PJ301MPort>(Vec(90, 20), module, HolonicSystemsPantryModule::INPUT_RESET));
		
		for (int i=0;i<2;i++){
			
			int rowHeight= 155;
			int vSpace=38;
			int base=65;
		
			addParam(createParam<TL1105>(Vec(10+vSpace*0+5, base+0 + i*rowHeight), module, HolonicSystemsPantryModule::PARAM_RECORD_1+i));
			addInput(createInput<PJ301MPort>(Vec(10+vSpace*0, base+20 + i*rowHeight), module, HolonicSystemsPantryModule::INPUT_RECORD_1+i));
			
			addParam(createParam<TL1105>(Vec(10+vSpace*1+5, base+0 + i*rowHeight), module, HolonicSystemsPantryModule::PARAM_OVERDUB_1+i));
			addInput(createInput<PJ301MPort>(Vec(10+vSpace*1, base+20 + i*rowHeight), module, HolonicSystemsPantryModule::INPUT_OVERDUB_1+i));
			
			addParam(createParam<TL1105>(Vec(10+vSpace*2+5, base+0 + i*rowHeight), module, HolonicSystemsPantryModule::PARAM_CLEAR_1+i));
			addInput(createInput<PJ301MPort>(Vec(10+vSpace*2, base+20 + i*rowHeight), module, HolonicSystemsPantryModule::INPUT_CLEAR_1+i));
			
			base += 50;
			
			//channel
			addInput(createInput<PJ301MPort>(Vec(10+vSpace*0, base + i*rowHeight), module, HolonicSystemsPantryModule::INPUT_CV_1+i));
			addInput(createInput<PJ301MPort>(Vec(10+vSpace*0, base+30 + i*rowHeight), module, HolonicSystemsPantryModule::INPUT_GATE_1+i));
			
			addOutput(createOutput<PJ301MPort>(Vec(10+vSpace*2, base + i*rowHeight), module, HolonicSystemsPantryModule::OUTPUT_CV_1+i));
			addOutput(createOutput<PJ301MPort>(Vec(10+vSpace*2, base+30 + i*rowHeight), module, HolonicSystemsPantryModule::OUTPUT_GATE_1+i));
			
			addChild(createLight<LargeLight<RedLight>>(	Vec(10+vSpace*1+5, base+20 + i*rowHeight), module, HolonicSystemsPantryModule::LIGHT_RECORDING_1+i));
			
			base += 60;
			
			HolonicSystemsKnob * lengthKnob = dynamic_cast<HolonicSystemsKnob*>(createParam<HolonicSystemsKnob>(Vec(10+vSpace*0-5, base + i*rowHeight), module, HolonicSystemsPantryModule::PARAM_LENGTH_1+i));
			HolonicSystemsLabel* const lengthLabel = new HolonicSystemsLabel;
			lengthLabel->box.pos = Vec((10+vSpace*0-5)/2, (base + i*rowHeight)/2+18);
			lengthKnob->names.push_back(std::string("length 0"));//this is needed, values start at 1...
			lengthKnob->names.push_back(std::string("length 1"));
			lengthKnob->names.push_back(std::string("length 2"));
			lengthKnob->names.push_back(std::string("length 3"));
			lengthKnob->names.push_back(std::string("length 4"));
			lengthKnob->names.push_back(std::string("length 5"));
			lengthKnob->names.push_back(std::string("length 6"));
			lengthKnob->names.push_back(std::string("length 7"));
			lengthKnob->names.push_back(std::string("length 8"));
			lengthKnob->names.push_back(std::string("length 9"));
			lengthKnob->names.push_back(std::string("length 10"));
			lengthKnob->names.push_back(std::string("length 11"));
			lengthKnob->names.push_back(std::string("length 12"));
			lengthKnob->names.push_back(std::string("length 13"));
			lengthKnob->names.push_back(std::string("length 14"));
			lengthKnob->names.push_back(std::string("length 15"));
			lengthKnob->names.push_back(std::string("length 16"));
			lengthKnob->names.push_back(std::string("length 17"));
			lengthKnob->names.push_back(std::string("length 18"));
			lengthKnob->names.push_back(std::string("length 19"));
			lengthKnob->names.push_back(std::string("length 20"));
			lengthKnob->names.push_back(std::string("length 21"));
			lengthKnob->names.push_back(std::string("length 22"));
			lengthKnob->names.push_back(std::string("length 23"));
			lengthKnob->names.push_back(std::string("length 24"));
			lengthKnob->names.push_back(std::string("length 25"));
			lengthKnob->names.push_back(std::string("length 26"));
			lengthKnob->names.push_back(std::string("length 27"));
			lengthKnob->names.push_back(std::string("length 28"));
			lengthKnob->names.push_back(std::string("length 29"));
			lengthKnob->names.push_back(std::string("length 30"));
			lengthKnob->names.push_back(std::string("length 31"));
			lengthKnob->names.push_back(std::string("length 32"));
			if (module){
				lengthKnob->connectLabel(lengthLabel);
			}
			addChild(lengthLabel);
			addParam(lengthKnob);
			
			addInput(createInput<PJ301MPort>(Vec(10+vSpace*0+23, base + i*rowHeight), module, HolonicSystemsPantryModule::INPUT_LENGTH_1+i));
			
			addInput(createInput<PJ301MPort>(Vec(10+vSpace*2-23, base + i*rowHeight), module, HolonicSystemsPantryModule::INPUT_SHIFT_1+i));		
			
			HolonicSystemsKnob * shiftKnob = dynamic_cast<HolonicSystemsKnob*>(createParam<HolonicSystemsKnob>(Vec(10+vSpace*2+5, base + i*rowHeight), module, HolonicSystemsPantryModule::PARAM_SHIFT_1+i));
			HolonicSystemsLabel* const shiftLabel = new HolonicSystemsLabel;
			shiftLabel->box.pos = Vec((10+vSpace*2+5)/2-10, (base + i*rowHeight)/2+18);
			shiftKnob->names.push_back(std::string("shift 0"));
			shiftKnob->names.push_back(std::string("shift 1"));
			shiftKnob->names.push_back(std::string("shift 2"));
			shiftKnob->names.push_back(std::string("shift 3"));
			shiftKnob->names.push_back(std::string("shift 4"));
			shiftKnob->names.push_back(std::string("shift 5"));
			shiftKnob->names.push_back(std::string("shift 6"));
			shiftKnob->names.push_back(std::string("shift 7"));
			shiftKnob->names.push_back(std::string("shift 8"));
			shiftKnob->names.push_back(std::string("shift 9"));
			shiftKnob->names.push_back(std::string("shift 10"));
			shiftKnob->names.push_back(std::string("shift 11"));
			shiftKnob->names.push_back(std::string("shift 12"));
			shiftKnob->names.push_back(std::string("shift 13"));
			shiftKnob->names.push_back(std::string("shift 14"));
			shiftKnob->names.push_back(std::string("shift 15"));
			shiftKnob->names.push_back(std::string("shift 16"));
			shiftKnob->names.push_back(std::string("shift 17"));
			shiftKnob->names.push_back(std::string("shift 18"));
			shiftKnob->names.push_back(std::string("shift 19"));
			shiftKnob->names.push_back(std::string("shift 20"));
			shiftKnob->names.push_back(std::string("shift 21"));
			shiftKnob->names.push_back(std::string("shift 22"));
			shiftKnob->names.push_back(std::string("shift 23"));
			shiftKnob->names.push_back(std::string("shift 24"));
			shiftKnob->names.push_back(std::string("shift 25"));
			shiftKnob->names.push_back(std::string("shift 26"));
			shiftKnob->names.push_back(std::string("shift 27"));
			shiftKnob->names.push_back(std::string("shift 28"));
			shiftKnob->names.push_back(std::string("shift 29"));
			shiftKnob->names.push_back(std::string("shift 30"));
			shiftKnob->names.push_back(std::string("shift 31"));
			if (module){
				shiftKnob->connectLabel(shiftLabel);
			}
			addChild(shiftLabel);
			addParam(shiftKnob);
			

		}
	}

};


Model *modelPantry = createModel<HolonicSystemsPantryModule, HolonicSystemsPantryWidget>("HolonicSystems-Pantry");
