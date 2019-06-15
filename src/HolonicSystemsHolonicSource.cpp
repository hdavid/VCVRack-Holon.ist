#include "HolonicSystems-Free.hpp"
#include "Ports.hpp"
#include <string>
#include <sys/time.h>

struct HolonicSystemsHolonicSourceModule : Module {

	enum ParamIds {
		PARAM_ATT_1,
		PARAM_ATT_2,
		PARAM_ATT_3,
		PARAM_ATT_4,
		PARAM_ATT_5,
		PARAM_ATT_6,
		PARAM_ATT_7,
		PARAM_ATT_8,
		PARAM_ALPHA_1,
		PARAM_ALPHA_2,
		PARAM_ALPHA_3,
		PARAM_ALPHA_4,
		PARAM_ALPHA_5,
		PARAM_ALPHA_6,
		PARAM_ALPHA_7,
		PARAM_ALPHA_8,
		PARAM_BUS,
		PARAM_ONE_TEN_VOLT_OSC_1,
		PARAM_S_H_1,
		PARAM_S_H_2,
		PARAM_S_H_3,
		PARAM_S_H_4,
		PARAM_S_H_5,
		PARAM_S_H_6,
		PARAM_S_H_7,
		PARAM_S_H_8,
		NUM_PARAMS
	};

	enum InputIds {
		INPUT_CLOCK,
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
		LIGHT_ACTIVITY_1,
		LIGHT_ACTIVITY_2,
		LIGHT_ACTIVITY_3,
		LIGHT_ACTIVITY_4,
		LIGHT_ACTIVITY_5,
		LIGHT_ACTIVITY_6,
		LIGHT_ACTIVITY_7,
		LIGHT_ACTIVITY_8,
		LIGHT_OUTPUT_POS_1,
		LIGHT_OUTPUT_NEG_1,
		LIGHT_OUTPUT_POS_2,
		LIGHT_OUTPUT_NEG_2,
		LIGHT_OUTPUT_POS_3,
		LIGHT_OUTPUT_NEG_3,
		LIGHT_OUTPUT_POS_4,
		LIGHT_OUTPUT_NEG_4,
		LIGHT_OUTPUT_POS_5,
		LIGHT_OUTPUT_NEG_5,
		LIGHT_OUTPUT_POS_6,
		LIGHT_OUTPUT_NEG_6,
		LIGHT_OUTPUT_POS_7,
		LIGHT_OUTPUT_NEG_7,
		LIGHT_OUTPUT_POS_8,
		LIGHT_OUTPUT_NEG_8,
		NUM_LIGHTS
	};	
	
	float lightValues[8];
	double outputValues[8];
	Ports ports;
	LooseSchmittTrigger clockTrigger;

	
	HolonicSystemsHolonicSourceModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for(int i=0l;i<8;i++){
			configParam(PARAM_ATT_1+i,0.f, 1.f, 1.f, "Attenuator");
			configParam(PARAM_ALPHA_1+i, 1.0f, 0.0f, 0.8f, "LPF");
			configParam(PARAM_S_H_1+1, 1.0f, 0.0f, 0.0f, "S/H");
		}
		configParam(PARAM_BUS, 0.f, 7.f, 0.f, "Bus");
		configParam(PARAM_ONE_TEN_VOLT_OSC_1, 0.0f, 1.0f, 1.0f, "One or Ten");
		onReset();
		ports.start();
	}


	~HolonicSystemsHolonicSourceModule() {
		ports.stop();
	}

	void onReset() override {
		for (int i=0; i<NUM_OUTPUTS; i++) {
			lightValues[i] = 0;
			outputValues[i] = 0;
		}
	}



	void process(const ProcessArgs &args) override {
	
		bool clock = clockTrigger.process(inputs[INPUT_CLOCK].value);
		//bank
		ports.setBank((int) params[PARAM_BUS].value);
		
		float deltaTime = APP->engine->getSampleTime();
		
		//compute channels
		for (int i=0; i<NUM_OUTPUTS; i++) {
			
			//previous value
			float out = outputValues[i];
			
			//get value
			ports.computeChannel(i, deltaTime);
			
			//attenuverter
			float in = params[PARAM_ATT_1 + i].value * ports.channelValues[i] * ((ports.channelModes[i]==4 || ports.channelModes[i]==50) && params[PARAM_ONE_TEN_VOLT_OSC_1].value > 0 ? 10 : 1);
			
			//filter
			float alpha = params[PARAM_ALPHA_1+i].value;
			if (alpha < 0.95) {
				// make our alpha sample rate linked
				// and exponantial 
				alpha = (alpha * alpha * alpha) * APP->engine->getSampleTime() * 50; //.000023
				out = in * (alpha) + out * (1 - alpha);
			} else {
				//ignore filtering at low values
				out = in;
			}
			
			outputValues[i] = out;
			
			
			if (params[PARAM_S_H_1+i].value == 0 || (clock && params[PARAM_S_H_1+i].value > 0) ) {
				//publish output if on clock or no s_h
				outputs[i].value = outputValues[i];
			}
			//monitoring leds
			lights[LIGHT_OUTPUT_POS_1 + 2*i].setSmoothBrightness(fmaxf(0.0, outputs[i].value/5.0),APP->engine->getSampleTime());	
			lights[LIGHT_OUTPUT_NEG_1 + 2*i].setSmoothBrightness(fmaxf(0.0, -1 * outputs[i].value/5.0),APP->engine->getSampleTime());
			
			//activity led
			if (ports.channelUpdated[i]) {
				ports.channelUpdated[i] = false;
				lightValues[i] = 1;
			}
			
			lights[i].setBrightness(lightValues[i]);
			lightValues[LIGHT_ACTIVITY_1+i] *= 1 - 10 * APP->engine->getSampleTime();
		}

	}

};





