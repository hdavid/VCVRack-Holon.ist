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
	
	HolonicSystemsGapsModule();
	~HolonicSystemsGapsModule();
	
	
	void onReset() override {
		srand(time(NULL));
	}
	
	void step() override;
	LooseSchmittTrigger clockTrigger;
	LooseSchmittTrigger resetTrigger;
	
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
								{1,2,3, 4,5,6,7,8} // seq
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
	bool trigMode = params[TRIG_MODE_PARAM].value > 0;
	float deltaTime = engineGetSampleTime();
	
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
				lights[LED_1+i].setBrightnessSmooth(outputs[OUTPUT_1+i].value);
			} else {
				outputs[OUTPUT_1+i].value = on ? 10 : 0;
				lights[LED_1+i].setBrightness(outputs[OUTPUT_1+i].value);
				int f = pulses[i].process(deltaTime) ? 10.0 : 0.0;
				f = f+1;
			}
			
		} else {
			if (trigMode) {
				outputs[OUTPUT_1+i].value = pulses[i].process(deltaTime) ? 10.0 : 0.0;
				lights[LED_1+i].setBrightnessSmooth(outputs[OUTPUT_1+i].value);
			} else {
				int f = pulses[i].process(deltaTime) ? 10.0 : 0.0;
				f = f+1;
			}
		}
		
	}
	
	if (clock){
		this->counter++;
	}
}

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

	void draw(NVGcontext *vg) override {
		nvgFillColor(vg, nvgRGB(0, 0, 0));
		nvgFontSize(vg, fontSize);
		if (module) {
			sprintf(str, "%d", module->divisions[(int)module->params[HolonicSystemsGapsModule::MODE_PARAM].value][index]);
			nvgText(vg, box.pos.x, box.pos.y, str, NULL);
		}else {
			nvgText(vg, box.pos.x, box.pos.y, "", NULL);
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

	void draw(NVGcontext *vg) override {
		nvgFillColor(vg, nvgRGB(0, 0, 0));
		nvgFontSize(vg, fontSize);
		if (module){
			if ( module->params[HolonicSystemsGapsModule::TRIG_MODE_PARAM].value==0){
				nvgText(vg, box.pos.x, box.pos.y, "gate", NULL);
			}else{
				nvgText(vg, box.pos.x, box.pos.y, "trig", NULL);
			}
		}else{
			nvgText(vg, box.pos.x, box.pos.y, "trig", NULL);
		}
		
	}
};



struct HolonicSystemsGapsWidget : ModuleWidget {

	HolonicSystemsGapsWidget(HolonicSystemsGapsModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin,"res/HolonicSystems-Gaps.svg")));
		
		//screws
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		//IN
		addInput(Port::create<PJ301MPort>(Vec(5, 57), Port::INPUT, module, HolonicSystemsGapsModule::INPUT_CLOCK));
		addInput(Port::create<PJ301MPort>(Vec(30, 57), Port::INPUT, module, HolonicSystemsGapsModule::INPUT_RESET));
		
		addParam(ParamWidget::create<CKSS>(Vec(43, 355), module, HolonicSystemsGapsModule::TRIG_MODE_PARAM, 0, 1.0, 0.0));
		HolonicGapsTrigGateLabel* const trigGateLabel = new HolonicGapsTrigGateLabel(10, module);
		trigGateLabel->box.pos = Vec(10, 355/2+5);
		addChild(trigGateLabel);
		
		HolonicSystemsKnob *modeKnob = dynamic_cast<HolonicSystemsKnob*>(ParamWidget::create<HolonicSystemsKnob>(Vec(10, 90), module, HolonicSystemsGapsModule::MODE_PARAM, 0.0, 6, 0));
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
			addOutput(Port::create<PJ301MPort>(Vec(10,120 + 30*i), Port::OUTPUT, module, HolonicSystemsGapsModule::OUTPUT_1+i));
			addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(40,120 + 8+ 30*i), module, HolonicSystemsGapsModule::LED_1+i));
			HolonicGapsLabel* const modeLabel = new HolonicGapsLabel(12,module,i);
			modeLabel->box.pos = Vec(40/2,(120 + 8+ 30*i)/2-2);
			addChild(modeLabel);
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
