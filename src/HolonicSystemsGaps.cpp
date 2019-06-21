#include "HolonicSystems-Free.hpp"
#include "dsp/digital.hpp"
#include <vector>
#include <time.h>
#include <stdlib.h>

struct HolonicSystemsGapsModule : Module {

	enum ParamIds {
		MODE_PARAM,
		TRIG_MODE_PARAM,
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
	

	LooseSchmittTrigger clockTrigger;
	LooseSchmittTrigger resetTrigger;
	
	 int counter = 0;
	 std::vector<float> track;  
	 dsp::PulseGenerator pulses[8];
	 int divisions[7][8] = {
		{2,3,4,5, 6,7,8,9}, // int
		{2,4,6,8, 10,12,14,16}, // even
		{3,5,7,9, 11,13,15,17}, // odd
		{2,3,5,7, 11,13,17,19}, // prime
		{2,4,8,16, 32,64,128,256}, // binary

		{2,3,4,5, 6,7,8,9}, // random
		{1,2,3, 4,5,6,7,8} // seq
	};


	HolonicSystemsGapsModule(){
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(MODE_PARAM,0.f, 6.f, 0.f, "Division Mode");
		configParam(TRIG_MODE_PARAM,0.f, 1.f, 0.f, "Trigger/Gate Mode");
		onReset();
	}


	~HolonicSystemsGapsModule() {
	}

	
	void onReset() override {
		srand(time(NULL));
	}

	void process(const ProcessArgs &args) override {
		
		bool clock = clockTrigger.process(inputs[INPUT_CLOCK].value);
		bool reset = resetTrigger.process(inputs[INPUT_RESET].value);
		bool clockIsHigh = clockTrigger.isHigh();
		bool trigMode = params[TRIG_MODE_PARAM].value == 1;
		bool gateMode = params[TRIG_MODE_PARAM].value == 0;
		float deltaTime = APP->engine->getSampleTime();
		
		if (reset) {
			this->counter = 0;
		}
		
		for (int i=0;i<8;i++){
			bool on = false;
			if (clock) {
				if ((int)params[MODE_PARAM].value <= 4){
					if (trigMode){
						on =  0 == counter % divisions[(int)params[MODE_PARAM].value][i];
					} else {
						on =  divisions[(int)params[MODE_PARAM].value][i]/2 <= counter % divisions[(int)params[MODE_PARAM].value][i];		
					}
				} else if ((int)params[MODE_PARAM].value == 5){
					on =  /*(0 == counter % 2) &&*/ (rand() < RAND_MAX / divisions[(int)params[MODE_PARAM].value][i]);
				} else if ((int)params[MODE_PARAM].value == 6){
					on = (0 == (counter + 8 - i) % 8);
				}
				if (trigMode){
					if(on){
						pulses[i].trigger(1e-3);
					}
					outputs[OUTPUT_1+i].value = pulses[i].process(deltaTime) ? 10.0 : 0.0;
					lights[LED_1+i].setSmoothBrightness(outputs[OUTPUT_1+i].value, APP->engine->getSampleTime());
				} else if (gateMode) {
					outputs[OUTPUT_1+i].value = on ? 10 : 0;
					lights[LED_1+i].setSmoothBrightness(outputs[OUTPUT_1+i].value, APP->engine->getSampleTime());
					int f = pulses[i].process(deltaTime) ? 10.0 : 0.0;
					f = f+1;
				} else {
					outputs[OUTPUT_1+i].value = outputs[OUTPUT_1+i].value = on && clockIsHigh ? 10 : 0;
					lights[LED_1+i].setBrightness(outputs[OUTPUT_1+i].value);
					int f = pulses[i].process(deltaTime) ? 10.0 : 0.0;
					f = f+1;
				}
				
			} else {
				if (trigMode) {
					outputs[OUTPUT_1+i].value = pulses[i].process(deltaTime) ? 10.0 : 0.0;
					lights[LED_1+i].setSmoothBrightness(outputs[OUTPUT_1+i].value, APP->engine->getSampleTime());
				} else if (gateMode){
					int f = pulses[i].process(deltaTime) ? 10.0 : 0.0;
					f = f+1;
				} else {
					outputs[OUTPUT_1+i].value = outputs[OUTPUT_1+i].value = outputs[OUTPUT_1+i].value>0 && clockIsHigh ? 10 : 0;
					lights[LED_1+i].setSmoothBrightness(outputs[OUTPUT_1+i].value, APP->engine->getSampleTime());
					int f = pulses[i].process(deltaTime) ? 10.0 : 0.0;
					f = f+1;
				}
			}
			
		}
		
		if (clock){
			this->counter++;
		}
	}


};




struct HolonicGapsLabel : Widget {
	std::string text = "xx";
	int fontSize;
	int index;
	HolonicSystemsGapsModule *module = nullptr;
	char str[10];

	
	HolonicGapsLabel(int _fontSize,HolonicSystemsGapsModule *_module, int _index) {
		fontSize = _fontSize;
		box.size.y = BND_WIDGET_HEIGHT;
		module = _module;
		index = _index;
	}

