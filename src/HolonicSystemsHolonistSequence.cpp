#include "HolonicSystems-Free.hpp"
#include "dsp/digital.hpp"
//#include <string>
//#include <sys/time.h>

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
		PARAM_LENGTH,
		PARAM_SEQ,
		PARAM_MODE,
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
		IN_FWD,
		IN_BCK,
		IN_RESET,
		IN_SEQ,
		IN_START,
		IN_LENGTH,
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
	bool reverse = false;
	
	LooseSchmittTrigger fwdTrigger;
	LooseSchmittTrigger bckTrigger;
	LooseSchmittTrigger rstTrigger;
	
	PulseGenerator trigger1;
	PulseGenerator trigger2;
	PulseGenerator changeTrigger;
	
	HolonicSystemsSequenceModule();
	~HolonicSystemsSequenceModule();
	
	void step() override;
	
	void onReset() override {
	}

};


HolonicSystemsSequenceModule::HolonicSystemsSequenceModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	onReset();
}


HolonicSystemsSequenceModule::~HolonicSystemsSequenceModule() {
}


void HolonicSystemsSequenceModule::step() {
	
	// Triggers
	bool backward = bckTrigger.process(inputs[IN_BCK].value);
	bool forward = fwdTrigger.process(inputs[IN_FWD].value);
	bool reset = rstTrigger.process(inputs[IN_RESET].value);
	int c = counter;
	
	// CV Inputs
	float in_start = inputs[IN_START].value/10.0;
	float in_length = inputs[IN_LENGTH].value/10.0;
	float in_seq = inputs[IN_SEQ].value/10.0;
	if (in_start<0){
		in_start = 0;
	}
	if (in_start>1){
		in_start = 1;
	}
	if (in_length<0){
		in_length = 0;
	}
	if (in_length>1){
		in_length = 1;
	}
	if (in_seq<0){
		in_seq = 0;
	}
	if (in_seq>1){
		in_seq = 1;
	}
	start = (int)(params[PARAM_START].value * (inputs[IN_START].active ? in_start : 1));
	length = (int)(params[PARAM_LENGTH].value * (inputs[IN_LENGTH].active ? in_length : 1)) + 1;
	float seq = in_seq * params[PARAM_SEQ].value;
	
	/*
	- wrap around when backward
	- dont go below 0 when backward
	- ping pong end point past 8 ?
	*/

	bool pingpong = params[PARAM_MODE].value == 0;

	
	
	//clocking	
	if (reset) {
		counter = start;
	} else if (inputs[IN_SEQ].active) {
		// address mode
		if (!inputs[IN_FWD].active || forward) {
			counter = ( (int)(start + seq * length) )%8;
		}
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
	
	
	//ping-pong
	if ((forward || backward) && pingpong){
		if (counter == start){
			if (forward){
				reverse = false;
			} else {
				reverse = true;
			}
		} else if (counter%8 == (start + length - 1)%8) {
			if (forward){
				reverse = true;
			} else {
				reverse = false;
			}
		}
	}

	
	if (counter != c) {
		changeTrigger.trigger(1e-3);
	}
	
	
	//process output
	for (int i=0;i<8;i++){
		lights[LIGHT_1+i].setBrightness(counter == i ? 1 : 0);
		if (counter == i){
			//output cv
			outputs[OUTPUT_CV].value = params[PARAM_ATT].value * params[PARAM_ATT_1+i].value * (inputs[IN_1+i].active ? inputs[IN_1+i].value : 10.0);
			//trigger
			if (counter != c){
				if (params[PARAM_TRIG_1+i].value == 1){
					trigger1.trigger(1e-3);
				} else if (params[PARAM_TRIG_1+i].value == 2){
					trigger2.trigger(1e-3);
				}
			}
		}
	}
	
	//process triggers
	float deltaTime = engineGetSampleTime();
	outputs[OUTPUT_TRIG_1].value = trigger1.process(deltaTime) ? 10.0 : 0.0;
	outputs[OUTPUT_TRIG_2].value = trigger2.process(deltaTime) ? 10.0 : 0.0;
	outputs[OUTPUT_CHANGE_TRIGGER].value = changeTrigger.process(deltaTime) ? 10.0 : 0.0;

}





struct HolonicSystemsSequenceWidget : ModuleWidget {
	HolonicSystemsSequenceWidget(HolonicSystemsSequenceModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/HolonicSystems-Sequence.svg")));
		
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));		
	
		addInput(Port::create<PJ301MPort>(							Vec(03, 34), Port::INPUT, module, HolonicSystemsSequenceModule::IN_BCK));
		addInput(Port::create<PJ301MPort>(							Vec(33, 34), Port::INPUT, module, HolonicSystemsSequenceModule::IN_RESET));
		addInput(Port::create<PJ301MPort>(							Vec(63, 34), Port::INPUT, module, HolonicSystemsSequenceModule::IN_FWD));
		
		addInput(Port::create<PJ301MPort>(							Vec(103, 34), Port::INPUT, module, HolonicSystemsSequenceModule::IN_SEQ));
		addParam(ParamWidget::create<RoundSmallBlackKnob>(			Vec(133, 34), module, HolonicSystemsSequenceModule::PARAM_SEQ, 0, 1.0, 1.0));
	
		//channels
		int start = 66;
		for (int i=0; i<8 ; i++) {
			addChild(ModuleLightWidget::create<MediumLight<RedLight>>(	Vec(10+5		, start + i * 36 + 8), module, HolonicSystemsSequenceModule::LIGHT_1 + i));
			addInput(Port::create<PJ301MPort>(							Vec(10+20, start + i * 36), Port::INPUT, module, HolonicSystemsSequenceModule::IN_1 + i));
			addParam(ParamWidget::create<RoundSmallBlackKnob>(			Vec(10+50, start + i * 36), module, HolonicSystemsSequenceModule::PARAM_ATT_1 + i, 0, 1.0, 1.0));
			addParam(ParamWidget::create<CKSSThree>(Vec(10+85, start + i * 36), module, HolonicSystemsSequenceModule::PARAM_TRIG_1+i, 0, 2.0, (i%3==0)? 0 : ((i%2==0)?1:2)) );
		}
		
		//start
		addInput(Port::create<PJ301MPort>(							Vec(123, 66+18*2), Port::INPUT, module, HolonicSystemsSequenceModule::IN_START));
		rack::RoundSmallBlackKnob* param_start = ParamWidget::create<RoundSmallBlackKnob>(			Vec(153, 66+18*2), module, HolonicSystemsSequenceModule::PARAM_START, 0, 8.0, 0.0);
		param_start->snap = true;
		addParam(param_start);
	
		
		//length
		addInput(Port::create<PJ301MPort>(							Vec(123, 66+18*5), Port::INPUT, module, HolonicSystemsSequenceModule::IN_LENGTH));
		rack::RoundSmallBlackKnob* param_length = ParamWidget::create<RoundSmallBlackKnob>(			Vec(153, 66+18*5), module, HolonicSystemsSequenceModule::PARAM_LENGTH, 0, 7.0, 7.0);
		param_length->snap = true;
		addParam(param_length);
		
		//Mode
		addParam(ParamWidget::create<CKSSThree>(Vec(123, 66+18*7), module, HolonicSystemsSequenceModule::PARAM_MODE, 0, 1.0, 1.0));
		
		//master
		addParam(ParamWidget::create<RoundSmallBlackKnob>(			Vec(123, 66+18*11), module, HolonicSystemsSequenceModule::PARAM_ATT, 0, 1.0, 1.0));
		addOutput(Port::create<PJ301MPort>(							Vec(153, 66+18*11), Port::OUTPUT, module, HolonicSystemsSequenceModule::OUTPUT_CV));
		
		//triggers
		addOutput(Port::create<PJ301MPort>(							Vec(123, 66+18*14), Port::OUTPUT, module, HolonicSystemsSequenceModule::OUTPUT_TRIG_1));
		addOutput(Port::create<PJ301MPort>(							Vec(153, 66+18*14), Port::OUTPUT, module, HolonicSystemsSequenceModule::OUTPUT_TRIG_2));
		addOutput(Port::create<PJ301MPort>(							Vec(123+15, 66+18*14+25), Port::OUTPUT, module, HolonicSystemsSequenceModule::OUTPUT_CHANGE_TRIGGER));
		
	}
};

Model *modelHolonicSystemsSequence = 
	Model::create<HolonicSystemsSequenceModule, HolonicSystemsSequenceWidget>(
		"Holonic Systems",
	 	"HolonicSystems-Sequence", 
		"Sequence",
		CONTROLLER_TAG, 
		EXTERNAL_TAG
);
