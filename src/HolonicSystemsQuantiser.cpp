#include "HolonicSystems-Free.hpp"
#include "dsp/digital.hpp"


struct HolonicSystemsQuantiserModule : Module {

	enum ParamIds {
		PARAM_SCALE,
		PARAM_SCALE_CV_ATT,
		PARAM_ATT,
		PARAM_SCALE_1,
		PARAM_SCALE_2,
		PARAM_SCALE_3,
		PARAM_SCALE_4,
		PARAM_SCALE_5,
		PARAM_SCALE_6,
		PARAM_SCALE_7,
		PARAM_SCALE_8,
		PARAM_SCALE_9,
		PARAM_SCALE_10,
		PARAM_SCALE_11,
		PARAM_SCALE_12,
		NUM_PARAMS
	};

	enum InputIds {
		INPUT_CV_1,
		INPUT_CV_2,
		INPUT_CV_3,
		INPUT_CV_4,
		INPUT_TRIGGER_1,
		INPUT_TRIGGER_2,
		INPUT_TRIGGER_3,
		INPUT_TRIGGER_4,
		INPUT_SCALE_CV,
		INPUT_TRANSPOSE_CV,
		NUM_INPUTS
	};

	enum OutputIds {
		OUTPUT_CV_1,
		OUTPUT_CV_2,
		OUTPUT_CV_3,
		OUTPUT_CV_4,
		OUTPUT_TRIGGER_1,
		OUTPUT_TRIGGER_2,
		OUTPUT_TRIGGER_3,
		OUTPUT_TRIGGER_4,
		NUM_OUTPUTS
	};
	
	enum LightIds {
		LIGHT_SCALE_1,
		LIGHT_SCALE_2,
		LIGHT_SCALE_3,
		LIGHT_SCALE_4,
		LIGHT_SCALE_5,
		LIGHT_SCALE_6,
		LIGHT_SCALE_7,
		LIGHT_SCALE_8,
		LIGHT_SCALE_9,
		LIGHT_SCALE_10,
		LIGHT_SCALE_11,
		LIGHT_SCALE_12,
		NUM_LIGHTS
	};	
	
	dsp::PulseGenerator outputTriggers[4];
	LooseSchmittTrigger inputTriggers[4];
	
	bool scales[7 * 12] = {
		  1,0,1,0,1,1,0,1,0,1,0,1,	// C Ionian (I)
		//0,1,0,1,1,0,1,0,1,0,1,1,
		  1,0,1,1,0,1,0,1,0,1,1,0,	// D Dorian (II)
		//0,1,1,0,1,0,1,0,1,1,0,1,
		  1,1,0,1,0,1,0,1,1,0,1,0,	// E Phrygian (III)
		  1,0,1,0,1,0,1,1,0,1,0,1,	// F Lydian (IV)
		//0,1,0,1,0,1,1,0,1,0,1,1,
		  1,0,1,0,1,1,0,1,0,1,1,0,	// G Myxolydian (V)
		//0,1,0,1,1,0,1,0,1,1,0,1,
		  1,0,1,1,0,1,0,1,1,0,1,0,	// A Aeolian (VI)
		//0,1,1,0,1,0,1,1,0,1,0,1,
		  1,1,0,1,0,1,1,0,1,0,1,0	// F Locrian (VII)
		
	};

	float currentCVs[4] = {0.0f,0.0f,0.0f,0.0f};
	

	HolonicSystemsQuantiserModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		//IN
		params[PARAM_SCALE].config(0.f, 6.f, 0.f, "scale");	
		params[PARAM_SCALE_CV_ATT].config(0.f, 1.f, 1.f, "scale cv att");		
		params[PARAM_ATT].config(0.f, 1.f, 1.f, "common att");