	void draw(const DrawArgs &args) override {
		nvgFillColor(args.vg, nvgRGB(0, 0, 0));
		nvgFontSize(args.vg, fontSize);
		if (module) {
			sprintf(str, "%d", module->divisions[(int)module->params[HolonicSystemsGapsModule::MODE_PARAM].value][index]);
			nvgText(args.vg, box.pos.x, box.pos.y, str, NULL);
		} else {
			nvgText(args.vg, box.pos.x, box.pos.y, "", NULL);
		}
		
	}
};


struct HolonicGapsTrigGateLabel : Widget {
	int fontSize;
	HolonicSystemsGapsModule *module = nullptr;

	
	HolonicGapsTrigGateLabel(int _fontSize,HolonicSystemsGapsModule *_module) {
		fontSize = _fontSize;
		box.size.y = BND_WIDGET_HEIGHT;
		module = _module;
	}

	void draw(const DrawArgs &args) override {
		nvgFillColor(args.vg, nvgRGB(0, 0, 0));
		nvgFontSize(args.vg, fontSize);
		if (module){
			if (module->params[HolonicSystemsGapsModule::TRIG_MODE_PARAM].value==0){
				nvgText(args.vg, box.pos.x, box.pos.y, "gate", NULL);
			} else if (module->params[HolonicSystemsGapsModule::TRIG_MODE_PARAM].value==1){
				nvgText(args.vg, box.pos.x, box.pos.y, "trig", NULL);
			}else{
				nvgText(args.vg, box.pos.x, box.pos.y, "as is", NULL);
			}
		} else {
			nvgText(args.vg, box.pos.x, box.pos.y, "trig", NULL);
		}
	}
};



struct HolonicSystemsGapsWidget : ModuleWidget {

	HolonicSystemsGapsWidget(HolonicSystemsGapsModule *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance,"res/HolonicSystems-Gaps.svg")));
		
		//screws
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		//IN
		addInput(createInput<PJ301MPort>(Vec(5, 57), module, HolonicSystemsGapsModule::INPUT_CLOCK));
		addInput(createInput<PJ301MPort>(Vec(30, 57), module, HolonicSystemsGapsModule::INPUT_RESET));
		
		addParam(createParam<CKSS>(Vec(43, 355), module, HolonicSystemsGapsModule::TRIG_MODE_PARAM));
		HolonicGapsTrigGateLabel* const trigGateLabel = new HolonicGapsTrigGateLabel(10, module);
		trigGateLabel->box.pos = Vec(10, 355/2+5);
		addChild(trigGateLabel);
		
		HolonicSystemsKnob *modeKnob = dynamic_cast<HolonicSystemsKnob*>(createParam<HolonicSystemsKnob>(Vec(10, 90), module, HolonicSystemsGapsModule::MODE_PARAM));
		HolonicSystemsLabel* const modeLabel = new HolonicSystemsLabel;
		modeLabel->box.pos = Vec(17, 54);
		modeLabel->text = "mode";
		modeLabel->fontSize=10;
		modeKnob->names.push_back(std::string("int"));
		modeKnob->names.push_back(std::string("even"));
		modeKnob->names.push_back(std::string("odd"));
		modeKnob->names.push_back(std::string("prime"));
		modeKnob->names.push_back(std::string("binary"));
		modeKnob->names.push_back(std::string("rand"));
		modeKnob->names.push_back(std::string("seq"));
		if (module){
			modeKnob->connectLabel(modeLabel);
		}
		addChild(modeLabel);
		addParam(modeKnob);
		

		for (int i=0;i<8;i++){
			addOutput(createOutput<PJ301MPort>(Vec(10,120 + 30*i), module, HolonicSystemsGapsModule::OUTPUT_1+i));
			addChild(createLight<MediumLight<RedLight>>(Vec(40,120 + 8+ 30*i), module, HolonicSystemsGapsModule::LED_1+i));
			HolonicGapsLabel* const modeLabel = new HolonicGapsLabel(12,module,i);
			modeLabel->box.pos = Vec(40/2,(120 + 8+ 30*i)/2-2);
			addChild(modeLabel);
		}
		
	}

};




Model *modelGaps = createModel<HolonicSystemsGapsModule, HolonicSystemsGapsWidget>("HolonicSystems-Gaps");
