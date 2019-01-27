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
		PARAM_ATT,
		PARAM_START,
		PARAM_START_ATT,
		PARAM_LENGTH,
		PARAM_LENGTH_ATT,
		PARAM_SEQ,
		PARAM_SEQ_ATT,
		PARAM_MODE,
		PARAM_MODE_ATT,
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
	
	PulseGenerator outputTrigger1;
	PulseGenerator outputTrigger2;
	
	HolonicSystemsSequenceModule();
	~HolonicSystemsSequenceModule();
	
	void step() override;
	
	void onReset() override {
		reverse = false;
		counter = 0;
		srand(time(NULL));
	}

};


HolonicSystemsSequenceModule::HolonicSystemsSequenceModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	onReset();
}


HolonicSystemsSequenceModule::~HolonicSystemsSequenceModule() {
}


void HolonicSystemsSequenceModule::step() {
	
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
			outputs[OUTPUT_CV].value = params[PARAM_ATT].value * params[PARAM_ATT_1+i].value * (inputs[IN_1+i].active ? inputs[IN_1+i].value : 10.0);
			//trigger
			if (counter != oldCounter){
				if (params[PARAM_TRIG_1+i].value == 1){
					outputTrigger1.trigger(1e-3);
				} else if (params[PARAM_TRIG_1+i].value == 2){
					outputTrigger2.trigger(1e-3);
				}
			}
		}
	}
	
	// Process Triggers
	float deltaTime = engineGetSampleTime();
	outputs[OUTPUT_TRIG_1].value = outputTrigger1.process(deltaTime) ? 10.0 : 0.0;
	outputs[OUTPUT_TRIG_2].value = outputTrigger2.process(deltaTime) ? 10.0 : 0.0;

}





struct HolonicSystemsSequenceWidget : ModuleWidget {
	HolonicSystemsSequenceWidget(HolonicSystemsSequenceModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/HolonicSystems-Sequence.svg")));
		
		// Screws
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));		
		
		// Trigger Inputs
		addInput(Port::create<PJ301MPort>(							Vec(33, 34), Port::INPUT, module, HolonicSystemsSequenceModule::IN_CLOCK));
		addInput(Port::create<PJ301MPort>(							Vec(63, 34), Port::INPUT, module, HolonicSystemsSequenceModule::IN_RESET));
		
		// Address Input
		addInput(Port::create<PJ301MPort>(							Vec(103, 34), Port::INPUT, module, HolonicSystemsSequenceModule::IN_SEQ));
		addParam(ParamWidget::create<Trimpot>(			Vec(133, 34+3), module, HolonicSystemsSequenceModule::PARAM_SEQ, 0, 1.0, 1.0));
	
		// Channels
		int start = 66;
		for (int i=0; i<8 ; i++) {
			addChild(ModuleLightWidget::create<MediumLight<RedLight>>(	Vec(10+5		, start + i * 36 + 8), module, HolonicSystemsSequenceModule::LIGHT_1 + i));
			addInput(Port::create<PJ301MPort>(							Vec(10+20, start + i * 36), Port::INPUT, module, HolonicSystemsSequenceModule::IN_1 + i));
			addParam(ParamWidget::create<RoundSmallBlackKnob>(			Vec(10+50, start + i * 36), module, HolonicSystemsSequenceModule::PARAM_ATT_1 + i, 0, 1.0, 1.0));
			addParam(ParamWidget::create<CKSSThree>(Vec(10+85, start + i * 36), module, HolonicSystemsSequenceModule::PARAM_TRIG_1+i, 0, 2.0, (i%3==0)? 0 : ((i%2==0)?1:2)) );
		}
		
		
		// Mode
		addInput(Port::create<PJ301MPort>(							Vec(113, 66+18*1), Port::INPUT, module, HolonicSystemsSequenceModule::IN_MODE));
		addParam(ParamWidget::create<Trimpot>(Vec(136, 66+18*1+15), module, HolonicSystemsSequenceModule::PARAM_MODE_ATT, 0, 1.0, 1.0));
		rack::RoundSmallBlackKnob* param_mode = ParamWidget::create<RoundSmallBlackKnob>(			Vec(153, 66+18*1), module, HolonicSystemsSequenceModule::PARAM_MODE, 0, 3.0, 0.0);
		param_mode->snap = true;
		addParam(param_mode);
		
		
		// Start
		addInput(Port::create<PJ301MPort>(							Vec(113, 66+18*4), Port::INPUT, module, HolonicSystemsSequenceModule::IN_START));
		addParam(ParamWidget::create<Trimpot>(Vec(136, 66+18*4+15), module, HolonicSystemsSequenceModule::PARAM_START_ATT, 0, 1.0, 1.0));
		rack::RoundSmallBlackKnob* param_start = ParamWidget::create<RoundSmallBlackKnob>(			Vec(153, 66+18*4), module, HolonicSystemsSequenceModule::PARAM_START, 0, 7.0, 0.0);
		param_start->snap = true;
		addParam(param_start);
		
		// Length
		addInput(Port::create<PJ301MPort>(							Vec(113, 66+18*7), Port::INPUT, module, HolonicSystemsSequenceModule::IN_LENGTH));
		addParam(ParamWidget::create<Trimpot>(Vec(136, 66+18*7+15), module, HolonicSystemsSequenceModule::PARAM_LENGTH_ATT, 0, 1.0, 1.0));
		rack::RoundSmallBlackKnob* param_length = ParamWidget::create<RoundSmallBlackKnob>(			Vec(153, 66+18*7), module, HolonicSystemsSequenceModule::PARAM_LENGTH, 0, 7.0, 7.0);
		param_length->snap = true;
		addParam(param_length);
		
		
		// Master
		addParam(ParamWidget::create<RoundSmallBlackKnob>(			Vec(123, 66+18*11), module, HolonicSystemsSequenceModule::PARAM_ATT, 0, 1.0, 1.0));
		addOutput(Port::create<PJ301MPort>(							Vec(153, 66+18*11), Port::OUTPUT, module, HolonicSystemsSequenceModule::OUTPUT_CV));
		
		// Triggers
		addOutput(Port::create<PJ301MPort>(							Vec(123, 66+18*14), Port::OUTPUT, module, HolonicSystemsSequenceModule::OUTPUT_TRIG_1));
		addOutput(Port::create<PJ301MPort>(							Vec(153, 66+18*14), Port::OUTPUT, module, HolonicSystemsSequenceModule::OUTPUT_TRIG_2));
	}
};

Model *modelHolonicSystemsSequence = 
	Model::create<HolonicSystemsSequenceModule, HolonicSystemsSequenceWidget>(
		"Holonic Systems",
	 	"HolonicSystems-Sequence A-155", 
		"Sequence",
		SEQUENCER_TAG,
		ATTENUATOR_TAG
);