struct HolonistOSCLabel : Widget {
	std::string text = "xx";
	int fontSize;
	int type;
	int index;
	struct timeval tv;
	int sec = 0;
	int ret = 0;
	HolonicSystemsHolonicSourceModule *module = nullptr;

	HolonistOSCLabel(int _fontSize, HolonicSystemsHolonicSourceModule *_module, int _type, int _index) {
		fontSize = _fontSize;
		box.size.y = BND_WIDGET_HEIGHT;
		module = _module;
		type = _type;
		index = _index;
	}

	void draw(const DrawArgs &args) override {
		nvgFillColor(args.vg, nvgRGB(0, 0, 0));
		nvgFontSize(args.vg, fontSize);
  		ret = gettimeofday (&tv, NULL); // timezone structure is obsolete
  	  	if (ret == 0) sec = (int)tv.tv_sec;
		if (module){		
			if (sec%4==0||sec%4==1){
				nvgText(args.vg, box.pos.x, box.pos.y, module->ports.names[index].c_str(), NULL);
			} else {
				nvgText(args.vg, box.pos.x, box.pos.y, module->ports.inputs[index].c_str(), NULL);
			}
		} else {
			nvgText(args.vg, box.pos.x, box.pos.y, "", NULL);
		}
	}
};


struct HolonicSystemsHolonicSourceWidget : ModuleWidget {
	HolonicSystemsHolonicSourceWidget(HolonicSystemsHolonicSourceModule *module)  {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HolonicSystems-HolonicSource.svg")));
		
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		//bank selector
		HolonicSystemsKnob *busKnob = dynamic_cast<HolonicSystemsKnob*>(createParam<HolonicSystemsKnob>(Vec(118, 20-3), module, HolonicSystemsHolonicSourceModule::PARAM_BUS));
		HolonicSystemsLabel* const busLabel = new HolonicSystemsLabel;
		busLabel->box.pos = Vec(40+18, 27-2);
		busLabel->text = "mode";
		busKnob->names.push_back(std::string("Bus A"));
		busKnob->names.push_back(std::string("Bus B"));
		busKnob->names.push_back(std::string("Bus C"));
		busKnob->names.push_back(std::string("Bus D"));
		busKnob->names.push_back(std::string("Bus E"));
		busKnob->names.push_back(std::string("Bus F"));
		busKnob->names.push_back(std::string("Bus G"));
		busKnob->names.push_back(std::string("Bus H"));
		if (module){
			busKnob->connectLabel(busLabel);
		}
		addChild(busLabel);
		addParam(busKnob);
		
		addInput(createInput<PJ301MPort>(Vec(81, 20), module, HolonicSystemsHolonicSourceModule::INPUT_CLOCK));
	
		//channels
		int start = 66;
		for (int i=0; i<8 ; i++) {
			addChild(createLight<MediumLight<RedLight>>(	Vec(10+4		, start + i * 36 + 8), module, HolonicSystemsHolonicSourceModule::LIGHT_ACTIVITY_1 + i));
			addParam(createParam<RoundSmallBlackKnob>(			Vec(10+30*0.5	, start + i * 36), module, HolonicSystemsHolonicSourceModule::PARAM_ATT_1 + i));
			addParam(createParam<RoundSmallBlackKnob>(			Vec(10+30*1.5	, start + i * 36), module, HolonicSystemsHolonicSourceModule::PARAM_ALPHA_1 + i));
			addParam(createParam<CKSS>(Vec(10+30*2.5	, start + i * 36 +3), module, HolonicSystemsHolonicSourceModule::PARAM_S_H_1+i));
			addOutput(createOutput<PJ301MPort>(							Vec(10+30*3-3 + 10	, start + i * 36), module, HolonicSystemsHolonicSourceModule::OUTPUT_1 + i));
			addChild(createLight<MediumLight<GreenRedLight>>(Vec(10+30*3+23 + 10, start+ i * 36 + 8), module, HolonicSystemsHolonicSourceModule::LIGHT_OUTPUT_POS_1+i*2));
			
			HolonistOSCLabel* const inputLabel = new HolonistOSCLabel(10, module, 0, i);
			inputLabel->box.pos = Vec(5, 30 + i * 18+ 20 - 1);
			addChild(inputLabel);
			
			
		}
		
		addParam(createParam<CKSS>(Vec(47, 353), module, HolonicSystemsHolonicSourceModule::PARAM_ONE_TEN_VOLT_OSC_1));
	}
};

Model *modelHolonicSource = createModel<HolonicSystemsHolonicSourceModule, HolonicSystemsHolonicSourceWidget>("HolonicSource");
