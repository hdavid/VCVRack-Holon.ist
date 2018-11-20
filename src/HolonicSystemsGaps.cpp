#include "HolonicSystems-Free.hpp"
#include "dsp/digital.hpp"
#include <vector>
#include <time.h>
#include <stdlib.h>



struct HolonicSystemsGapsModule : Module {

	enum ParamIds {
		MODE_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		INPUT_CLOCK,
		INPUT_RESET,
		NUM_INPUTS
	};

	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		OUTPUT_4,
		OUTPUT_5,
		OUTPUT_6,
		OUTPUT_7,
		OUTPUT_8,
		NUM_OUTPUTS
	};
	
	enum LightIds {
		LED_1,
		LED_2,
		LED_3,
		LED_4,
		LED_5,
		LED_6,
		LED_7,
		LED_8,
		NUM_LIGHTS
	};	
	
	HolonicSystemsGapsModule();
	~HolonicSystemsGapsModule();
	
	
	void onReset() override {
		srand(time(NULL));
	}
	
	void step() override;
	SchmittTrigger clockTrigger;
	SchmittTrigger resetTrigger;
	
	 int counter = 0;
	 std::vector<float> track;  
	 PulseGenerator pulses[8];
	 int divisions[7][8] = {
		 						{2,3,4,5, 6,7,8,9},
								{2,4,6,8, 10,12,14,16},
								{3,5,7,9, 11,13,15,17},
								{2,3,5,7, 11,13,17,19},
								{2,4,8,16, 32,64,128,256},
								
								{2,3,4,5, 6,7,8,9}, // random
								{0,1,2,3, 4,5,6,7} // seq
							};
};


HolonicSystemsGapsModule::HolonicSystemsGapsModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	onReset();
}


HolonicSystemsGapsModule::~HolonicSystemsGapsModule() {
}


void HolonicSystemsGapsModule::step() {
	
	bool clock = clockTrigger.process(inputs[INPUT_CLOCK].value);
	bool reset = resetTrigger.process(inputs[INPUT_RESET].value);
	
	if (reset) {
		counter = 0;
	}
	
	float deltaTime = engineGetSampleTime();
	
	if (clock) {
		
		for (int i=0;i<8;i++){
			if ((int)params[MODE_PARAM].value <= 4){
				if (0 == counter % divisions[(int)params[MODE_PARAM].value][i]){
					pulses[i].trigger(1e-3);
				}
				outputs[OUTPUT_1+i].value = pulses[i].process(deltaTime) ? 10.0 : 0.0;
				//outputs[OUTPUT_1+i].value = (0 == counter % divisions[(int)params[MODE_PARAM].value][i]) ? 10.0 : 0.0;
				
			} else if ((int)params[MODE_PARAM].value == 5){
				if (0 == counter % 2){
					int r = rand();  
					outputs[OUTPUT_1+i].value = (r < RAND_MAX / divisions[(int)params[MODE_PARAM].value][i]) ? 10.0 : 0.0;
				} else {
					outputs[OUTPUT_1+i].value = 0;
				}
				
			} else if ((int)params[MODE_PARAM].value == 6){
				outputs[OUTPUT_1+i].value = (0 == (counter + 8 - i) % 8) ? 10.0 : 0.0;
				
			}
			lights[LED_1+i].setBrightness(outputs[OUTPUT_1+i].value);
		}
		counter++;
	} else {
		for (int i=0;i<8;i++){
			if ((int)params[MODE_PARAM].value <= 4){
				outputs[OUTPUT_1+i].value = pulses[i].process(deltaTime) ? 10.0 : 0.0;
			}
		}
	}
}


struct HolonicSystemsGapsWidget : ModuleWidget {

	HolonicSystemsGapsWidget(HolonicSystemsGapsModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/HolonicSystems-Gaps.svg")));
		
		//screws
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		//IN
		addInput(Port::create<PJ301MPort>(Vec(5, 50), Port::INPUT, module, HolonicSystemsGapsModule::INPUT_CLOCK));
		addInput(Port::create<PJ301MPort>(Vec(30, 50), Port::INPUT, module, HolonicSystemsGapsModule::INPUT_RESET));
		
		addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(18, 90), module, HolonicSystemsGapsModule::MODE_PARAM, 0, 6, 0));
		
		
		for (int i=0;i<8;i++){
			addOutput(Port::create<PJ301MPort>(Vec(10,120 + 30*i), Port::OUTPUT, module, HolonicSystemsGapsModule::OUTPUT_1+i));
			addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(40,120 + 8+ 30*i), module, HolonicSystemsGapsModule::LED_1+i));
		}
		
	}

};


Model *modelHolonicSystemsGaps = 
	Model::create<HolonicSystemsGapsModule, HolonicSystemsGapsWidget>(
		"Holonic Systems",
	 	"HolonicSystems-Gaps", 
		"Gaps",
		CLOCK_TAG
);
