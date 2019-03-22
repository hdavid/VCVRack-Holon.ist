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

	
	HolonicSystemsHolonicSourceModule();

	~HolonicSystemsHolonicSourceModule(){
		ports.stop();
	}
	
	void step() override;
	
	void onReset() override {
		for (int i=0; i<NUM_OUTPUTS; i++) {
			lightValues[i] = 0;
			outputValues[i] = 0;
		}
	}

};


HolonicSystemsHolonicSourceModule::HolonicSystemsHolonicSourceModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	onReset();
	ports.start();
}



void HolonicSystemsHolonicSourceModule::step() {
	
	//bank
	ports.setBank((int) params[BANK_PARAM].value);
	
	float deltaTime = engineGetSampleTime();
	
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
			alpha = (alpha * alpha * alpha) * engineGetSampleTime() * 50; //.000023
			out = in * (alpha) + out * (1 - alpha);
		} else {
			//ignore filtering at low values
			out = in;
		}
		
		//publish output
		outputs[i].value = out;
		//monitoring leds
		lights[OUTPUT_LIGHT_POS_1 + 2*i].setBrightnessSmooth(fmaxf(0.0, outputs[i].value/5.0));	
		lights[OUTPUT_LIGHT_NEG_1 + 2*i].setBrightnessSmooth(fmaxf(0.0, -1 * outputs[i].value/5.0));
		
		//activity led
		if (ports.channelUpdated[i]) {
			ports.channelUpdated[i] = false;
			lightValues[i] = 1;
		}
		
		lights[i].setBrightness(lightValues[i]);
		lightValues[ACTIVITY_1_LIGHT+i] *= 1 - 10 * engineGetSampleTime();
	}

}


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

	void draw(NVGcontext *vg) override {
		nvgFillColor(vg, nvgRGB(0, 0, 0));
		nvgFontSize(vg, fontSize);
  		ret = gettimeofday (&tv, NULL); // timezone structure is obsolete
  	  	if (ret == 0) sec = (int)tv.tv_sec;
		if (module){
			if (sec%4==0||sec%4==1){
				nvgText(vg, box.pos.x, box.pos.y, module->ports.names[index].c_str(), NULL);
			} else {
				nvgText(vg, box.pos.x, box.pos.y, module->ports.inputs[index].c_str(), NULL);
			}
		} else {
			nvgText(vg, box.pos.x, box.pos.y, "", NULL);
		}
	}
};


struct HolonicSystemsHolonicSourceWidget : ModuleWidget {
	HolonicSystemsHolonicSourceWidget(HolonicSystemsHolonicSourceModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/HolonicSystems-HolonicSource.svg")));
		
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		//bank selector
		HolonicSystemsKnob *busKnob = dynamic_cast<HolonicSystemsKnob*>(ParamWidget::create<HolonicSystemsKnob>(Vec(84, 20-4), module, HolonicSystemsHolonicSourceModule::BANK_PARAM, 0.0, 7, 0));
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
		if (module){
			busKnob->connectLabel(busLabel);
		}
		addChild(busLabel);
		addParam(busKnob);
		
	
		//channels
		int start = 66;
		for (int i=0; i<8 ; i++) {
			addChild(ModuleLightWidget::create<MediumLight<RedLight>>(	Vec(10+4		, start + i * 36 + 8), module, HolonicSystemsHolonicSourceModule::ACTIVITY_1_LIGHT + i));
			addParam(ParamWidget::create<RoundSmallBlackKnob>(			Vec(10+30*0.5	, start + i * 36), module, HolonicSystemsHolonicSourceModule::ATT_1 + i, 0, 1.0, 1.0));
			addParam(ParamWidget::create<RoundSmallBlackKnob>(			Vec(10+30*1.5	, start + i * 36), module, HolonicSystemsHolonicSourceModule::ALPHA_1 + i, 1.0, 0.0, 0.8));
			addOutput(Port::create<PJ301MPort>(							Vec(10+30*2.5-3	, start + i * 36), Port::OUTPUT, module, HolonicSystemsHolonicSourceModule::OUTPUT_1 + i));
			addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(10+30*2.5+23, start+ i * 36 + 8), module, HolonicSystemsHolonicSourceModule::OUTPUT_LIGHT_POS_1+i*2));
			
			HolonistOSCLabel* const inputLabel = new HolonistOSCLabel(10, module, 0, i);
			inputLabel->box.pos = Vec(5, 30 + i * 18+ 20 - 1);
			addChild(inputLabel);
		}
		
		addParam(ParamWidget::create<CKSS>(Vec(47, 353), module, HolonicSystemsHolonicSourceModule::ONE_TEN_VOLT_OSC_PARAM_1, 0, 1.0, 1.0));
	}
};

Model *modelHolonicSystemsHolonicSource = 
	Model::create<HolonicSystemsHolonicSourceModule, HolonicSystemsHolonicSourceWidget>(
		"Holonic Systems",
	 	"HolonicSystems-HolonicSource", 
		"Holonic Source",
		CONTROLLER_TAG, 
		EXTERNAL_TAG
);
		
// Model *modelHolonicSystemsHolonistReceiver =
// 	Model::create<HolonicSystemsHolonicSourceModule, HolonicSystemsHolonicSourceWidget>(
// 		"Holonic Systems",
// 		"HolonicSystems-Holon.ist",
// 		"Holon.ist Receiver",
// 		CONTROLLER_TAG,
// 		EXTERNAL_TAG
// );
