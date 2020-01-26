#include "HolonicSystems-Free.hpp"
#include "dsp/digital.hpp"

struct HolonicSystemsJunctionsModule : Module {

	enum ParamIds {
		TRIG_MODE_PARAM,
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
	bool isA[2] =  {true, true};
	

	HolonicSystemsJunctionsModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	~HolonicSystemsJunctionsModule() {
	}

	void onReset() override {
	}

	void process(const ProcessArgs &args) override {
		for (int i=0; i<2; i++){
			bool trigger = triggers[i].process(inputs[INPUT_1_CV+i].value);
			if (params[TRIG_MODE_PARAM].value == 0){
				isA[i] = !triggers[i].isHigh();
			} else {
				if (trigger) {
					isA[i] = !isA[i];
				}
			}
			outputs[OUTPUT_1+i].value = isA[i] ? inputs[INPUT_1_A+i].value : inputs[INPUT_1_B+i].value;
			lights[LED_1_A+i].setSmoothBrightness(isA[i] ? 1 : 0, APP->engine->getSampleTime());
			lights[LED_1_B+i].setSmoothBrightness(isA[i] ? 0 : 1, APP->engine->getSampleTime());
		}
	}
	
};


struct HolonicJunctionTrigGateLabel : Widget {
	int fontSize;
	HolonicSystemsJunctionsModule *module = nullptr;

	
	HolonicJunctionTrigGateLabel(int _fontSize,HolonicSystemsJunctionsModule *_module) {
		fontSize = _fontSize;
		box.size.y = BND_WIDGET_HEIGHT;
		module = _module;
	}

	void draw(const DrawArgs &args) override {
		nvgFillColor(args.vg, nvgRGB(0, 0, 0));
		nvgFontSize(args.vg, fontSize);
		if (module){
			if (module->params[HolonicSystemsJunctionsModule::TRIG_MODE_PARAM].value==0){
				nvgText(args.vg, box.pos.x, box.pos.y, "gate", NULL);
			} else {
				nvgText(args.vg, box.pos.x, box.pos.y, "trig", NULL);
			}
		} else {
			nvgText(args.vg, box.pos.x, box.pos.y, "trig", NULL);
		}
	}
	
};




struct HolonicSystemsJunctionsWidget : ModuleWidget {
	HolonicSystemsJunctionsWidget(HolonicSystemsJunctionsModule *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HolonicSystems-Junctions.svg")));
		
		addParam(createParam<CKSS>(Vec(43, 350), module, HolonicSystemsJunctionsModule::TRIG_MODE_PARAM));
		HolonicJunctionTrigGateLabel* const trigGateLabel = new HolonicJunctionTrigGateLabel(10, module);
		trigGateLabel->box.pos = Vec(10, 355/2+5);
		addChild(trigGateLabel);
		
		
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
