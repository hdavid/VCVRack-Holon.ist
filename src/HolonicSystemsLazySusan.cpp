#include "HolonicSystems-Free.hpp"
#include "dsp/digital.hpp"


struct HolonicSystemsLazySusanModule : Module {

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
	LooseSchmittTrigger scaleButtons[12];
	
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
	

	HolonicSystemsLazySusanModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		//IN
		configParam(PARAM_SCALE,0.f, 6.f, 0.f, "scale");	
		configParam(PARAM_SCALE_CV_ATT,0.f, 1.f, 1.f, "scale cv att");		
		configParam(PARAM_ATT,0.f, 1.f, 1.f, "common att");

		onReset();
	}


	~HolonicSystemsLazySusanModule() {
	}

		
	void onReset() override {
	}
	
	
	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		// scales
		json_t *scalesJ = json_array();
		for (int i = 0; i < 7*12; i++) {
			json_t *itemJ = json_boolean(scales[i]);
			json_array_append_new(scalesJ, itemJ);
		}
		json_object_set_new(rootJ, "scales", scalesJ);
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		// scales
		json_t *scalesJ = json_object_get(rootJ, "scales");
		if (scalesJ) {
			for (int i = 0; i < 7*12; i++) {
			json_t *itemJ = json_array_get(scalesJ, i);
			if (itemJ)
				scales[i] = json_boolean_value(itemJ);
			}
		}
	}
	
	void process(const ProcessArgs &args) override {
		float scaleCV = inputs[INPUT_SCALE_CV].active ? (inputs[INPUT_SCALE_CV].value/10) : 0.0;
		float scaleParam = params[PARAM_SCALE].value;
		float scaleCVATTParam = params[PARAM_SCALE_CV_ATT].value;

		int scaleIndex = ((int)(scaleCV * scaleCVATTParam * 7 + scaleParam))%7;
		int offset = scaleIndex * 12;


		float att = params[PARAM_ATT].value;


	for(int i=0;i<12;i++){
		bool buttonStatus 	= scaleButtons[i].process(params[PARAM_SCALE_1+i].value);
		if (buttonStatus){
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
						inputCV += inputs[INPUT_TRANSPOSE_CV].value;
					}

					//offset so that negative voltage are handled just the same.
					inputCV += 100; 

					//quantising
					int octave = (int)inputCV;
					float semitones = (inputCV-octave)*12;
					int below=0;
					int above=0;
					// set starting points
					// below is last note of below octave in scale
					// above is first note of above octave in scale
					for (int j=0; j<12; j++) {
						if (scales[offset+j]) {
							below = j-12;
							if (above==0){
								above = j+12;
							}
						}
					}
				
					// find closest above and below notes
					for (int j=0; j<12; j++) {
						if (scales[offset+j]) {
							if (j<semitones){
								below = j;
							}
							if (j>=semitones){
								above = j;
								break;
							}
						}
					}

					//TODO: handle over octave stuff.
					if (abs(above - semitones) < abs(below - semitones)){
						semitones=above;
					} else {
						semitones=below;
					}
				
					float newValue = octave + semitones / 12 - 100;

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


struct HolonicSystemsLazySusanWidget : ModuleWidget {
	HolonicSystemsLazySusanWidget(HolonicSystemsLazySusanModule *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HolonicSystems-LazySusan.svg")));
		
		// Screws
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	
 
		//IN
		rack::RoundSmallBlackKnob* scale = createParam<RoundSmallBlackKnob>(Vec(10,40), module, HolonicSystemsLazySusanModule::PARAM_SCALE);
		scale->snap = true;
		addParam(scale);
		
		addParam(createParam<RoundSmallBlackKnob>(Vec(123, 66+18*11), module, HolonicSystemsLazySusanModule::PARAM_SCALE_CV_ATT));
		
		addInput(createInput<PJ301MPort>(Vec(60, 40), module, HolonicSystemsLazySusanModule::INPUT_SCALE_CV));

		addParam(createParam<Trimpot>(Vec(90,40+5), module, HolonicSystemsLazySusanModule::PARAM_SCALE_CV_ATT));
		
		//transpose CV
		addInput(createInput<PJ301MPort>(Vec(10, 70), module, HolonicSystemsLazySusanModule::INPUT_TRANSPOSE_CV));

		//common ATT
		addParam(createParam<Trimpot>(Vec(90, 105), module, HolonicSystemsLazySusanModule::PARAM_ATT));
		

		int rowHeight = 65;
		int vSpace = 35;
		int base = 105;

		for (int i=0;i<4;i++){
			addInput(createInput<PJ301MPort>(Vec(10+vSpace*0, base + i*rowHeight), module, HolonicSystemsLazySusanModule::INPUT_CV_1+i));
			addInput(createInput<PJ301MPort>(Vec(10+vSpace*0, base + i*rowHeight+30), module, HolonicSystemsLazySusanModule::INPUT_TRIGGER_1+i));
			
			addOutput(createOutput<PJ301MPort>(Vec(10+vSpace*1, base + i*rowHeight), module, HolonicSystemsLazySusanModule::OUTPUT_CV_1+i));
			addOutput(createOutput<PJ301MPort>(Vec(10+vSpace*1, base + i*rowHeight+30), module, HolonicSystemsLazySusanModule::OUTPUT_TRIGGER_1+i));
		}
		
		

		int x = 7;
		base = 180;
		int row = 20;
		int left = 95;
		for (int i=0;i<12 ;i++ ) {
			//0		2		4	5		7		9		11
			//	1		3			6		8		10	
			if (i == 0 || i == 2 || i == 4 || i == 5 || i == 7 || i == 9 || i == 11) {	
				x--;
				addParam(createParam<TL1105>(Vec(left, base+ x*row), module, HolonicSystemsLazySusanModule::PARAM_SCALE_1+i));
				addChild(createLight<LargeLight<RedLight>>(Vec(left, base+ x*row), module, HolonicSystemsLazySusanModule::LIGHT_SCALE_1+i));
				if (i==0 || i == 2 || i == 5 || i == 7 || i == 9){
					addParam(createParam<TL1105>(Vec(left-20, base+x*row - row/2), module, HolonicSystemsLazySusanModule::PARAM_SCALE_1+i+1));
					addChild(createLight<LargeLight<RedLight>>(Vec(left-20, base+x*row - row/2), module, HolonicSystemsLazySusanModule::LIGHT_SCALE_1+i+1));
				}
			}
			
		}
	}

};



Model *modelHolonicSystemsLazySusan = createModel<HolonicSystemsLazySusanModule, HolonicSystemsLazySusanWidget>("HolonicSystems-LazySusan Quantiser");


