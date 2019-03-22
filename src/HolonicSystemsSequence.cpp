#include "HolonicSystems-Free.hpp"
#include "dsp/digital.hpp"
#include <time.h>
#include <stdlib.h>

struct HolonicSystemsSequenceModule : Module {

	enum ParamIds {
		PARAM_ATT_1,
		PARAM_ATT_2,
		PARAM_ATT_3,
		PARAM_ATT_4,
		PARAM_ATT_5,
		PARAM_ATT_6,
		PARAM_ATT_7,
		PARAM_ATT_8,
		PARAM_TRIG_1,
		PARAM_TRIG_2,
		PARAM_TRIG_3,
		PARAM_TRIG_4,
		PARAM_TRIG_5,
		PARAM_TRIG_6,
		PARAM_TRIG_7,
		PARAM_TRIG_8,
		PARAM_OUTPUT_ATT,
		PARAM_START,
		PARAM_START_ATT,
		PARAM_LENGTH,
		PARAM_LENGTH_ATT,
		PARAM_SEQ,
		PARAM_SEQ_ATT,
		PARAM_MODE,
		PARAM_MODE_ATT,
		//PARAM_CONTINOUS,
		NUM_PARAMS
	};

	enum InputIds {
		IN_1,
		IN_2,
		IN_3,
		IN_4,
		IN_5,
		IN_6,
		IN_7,
		IN_8,
		IN_CLOCK,
		IN_RESET,
		IN_SEQ,
		IN_START,
		IN_LENGTH,
		IN_MODE,
		NUM_INPUTS
	};

	enum OutputIds {
		OUTPUT_CV,
		OUTPUT_TRIG_1,
		OUTPUT_TRIG_2,
		OUTPUT_CHANGE_TRIGGER,
		NUM_OUTPUTS
	};
	
	enum LightIds {
		LIGHT_1,
		LIGHT_2,
		LIGHT_3,
		LIGHT_4,
		LIGHT_5,
		LIGHT_6,
		LIGHT_7,
		LIGHT_8,
		NUM_LIGHTS
	};	
	
	int counter = 0;
	int start = 0;
	int length = 8; 
	bool reverse = false; // for ping pong
	
	LooseSchmittTrigger fwdTrigger;
	LooseSchmittTrigger bckTrigger;
	LooseSchmittTrigger rstTrigger;
	
	dsp::PulseGenerator outputTrigger1;
	dsp::PulseGenerator outputTrigger2;


	HolonicSystemsSequenceModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for(int i=0;i<8;i++){
			params[PARAM_ATT_1 + i].config( 0.f, 1.0f, 1.0f, "Attenutor 1");
			params[PARAM_TRIG_1 + i].config(0.f, 2.f, (i%3==0)? 0.f : ((i%2==0)?1.f:2.f), "Trigger 1");
		}
		params[PARAM_OUTPUT_ATT].config(0.f, 1.f, 1.f, "Output Att");
		params[PARAM_START].config(0.f, 7.f, 0.f, "Start");
		params[PARAM_START_ATT].config(0.f, 1.f, 1.f, "Start CV Att");
		params[PARAM_LENGTH].config(0.f, 7.f, 7.f, "Length");
		params[PARAM_LENGTH_ATT].config(0.f, 1.f, 1.f, "Length CV Att");
		params[PARAM_SEQ_ATT].config(0.f, 1.f, 1.f, "Address CV Att");
		params[PARAM_MODE].config(0.f, 3.f, 0.f, "Mode");
		params[PARAM_MODE_ATT].config( 0.f, 1.f, 1.f, "Mode CV Att");

