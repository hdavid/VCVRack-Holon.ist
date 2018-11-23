#include "HolonicSystems-Free.hpp"
#include "dsp/digital.hpp"

struct HolonicSystemsJunctionsModule : Module {

	enum ParamIds {
		NUM_PARAMS
	};

	enum InputIds {
		INPUT_1_A,
		INPUT_2_A,
		INPUT_1_B,
		INPUT_2_B,
		INPUT_1_CV,
		INPUT_2_CV,
		NUM_INPUTS
	};

	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		NUM_OUTPUTS
	};
	
	enum LightIds {
		LED_1_A,
		LED_2_A,
		LED_1_B,
		LED_2_B,
		NUM_LIGHTS
	};	
	
	HolonicSystemsJunctionsModule();
	~HolonicSystemsJunctionsModule();
	
	void step() override;
	
	void onReset() override {
	}
};


HolonicSystemsJunctionsModule::HolonicSystemsJunctionsModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	onReset();
}


HolonicSystemsJunctionsModule::~HolonicSystemsJunctionsModule() {
}


void HolonicSystemsJunctionsModule::step() {
	for (int i=0; i<2; i++){
		bool a = inputs[INPUT_1_CV+i].value < 1;
		outputs[OUTPUT_1+i].value = (a) ? inputs[INPUT_1_A+i].value : inputs[INPUT_1_B+i].value;
		lights[LED_1_A+i].setBrightnessSmooth(a ? 1 : 0);
		lights[LED_1_B+i].setBrightnessSmooth(a ? 0 : 1);
	}
}


struct HolonicSystemsJunctionsWidget : ModuleWidget {

	HolonicSystemsJunctionsWidget(HolonicSystemsJunctionsModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/HolonicSystems-Junctions.svg")));
		
		//screws
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		
		//channels
		for (int i=0; i<2; i++) {	

			//CV
			addInput(Port::create<PJ301MPort>(Vec(10,60 + i*150), Port::INPUT, module, HolonicSystemsJunctionsModule::INPUT_1_CV+i));

			//IN A
			addInput(Port::create<PJ301MPort>(Vec(10,95 + i*150), Port::INPUT, module, HolonicSystemsJunctionsModule::INPUT_1_A+i));
			addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(37, 95+8 + i*150), module, HolonicSystemsJunctionsModule::LED_1_A+i));
			//IN B
			addInput(Port::create<PJ301MPort>(Vec(10,125 + i*150), Port::INPUT, module, HolonicSystemsJunctionsModule::INPUT_1_B+i));
			addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(37, 125+8 + i*150), module, HolonicSystemsJunctionsModule::LED_1_B+i));

			//Out
			addOutput(Port::create<PJ301MPort>(Vec(10,160 + i*150), Port::OUTPUT, module, HolonicSystemsJunctionsModule::OUTPUT_1+i));
		}
	}

};


Model *modelHolonicSystemsJunctions = 
	Model::create<HolonicSystemsJunctionsModule, HolonicSystemsJunctionsWidget>(
		"Holonic Systems",
	 	"HolonicSystems-Junctions", 
		"Junctions",
		UTILITY_TAG
);
