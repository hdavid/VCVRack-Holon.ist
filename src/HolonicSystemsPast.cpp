#include "HolonicSystems-Free.hpp"
#include "dsp/digital.hpp"
#include <vector>

struct HolonicSystemsPastModule : Module {

	enum ParamIds {
		PARAM_LENGTH_1,
		PARAM_LENGTH_2,
		PARAM_LENGTH_3,
		PARAM_LENGTH_4,
		NUM_PARAMS
	};

	enum InputIds {
		INPUT_CLOCK,
		INPUT_RESET,
		INPUT_RECORD_1,
		INPUT_RECORD_2,
		INPUT_RECORD_3,
		INPUT_RECORD_4,
		INPUT_CV_1,
		INPUT_CV_2,
		INPUT_CV_3,
		INPUT_CV_4,
		INPUT_GATE_1,
		INPUT_GATE_2,
		INPUT_GATE_3,
		INPUT_GATE_4,
		NUM_INPUTS
	};

	enum OutputIds {
		OUTPUT_CV_1,
		OUTPUT_CV_2,
		OUTPUT_CV_3,
		OUTPUT_CV_4,
		OUTPUT_GATE_1,
		OUTPUT_GATE_2,
		OUTPUT_GATE_3,
		OUTPUT_GATE_4,
		NUM_OUTPUTS
	};
	
	enum LightIds {
		NUM_LIGHTS
	};	
	
	HolonicSystemsPastModule();
	~HolonicSystemsPastModule();
	
	void onReset() override {
		printf("HolonicSystemsPastModule::onReset\n");
		for (int i=0; i<4; i++) {
			counters[i] = 0;
			recordings[i] = false;
			for (int j=0;j<64;j++){
				cvs[i][j] =0;
				gates[i][j]=0;
			}
			//cvs[i].resize(0);
			//gates[i].resize(0);
		}
	}
	
	void step() override;
	
	SchmittTrigger clockTrigger;
	SchmittTrigger resetTrigger;
	int counters[4] = {0,0,0,0};
	bool recordings[4] = {false,false,false,false};
	std::vector<float> cvs[4] = {std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64)};
	std::vector<float> gates[4]= {std::vector<float>(64),std::vector<float>(64),std::vector<float>(64),std::vector<float>(64)};  
	 
};


HolonicSystemsPastModule::HolonicSystemsPastModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	onReset();
}


HolonicSystemsPastModule::~HolonicSystemsPastModule() {
}


void HolonicSystemsPastModule::step() {
		
	bool clock = clockTrigger.process(inputs[INPUT_CLOCK].value);
	bool reset = resetTrigger.process(inputs[INPUT_RESET].value);
	
	for (int i = 0; i<4;i++){
		
		if (reset){
			counters[i] = 0;
		}
		
		
		if (clock) {
			
			counters[i] = counters[i] % (int)params[PARAM_LENGTH_1+i].value;
			
			bool r = inputs[INPUT_RECORD_1+i].value > 1;
			
			// begin recording
			//if (!recordings[i] && r) {
				recordings[i] = true;
				//cvs[i].resize(0);
				//gates[i].resize(0);
				//counters[i] = 0;
			//}
			
			//stop recording
			// if (!r && recordings[i]){
// 				if (counters[i]%(int)params[PARAM_LENGTH_1+i].value == 0) {
// 					recordings[i] = false;
// 					counters[i] = 0;
// 				}
// 			}

			//recording
			//if (recordings[i]) {
			//	cvs[i].push_back(inputs[INPUT_CV_1+i].value);
			//	gates[i].push_back(inputs[INPUT_GATE_1+i].value);
			//}
			if (r){
				cvs[i][counters[i]]=(inputs[INPUT_CV_1+i].value);
				gates[i][counters[i]]=(inputs[INPUT_GATE_1+i].value);
			}
			
			if (recordings[i]){
 				if (counters[i]%(int)params[PARAM_LENGTH_1+i].value == 0) {
 					recordings[i] = false;
 					counters[i] = 0;
 				}
 			}
	
			//output
			//if (cvs[i].size()>0){
				outputs[OUTPUT_CV_1+i].value = cvs[i].at(counters[i]);
				outputs[OUTPUT_GATE_1+i].value = gates[i].at(counters[i]);
			//}
			
			//step
			counters[i]++;
			//if (!recordings[i] && counters[i] >= cvs[i].size()){
			//	counters[i] = 0;
			//}
		}
	}
}


struct HolonicSystemsPastWidget : ModuleWidget {

	HolonicSystemsPastWidget(HolonicSystemsPastModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/HolonicSystems-Past.svg")));
		
		//screws
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
 
		//IN
		addInput(Port::create<PJ301MPort>(Vec(10, 50), Port::INPUT, module, HolonicSystemsPastModule::INPUT_CLOCK));
		addInput(Port::create<PJ301MPort>(Vec(40, 50), Port::INPUT, module, HolonicSystemsPastModule::INPUT_RESET));
		
		for (int i=0;i<4;i++){
			//channel
			addInput(Port::create<PJ301MPort>(Vec(10, 80 + i*70), Port::INPUT, module, HolonicSystemsPastModule::INPUT_CV_1+i));
			addInput(Port::create<PJ301MPort>(Vec(10, 110 + i*70), Port::INPUT, module, HolonicSystemsPastModule::INPUT_GATE_1+i));
			
			addInput(Port::create<PJ301MPort>(Vec(50, 80 + i*70), Port::INPUT, module, HolonicSystemsPastModule::INPUT_RECORD_1+i));
			addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(50, 110 + i*70), module, HolonicSystemsPastModule::PARAM_LENGTH_1+i, 1, 32, 16));
			
			addOutput(Port::create<PJ301MPort>(Vec(90, 80 + i*70), Port::OUTPUT, module, HolonicSystemsPastModule::OUTPUT_CV_1+i));
			addOutput(Port::create<PJ301MPort>(Vec(90,110 + i*70), Port::OUTPUT, module, HolonicSystemsPastModule::OUTPUT_GATE_1));
		}
	}

};


Model *modelHolonicSystemsPast = 
	Model::create<HolonicSystemsPastModule, HolonicSystemsPastWidget>(
		"Holonic Systems",
	 	"HolonicSystems-Past", 
		"Past",
		CLOCK_TAG
);
