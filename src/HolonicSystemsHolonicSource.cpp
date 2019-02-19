#include "HolonicSystems-Free.hpp"
#include "Ports.hpp"
#include <string>
#include <sys/time.h>

struct HolonicSystemsHolonicSourceModule : Module {

	enum ParamIds {
		ATT_1,
		ATT_2,
		ATT_3,
		ATT_4,
		ATT_5,
		ATT_6,
		ATT_7,
		ATT_8,
		ALPHA_1,
		ALPHA_2,
		ALPHA_3,
		ALPHA_4,
		ALPHA_5,
		ALPHA_6,
		ALPHA_7,
		ALPHA_8,
		BANK_PARAM,
		ONE_TEN_VOLT_OSC_PARAM_1,
		NUM_PARAMS
	};

	enum InputIds {
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
		ACTIVITY_1_LIGHT,
		ACTIVITY_2_LIGHT,
		ACTIVITY_3_LIGHT,
		ACTIVITY_4_LIGHT,
		ACTIVITY_5_LIGHT,
		ACTIVITY_6_LIGHT,
		ACTIVITY_7_LIGHT,
		ACTIVITY_8_LIGHT,
		OUTPUT_LIGHT_POS_1,
		OUTPUT_LIGHT_NEG_1,
		OUTPUT_LIGHT_POS_2,
		OUTPUT_LIGHT_NEG_2,
		OUTPUT_LIGHT_POS_3,
		OUTPUT_LIGHT_NEG_3,
		OUTPUT_LIGHT_POS_4,
		OUTPUT_LIGHT_NEG_4,
		OUTPUT_LIGHT_POS_5,
		OUTPUT_LIGHT_NEG_5,
		OUTPUT_LIGHT_POS_6,
		OUTPUT_LIGHT_NEG_6,
		OUTPUT_LIGHT_POS_7,
		OUTPUT_LIGHT_NEG_7,
		OUTPUT_LIGHT_POS_8,
		OUTPUT_LIGHT_NEG_8,
		NUM_LIGHTS
	};	
	
	float lightValues[8];
	double outputValues[8];
	Ports ports;

	
	HolonicSystemsHolonicSourceModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for(int i=0l;i<8;i++){
			params[ATT_1].config(0.f, 1.f, 1.f, "Attenuator");
			params[ALPHA_1].config(1.0f, 0.0f, 0.8f, "LPF");
		}
		params[BANK_PARAM].config(0.f, 7.f, 0.f, "Bus");
		params[ONE_TEN_VOLT_OSC_PARAM_1].config( 0.0f, 1.0f, 1.0f, "One or Ten");
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
		
		//bank
		ports.setBank((int) params[BANK_PARAM].value);
		
		float deltaTime = APP->engine->getSampleTime();
		
		//compute channels
		for (int i=0; i<NUM_OUTPUTS; i++) {
			
			//previous value
			float out = outputs[i].value;
			
			//get value
			ports.computeChannel(i, deltaTime);
			
			//attenuverter
			float in = params[ATT_1 + i].value * ports.channelValues[i] * ((ports.channelModes[i]==4 || ports.channelModes[i]==50) && params[ONE_TEN_VOLT_OSC_PARAM_1].value > 0 ? 10 : 1);
			
			//filter
			float alpha = params[ALPHA_1+i].value;
			if (alpha < 0.95) {
				// make our alpha sample rate linked
				// and exponantial 
				alpha = (alpha * alpha * alpha) * APP->engine->getSampleTime() * 50; //.000023
				out = in * (alpha) + out * (1 - alpha);
			} else {
				//ignore filtering at low values
				out = in;
			}
			
			//publish output
			outputs[i].value = out;
			//monitoring leds
			lights[OUTPUT_LIGHT_POS_1 + 2*i].setSmoothBrightness(fmaxf(0.0, outputs[i].value/5.0),APP->engine->getSampleTime());	
			lights[OUTPUT_LIGHT_NEG_1 + 2*i].setSmoothBrightness(fmaxf(0.0, -1 * outputs[i].value/5.0),APP->engine->getSampleTime());
			
			//activity led
			if (ports.channelUpdated[i]) {
				ports.channelUpdated[i] = false;
				lightValues[i] = 1;
			}
			
			lights[i].setBrightness(lightValues[i]);
			lightValues[ACTIVITY_1_LIGHT+i] *= 1 - 10 * APP->engine->getSampleTime();
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
		if (sec%4==0||sec%4==1){
			nvgText(args.vg, box.pos.x, box.pos.y, module->ports.names[index].c_str(), NULL);
		} else {
			nvgText(args.vg, box.pos.x, box.pos.y, module->ports.inputs[index].c_str(), NULL);
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
		HolonicSystemsKnob *busKnob = dynamic_cast<HolonicSystemsKnob*>(createParam<HolonicSystemsKnob>(Vec(84, 20-4), module, HolonicSystemsHolonicSourceModule::BANK_PARAM));
		HolonicSystemsLabel* const busLabel = new HolonicSystemsLabel;
		busLabel->box.pos = Vec(40, 27-2);
		busLabel->text = "mode";
		busKnob->names.push_back(std::string("Bus A"));
		busKnob->names.push_back(std::string("Bus B"));
		busKnob->names.push_back(std::string("Bus C"));
		busKnob->names.push_back(std::string("Bus D"));
		busKnob->names.push_back(std::string("Bus E"));
		busKnob->names.push_back(std::string("Bus F"));
		busKnob->names.push_back(std::string("Bus G"));
		busKnob->names.push_back(std::string("Bus H"));
		busKnob->connectLabel(busLabel);
		addChild(busLabel);
		addParam(busKnob);
		
	
		//channels
		int start = 66;
		for (int i=0; i<8 ; i++) {
			addChild(createLight<MediumLight<RedLight>>(	Vec(10+4		, start + i * 36 + 8), module, HolonicSystemsHolonicSourceModule::ACTIVITY_1_LIGHT + i));
			addParam(createParam<RoundSmallBlackKnob>(			Vec(10+30*0.5	, start + i * 36), module, HolonicSystemsHolonicSourceModule::ATT_1 + i));
			addParam(createParam<RoundSmallBlackKnob>(			Vec(10+30*1.5	, start + i * 36), module, HolonicSystemsHolonicSourceModule::ALPHA_1 + i));
			addOutput(createOutput<PJ301MPort>(							Vec(10+30*2.5-3	, start + i * 36), module, HolonicSystemsHolonicSourceModule::OUTPUT_1 + i));
			addChild(createLight<MediumLight<GreenRedLight>>(Vec(10+30*2.5+23, start+ i * 36 + 8), module, HolonicSystemsHolonicSourceModule::OUTPUT_LIGHT_POS_1+i*2));
			
			HolonistOSCLabel* const inputLabel = new HolonistOSCLabel(10, module, 0, i);
			inputLabel->box.pos = Vec(5, 30 + i * 18+ 20 - 1);
			addChild(inputLabel);
		}
		
		addParam(createParam<CKSS>(Vec(47, 353), module, HolonicSystemsHolonicSourceModule::ONE_TEN_VOLT_OSC_PARAM_1));
	}
};

Model *modelHolonicSystemsHolonicSource = createModel<HolonicSystemsHolonicSourceModule, HolonicSystemsHolonicSourceWidget>("HolonicSystems-HolonicSource");
		
// Model *modelHolonicSystemsHolonistReceiver =	createModel<HolonicSystemsHolonicSourceModule, HolonicSystemsHolonicSourceWidget>("HolonicSystems-Holon.ist");
