#include "HolonicSystems-Free.hpp"
#include "Ports.hpp"

struct HolonicSystemsHolonistModule : Module {

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
		NUM_LIGHTS
	};	
	
	float lightValues[8];
	double outputValues[8];
	Ports ports;
	
	HolonicSystemsHolonistModule();
	~HolonicSystemsHolonistModule();
	
	void step() override;
	
	void onReset() override {
		for (int i=0; i<NUM_OUTPUTS; i++) {
			lightValues[i] = 0;
			outputValues[i] = 0;
		}
	}

};


HolonicSystemsHolonistModule::HolonicSystemsHolonistModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	//printf("HolonicSystemsHolonistModule()\n");
	onReset();
 	ports.start();
}


HolonicSystemsHolonistModule::~HolonicSystemsHolonistModule() {
	//printf("~HolonicSystemsHolonistModule()\n");
	ports.stop();
}


void HolonicSystemsHolonistModule::step() {
	
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
		float in = params[ATT_1 + i].value * ports.channelValues[i] * ((ports.channelModes[i]==4 || ports.channelModes[i]==5) && params[ONE_TEN_VOLT_OSC_PARAM_1].value > 0 ? 10 : 1);
		
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
		
		//activity led
		if (ports.channelUpdated[i]) {
			ports.channelUpdated[i] = false;
			lightValues[i] = 1;
		}
		lights[i].setBrightness(lightValues[i]);
		lightValues[i] *= 1 - 10 * engineGetSampleTime();
	}

}


struct HolonicSystemsHolonistWidget : ModuleWidget {
	HolonicSystemsHolonistWidget(HolonicSystemsHolonistModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/HolonicSystems-Holonist.svg")));
		
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		//bank selector
		addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(84, 27), module, HolonicSystemsHolonistModule::BANK_PARAM, 0.0, 8.0, 0.0));
	
		//channels
		for (int i=0; i<8 ; i++) {
			addChild(ModuleLightWidget::create<MediumLight<RedLight>>(	Vec(10			, 70 + i * 36 + 8), module, HolonicSystemsHolonistModule::ACTIVITY_1_LIGHT + i));
			addParam(ParamWidget::create<RoundSmallBlackKnob>(			Vec(10+30*0.5	, 70 + i * 36), module, HolonicSystemsHolonistModule::ATT_1 + i, 0, 1.0, 1.0));
			addParam(ParamWidget::create<RoundSmallBlackKnob>(			Vec(10+30*1.5	, 70 + i * 36), module, HolonicSystemsHolonistModule::ALPHA_1 + i, 1.0, 0.0, 0.8));
			addOutput(Port::create<PJ301MPort>(							Vec(10+30*2.5	, 70 + i * 36), Port::OUTPUT, module, HolonicSystemsHolonistModule::OUTPUT_1 + i));
		}
		
		addParam(ParamWidget::create<CKSS>(Vec(47, 353), module, HolonicSystemsHolonistModule::ONE_TEN_VOLT_OSC_PARAM_1, 0, 1.0, 1.0));
	}
};

Model *modelHolonicSystemsHolonist = 
	Model::create<HolonicSystemsHolonistModule, HolonicSystemsHolonistWidget>(
		"Holonic Systems",
	 	"HolonicSystems-Holon.ist", 
		"Holon.ist Receiver",
		CONTROLLER_TAG, 
		EXTERNAL_TAG
);