		onReset();
	}


	~HolonicSystemsSequenceModule() {
	}

		
	void onReset() override {
		reverse = false;
		counter = 0;
		srand(time(NULL));
	}


	void process(const ProcessArgs &args) override {
		
		// Triggers Inputs
		bool clock = fwdTrigger.process(inputs[IN_CLOCK].value);
		bool reset = rstTrigger.process(inputs[IN_RESET].value);
		
		// CV Inputs
		float in_start = inputs[IN_START].value/10.0;
		if (in_start<0) {
			in_start = 0;
		}
		if (in_start>1) {
			in_start = 1;
		}
		
		float in_length = inputs[IN_LENGTH].value/10.0;
		if (in_length<0) {
			in_length = 0;
		}
		if (in_length>1) {
			in_length = 1;
		}
		
		float in_seq = inputs[IN_SEQ].value/10.0;
		if (in_seq<0) {
			in_seq = 0;
		}
		if (in_seq>1) {
			in_seq = 1;
		}
		
		int in_mode = (int)(inputs[IN_MODE].value/10.0*3);
		if (in_mode<0) {
			in_mode = 0;
		}
		if (in_mode>3) {
			in_mode = 3;
		}

		// Parameters
		start = (
				( (int)(params[PARAM_START].value) )
				+ ( (int)(inputs[IN_START].active ? (in_start*params[PARAM_START_ATT].value * 8) : 0) ) 
				+ 8
			)%8;
		length = (
				 ( (int)(params[PARAM_LENGTH].value) )
				+ ( (int)(inputs[IN_LENGTH].active ? (in_length*params[PARAM_LENGTH_ATT].value * 8) : 0) )
				+ 8
			)%8 + 1;
		
		float seq = in_seq * params[PARAM_SEQ].value;
		int mode = (((int)params[PARAM_MODE].value) + in_mode)%4;
		bool forward = mode == 0 || mode == 2;
		bool backward = mode == 1;
		bool pingpong = mode == 2;
		bool randomMode = mode == 3;
		
		if (!pingpong) {
			reverse = false;
		}
		int oldCounter = counter;

		
		// Clocking	
		if (reset) {
			counter = start;
		} else if (inputs[IN_SEQ].active) {
			// Address Mode
			if (!inputs[IN_CLOCK].active || clock) {
				counter = ( (int)(start +  seq * length) )%8;
			}
		} else if (clock){
			if (randomMode){
				counter = ( (int)(start + ((float)rand())/RAND_MAX  * length) )%8;
			} else if ((forward && !(reverse && pingpong)) 
				|| (backward && (reverse && pingpong))
			) {
				counter = (start + ((counter - start + 1)%(length)) )%8;
			} else if ((forward && (reverse && pingpong))
				|| (backward && !(reverse && pingpong) )
				) {
				//check above zero and start
				counter = (start + ((counter - start - 1 + length)%(length))+8)%8;
			}
			// ping-pong
			if (pingpong) {
				if (counter == start) {
					if (forward) {
						reverse = false;
					} else if (backward) {
						reverse = true;
					}
				} else if (counter%8 == (start + length - 1)%8) {
					if (forward) {
						reverse = true;
					} else if (backward) {
						reverse = false;
					}
				}
			}
		}
		
		// Process CV Output
		for (int i=0;i<8;i++) {
			lights[LIGHT_1+i].setBrightness(counter == i ? 1 : 0);
			if (counter == i) {
				//output cv
				if ((inputs[IN_CLOCK].active && clock) || counter != oldCounter ){//|| params[PARAM_CONTINOUS].value == 1) {
					outputs[OUTPUT_CV].value = params[PARAM_OUTPUT_ATT].value * params[PARAM_ATT_1+i].value * (inputs[IN_1+i].active ? inputs[IN_1+i].value : 10.0);
				}
				//trigger
				if ((inputs[IN_CLOCK].active && clock) || counter != oldCounter){
					if (params[PARAM_TRIG_1+i].value == 1){
						outputTrigger1.trigger(1e-3);
					} else if (params[PARAM_TRIG_1+i].value == 2){
						outputTrigger2.trigger(1e-3);
					}
				}
			}
		}
		
		// Process Triggers
		float deltaTime = APP->engine->getSampleTime();
		outputs[OUTPUT_TRIG_1].value = outputTrigger1.process(deltaTime) ? 10.0 : 0.0;
		outputs[OUTPUT_TRIG_2].value = outputTrigger2.process(deltaTime) ? 10.0 : 0.0;

	}

};


