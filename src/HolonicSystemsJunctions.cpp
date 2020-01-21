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
	
	LooseSchmittTrigger triggers[2];
	

	HolonicSystemsJunctionsModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}


	~HolonicSystemsJunctionsModule() {
	}

	void onReset() override {
	}


	void process(const ProcessArgs &args) override {
		for (int i=0; i<2; i++){
			bool x = triggers[i].process(inputs[INPUT_1_CV+i].value);
			bool a = !triggers[i].isHigh();
			x = a && x;
			outputs[OUTPUT_1+i].value = (a) ? inputs[INPUT_1_A+i].value : inputs[INPUT_1_B+i].value;
			lights[LED_1_A+i].setSmoothBrightness(a ? 1 : 0, APP->engine->getSampleTime());
			lights[LED_1_B+i].setSmoothBrightness(a ? 0 : 1, APP->engine->getSampleTime());
		}
	}
};




struct HolonicSystemsJunctionsWidget : ModuleWidget {
	HolonicSystemsJunctionsWidget(HolonicSystemsJunctionsModule *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HolonicSystems-Junctions.svg")));
		
		//screws
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		
		//channels
		for (int i=0; i<2; i++) {

			//CV
			addInput(createInput<PJ301MPort>(Vec(10,60 + i*150), module, HolonicSystemsJunctionsModule::INPUT_1_CV+i));

			//IN A
			addInput(createInput<PJ301MPort>(Vec(10,95 + i*150), module, HolonicSystemsJunctionsModule::INPUT_1_A+i));
			addChild(createLight<MediumLight<RedLight>>(Vec(37, 95+8 + i*150), module, HolonicSystemsJunctionsModule::LED_1_A+i));
			//IN B
			addInput(createInput<PJ301MPort>(Vec(10,125 + i*150), module, HolonicSystemsJunctionsModule::INPUT_1_B+i));
			addChild(createLight<MediumLight<RedLight>>(Vec(37, 125+8 + i*150), module, HolonicSystemsJunctionsModule::LED_1_B+i));

			//Out
			addOutput(createOutput<PJ301MPort>(Vec(10,160 + i*150), module, HolonicSystemsJunctionsModule::OUTPUT_1+i));
		}
	}

};


Model *modelJunctions = createModel<HolonicSystemsJunctionsModule, HolonicSystemsJunctionsWidget>("HolonicSystems-Junctions");