		onReset();
	}


	~HolonicSystemsQuantiserModule() {
	}

		
	void onReset() override {
	}
	
	void process(const ProcessArgs &args) override {
		float transposeCV = inputs[INPUT_TRANSPOSE_CV].value;
		float scaleCV = inputs[INPUT_SCALE_CV].active ? (inputs[INPUT_SCALE_CV].value/10) : 0.0;
		float scaleParam = params[PARAM_SCALE].value;
		float scaleCVATTParam = params[PARAM_SCALE_CV_ATT].value;

		int scaleIndex = ((int)(scaleCV * scaleCVATTParam * 7 + scaleParam))%7;
		int offset = scaleIndex * 12;


		float att = params[PARAM_ATT].value;


		for(int i=0;i<12;i++){
			if (params[PARAM_SCALE_1+i].value > 0){
				scales[offset+i] = !scales[offset+i];
			}
		}
		
		//scale display
		for(int i=0;i<12;i++){
			lights[LIGHT_SCALE_1+i].setBrightness(scales[offset+i]?10:0);
		}

		for (int i = 0; i<4; i++) {
			if (inputs[INPUT_CV_1+i].active){
			
				bool triggerIn = inputTriggers[i].process(inputs[INPUT_TRIGGER_1+i].value);
				if (!inputs[INPUT_TRIGGER_1+i].active || triggerIn) {

					float inputCV = att * inputs[INPUT_CV_1+i].value;

					// transpose
					if (inputs[INPUT_TRANSPOSE_CV].active){
						inputCV += transposeCV;
					}

					//quantising
					int octave = (int)inputCV;
					float semitones = (inputCV-octave)*12;
					int below=0;
					int above=0;
					for (int j=0; j<12; j++) {
						if (scales[offset+j]) {
							if (semitones<j){
								below = j;
							}
							if (semitones>=j){
								above = j;
							}
						}
					}

					//TODO: handle over octave stuff.
					if (abs(above - semitones) > abs(below - semitones)){
						semitones=below;
					} else {
						semitones=above;
					}

					float newValue = octave + semitones / 12;

					if (!inputs[INPUT_TRIGGER_1+1].active || triggerIn) {
						if (newValue != currentCVs[i]) {
							currentCVs[i] = newValue;
							outputs[OUTPUT_CV_1+i].value = currentCVs[i];
							//trigger
							outputTriggers[i].trigger(1e-3);
						}
					}
				}

				// Process Triggers
				float deltaTime = APP->engine->getSampleTime();
				outputs[OUTPUT_TRIGGER_1+i].value = outputTriggers[i].process(deltaTime) ? 10.0 : 0.0;

			}
		
		}
	}
	
	 
};


struct HolonicSystemsQuantiserWidget : ModuleWidget {
	HolonicSystemsQuantiserWidget(HolonicSystemsQuantiserModule *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HolonicSystems-Quantiser.svg")));
		
		// Screws
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	
 
		//IN
		rack::RoundSmallBlackKnob* scale = createParam<RoundSmallBlackKnob>(Vec(10,40), module, HolonicSystemsQuantiserModule::PARAM_SCALE);
		scale->snap = true;
		addParam(scale);
		
		addParam(createParam<RoundSmallBlackKnob>(Vec(123, 66+18*11), module, HolonicSystemsQuantiserModule::PARAM_SCALE_CV_ATT));
		
		addInput(createInput<PJ301MPort>(Vec(50, 40), module, HolonicSystemsQuantiserModule::INPUT_SCALE_CV));

		addParam(createParam<Trimpot>(Vec(80,40+5), module, HolonicSystemsQuantiserModule::PARAM_SCALE_CV_ATT));
		
		//transpose CV
		addInput(createInput<PJ301MPort>(Vec(10, 70), module, HolonicSystemsQuantiserModule::INPUT_TRANSPOSE_CV));

		//common ATT
		addParam(createParam<Trimpot>(Vec(90, 85), module, HolonicSystemsQuantiserModule::PARAM_ATT));
		

		int rowHeight = 65;
		int vSpace = 35;
		int base = 105;

		for (int i=0;i<4;i++){
			addInput(createInput<PJ301MPort>(Vec(10+vSpace*0, base + i*rowHeight), module, HolonicSystemsQuantiserModule::INPUT_CV_1+i));
			addInput(createInput<PJ301MPort>(Vec(10+vSpace*0, base + i*rowHeight+30), module, HolonicSystemsQuantiserModule::INPUT_TRIGGER_1+i));
			
			addOutput(createOutput<PJ301MPort>(Vec(10+vSpace*1, base + i*rowHeight), module, HolonicSystemsQuantiserModule::OUTPUT_CV_1+i));
			addOutput(createOutput<PJ301MPort>(Vec(10+vSpace*1, base + i*rowHeight+30), module, HolonicSystemsQuantiserModule::OUTPUT_TRIGGER_1+i));
		}
		
		

		int x = 7;
		base = 130;
		for (int i=0;i<12 ;i++ ) {
			//0		2		4	5		7		9		11
			//	1		3			6		8		10	
			if (i == 0 || i == 2 || i == 4 || i == 5 || i == 7 || i == 9 || i == 11) {	
				x--;
				addChild(createLight<LargeLight<RedLight>>(Vec(10+65+25, base+ x*30), module, HolonicSystemsQuantiserModule::LIGHT_SCALE_1+i));
				addParam(createParam<TL1105>(Vec(10+65+25-15, base+ x*30), module, HolonicSystemsQuantiserModule::PARAM_SCALE_1+i));
			
				if (i==0 || i == 2 || i == 5 || i == 7 || i == 9){
					addChild(createLight<LargeLight<RedLight>>(Vec(10+65, base+x*30 - 15), module, HolonicSystemsQuantiserModule::LIGHT_SCALE_1+i+1));
					addParam(createParam<TL1105>(Vec(10+65-15, base+x*30 - 15), module, HolonicSystemsQuantiserModule::PARAM_SCALE_1+i+1));
				}
			}
			
		}
	}

};



Model *modelHolonicSystemsQuantiser = createModel<HolonicSystemsQuantiserModule, HolonicSystemsQuantiserWidget>("HolonicSystems-Quantiser");