struct HolonicSystemsSequenceWidget : ModuleWidget {
	HolonicSystemsSequenceWidget(HolonicSystemsSequenceModule *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HolonicSystems-Sequence.svg")));

		// Screws
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));		
		
		// Trigger Inputs
		addInput(createInput<PJ301MPort>(Vec(33, 34), module, HolonicSystemsSequenceModule::IN_CLOCK));
		addInput(createInput<PJ301MPort>(Vec(63, 34), module, HolonicSystemsSequenceModule::IN_RESET));
		
		// Address Input
		addInput(createInput<PJ301MPort>(Vec(103, 34), module, HolonicSystemsSequenceModule::IN_SEQ));
		addParam(createParam<Trimpot>(Vec(133, 34+3), module, HolonicSystemsSequenceModule::PARAM_SEQ_ATT));
	
		// Channels
		int start = 66;
		for (int i=0; i<8 ; i++) {
			addChild(createLight<MediumLight<RedLight>>(Vec(10+5		, start + i * 36 + 8), module, HolonicSystemsSequenceModule::LIGHT_1 + i));
			addInput(createInput<PJ301MPort>(Vec(10+20, start + i * 36), module, HolonicSystemsSequenceModule::IN_1 + i));
			addParam(createParam<RoundSmallBlackKnob>(Vec(10+50, start + i * 36), module, HolonicSystemsSequenceModule::PARAM_ATT_1 + i));
			addParam(createParam<CKSSThree>(Vec(10+85, start + i * 36), module, HolonicSystemsSequenceModule::PARAM_TRIG_1+i));
		}
		
		
		// Mode
		addInput(createInput<PJ301MPort>(Vec(113, 66+18*1), module, HolonicSystemsSequenceModule::IN_MODE));
		addParam(createParam<Trimpot>(Vec(136, 66+18*1+15), module, HolonicSystemsSequenceModule::PARAM_MODE_ATT));
		rack::RoundSmallBlackKnob* param_mode = createParam<RoundSmallBlackKnob>(Vec(153, 66+18*1), module, HolonicSystemsSequenceModule::PARAM_MODE);
		param_mode->snap = true;
		addParam(param_mode);
		
		
		// Start
		addInput(createInput<PJ301MPort>(Vec(113, 66+18*4), module, HolonicSystemsSequenceModule::IN_START));
		addParam(createParam<Trimpot>(Vec(136, 66+18*4+15), module, HolonicSystemsSequenceModule::PARAM_START_ATT));
		rack::RoundSmallBlackKnob* param_start = createParam<RoundSmallBlackKnob>(Vec(153, 66+18*4), module, HolonicSystemsSequenceModule::PARAM_START);
		param_start->snap = true;
		addParam(param_start);
		
		// Length
		addInput(createInput<PJ301MPort>(Vec(113, 66+18*7), module, HolonicSystemsSequenceModule::IN_LENGTH));
		addParam(createParam<Trimpot>(Vec(136, 66+18*7+15), module, HolonicSystemsSequenceModule::PARAM_LENGTH_ATT));
		rack::RoundSmallBlackKnob* param_length = createParam<RoundSmallBlackKnob>(			Vec(153, 66+18*7), module, HolonicSystemsSequenceModule::PARAM_LENGTH);
		param_length->snap = true;
		addParam(param_length);
		
		//addParam(ParamWidget::create<CKSS>(Vec(113, 66+18*8+15), module, HolonicSystemsSequenceModule::PARAM_CONTINOUS, 0, 1.0, 1.0));
		
		// Master
		addParam(createParam<RoundSmallBlackKnob>(Vec(123, 66+18*11), module, HolonicSystemsSequenceModule::PARAM_OUTPUT_ATT));
		addOutput(createOutput<PJ301MPort>(Vec(153, 66+18*11), module, HolonicSystemsSequenceModule::OUTPUT_CV));
		
		// Triggers
		addOutput(createOutput<PJ301MPort>(Vec(123, 66+18*14), module, HolonicSystemsSequenceModule::OUTPUT_TRIG_1));
		addOutput(createOutput<PJ301MPort>(Vec(153, 66+18*14), module, HolonicSystemsSequenceModule::OUTPUT_TRIG_2));
	}
};

Model *modelHolonicSystemsSequence = createModel<HolonicSystemsSequenceModule, HolonicSystemsSequenceWidget>("HolonicSystems-Sequence A-155");

