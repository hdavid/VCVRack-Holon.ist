#include "HolonicSystems-Free.hpp"
#include "dsp/digital.hpp"

struct HolonicSystemsSwissCheeseKnifeModule : Module {

	enum ParamIds {
		PARAM_ATT_1,
		PARAM_ATT_2,
		PARAM_ATT_3,
		PARAM_ATT_4,
		PARAM_INV_1,
		PARAM_INV_2,
		PARAM_INV_3,
		PARAM_INV_4,
		PARAM_LPF_1,
		PARAM_LPF_2,
		PARAM_LPF_3,
		PARAM_LPF_4,
		PARAM_SLEW_UP_1,
		PARAM_SLEW_UP_2,
		PARAM_SLEW_UP_3,
		PARAM_SLEW_UP_4,
		PARAM_SLEW_DOWN_1,
		PARAM_SLEW_DOWN_2,
		PARAM_SLEW_DOWN_3,
		PARAM_SLEW_DOWN_4,
		PARAM_SLEW_SHAPE_1,
		PARAM_SLEW_SHAPE_2,
		PARAM_SLEW_SHAPE_3,
		PARAM_SLEW_SHAPE_4,	
		PARAM_VCA_RESPONSE_1,
		PARAM_VCA_RESPONSE_2,
		PARAM_VCA_RESPONSE_3,
		PARAM_VCA_RESPONSE_4,
		PARAM_OFFSET_1,
		PARAM_OFFSET_2,
		PARAM_OFFSET_3,
		PARAM_OFFSET_4,
		PARAM_AC_DC_1,
		PARAM_AC_DC_2,
		PARAM_AC_DC_3,
		PARAM_AC_DC_4,
		NUM_PARAMS
	};

	enum InputIds {
		INPUT_1,
		INPUT_2,
		INPUT_3,
		INPUT_4,
		INPUT_VCA_1,
		INPUT_VCA_2,
		INPUT_VCA_3,
		INPUT_VCA_4,
		INPUT_SH_1,
		INPUT_SH_2,
		INPUT_SH_3,
		INPUT_SH_4,
		NUM_INPUTS
	};

	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		OUTPUT_4,
		NUM_OUTPUTS
	};
	
	enum LightIds {
		NUM_LIGHTS
	};	
	

	LooseSchmittTrigger sampleAndHoldTrigger[NUM_OUTPUTS];
	float in[NUM_OUTPUTS];
	float preLPF[NUM_OUTPUTS];
	float preSlew[NUM_OUTPUTS];
	float postSlew[NUM_OUTPUTS];
	float out[NUM_OUTPUTS];
	double avg[NUM_OUTPUTS];



	HolonicSystemsSwissCheeseKnifeModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for(int i=0;i<4;i++){
			configInput(INPUT_1+i, "Input");
			configInput(INPUT_VCA_1+i, "VCA");
			configInput(INPUT_SH_1+i, "S/H");
			configOutput(OUTPUT_1+i, "Output");
			configParam(PARAM_ATT_1+i,0.f, 1.f, 1.f, "Attenuator");
			configSwitch(PARAM_INV_1+i,0.f, 1.f, 1.f, "Invert", {"Inverted", "Normal"});
			configParam(PARAM_LPF_1+i,1.f, 0.f, 1.f, "LPF");
			configParam(PARAM_SLEW_UP_1+i,0.f, 1.f, 0.f, "Slew Up");
			configParam(PARAM_SLEW_DOWN_1+i,0.f, 1.f, 0.f, "Slew Down");
			configParam(PARAM_SLEW_SHAPE_1+i,0.f, 1.f, 0.5f, "Slew Response");
			configParam(PARAM_VCA_RESPONSE_1+i,0.f, 1.f, 0.5f, "VCA Response");
			configParam(PARAM_OFFSET_1+i,-5.0f, 5.0f, 0.0f, "Offset");
			configSwitch(PARAM_AC_DC_1+i,0.0f, 1.0f, 1.0f, "AC/DC" , {"AC Coupled", "DC Coupled"});
			configInput(INPUT_1+i,"CV");
			configInput(INPUT_VCA_1+i,"VCA");
			configInput(INPUT_SH_1+i,"S/H");
			configOutput(OUTPUT_1+i,"CV");
		}
		onReset();
	}


	~HolonicSystemsSwissCheeseKnifeModule() {
	}

	void onReset() override {
	}

	void process(const ProcessArgs &args) override {
			
		//compute channels
		for (int i=0; i<NUM_OUTPUTS; i++) {
			
			// input
			if (!inputs[INPUT_1+i].active) {
				inputs[INPUT_1+i].value = 0;
			}

			//sample and hold
			float att = params[PARAM_ATT_1+i].value;
			float inv = params[PARAM_INV_1+i].value<0.5?-1:1;
			bool sample = !inputs[INPUT_SH_1+i].active || sampleAndHoldTrigger[i].process(inputs[INPUT_SH_1+i].value);
			if (sample){
				in[i] = inputs[INPUT_1+i].value * inv * att;
			}
			
			
			//VCA
			//TODO: implement VCA response curve
			if (inputs[INPUT_VCA_1+i].active) {
				float vca_response = params[PARAM_VCA_RESPONSE_1+i].value;
				float linear = fmaxf(inputs[INPUT_VCA_1+i].value / 5.0, 0.0);
				linear = clamp(linear, 0.0f, 2.0f);
				const float base = 200.0;
				float exponential = rescale(powf(base, linear / 2.0f), 1.0f, base, 0.0f, 10.0f);
				preLPF[i] = in[i] * crossfade(exponential, linear, vca_response);
			} else {
				preLPF[i] = in[i];
			}
			
			
			// Low Pass Filter
			float lpf = params[PARAM_LPF_1+i].value;
			//out = lpf(out, i);
			if (lpf < 0.99) {
				// make our alpha sample rate linked and exponantial 
				lpf = (lpf * lpf * lpf * lpf) * APP->engine->getSampleTime() * 5000;
				preSlew[i] = preLPF[i] * (lpf) + preSlew[i] * (1 - lpf);
			} else {
				//ignore filtering at low values
				preSlew[i] = preLPF[i];
			}
			

			//slew
			if (params[PARAM_SLEW_UP_1+i].value>0.01 || params[PARAM_SLEW_DOWN_1+i].value>0.01) {
				float shape = 1.0 - params[PARAM_SLEW_SHAPE_1+i].value;
				// minimum and maximum slopes in volts per second
				const float slewMin = 0.1;
				const float slewMax = 10000.0;
				// Amount of extra slew per voltage difference
				const float shapeScale = 1/10.0;
				// Rise
				if (preSlew[i] > postSlew[i]) {
					float rise = params[PARAM_SLEW_UP_1+i].value;
					float slew = slewMax * powf(slewMin / slewMax, rise);
					postSlew[i] += slew * crossfade(1.0f, shapeScale * (preSlew[i] - postSlew[i]), shape) * APP->engine->getSampleTime();
					if (postSlew[i] > preSlew[i]) {
						postSlew[i]  = preSlew[i];
					}
				} else if (preSlew[i] < postSlew[i]) {
					// Fall
					float fall = params[PARAM_SLEW_DOWN_1+i].value;
					float slew = slewMax * powf(slewMin / slewMax, fall);
					postSlew[i] -= slew * crossfade(1.0f, shapeScale * (postSlew[i] - preSlew[i]), shape) * APP->engine->getSampleTime();
					if (postSlew[i] < preSlew[i]) {
						postSlew[i] = preSlew[i];
					}
				}
			} else {
				postSlew[i] = preSlew[i];
			}

			

			//offset
			float offset = params[PARAM_OFFSET_1+i].value;

			out[i] = postSlew[i] + offset;
			
			

			//remove dc offset
			double alpha = 0.00001;
			avg[i] = alpha * ((double)out[i]) + (1-alpha) * avg[i];
			if (params[PARAM_AC_DC_1+i].value<0.4){
				outputs[OUTPUT_1+i].value = out[i] - avg[i];
			} else {
				outputs[OUTPUT_1+i].value = out[i];
			}

			//output normaling
			if (i>0 && !outputs[OUTPUT_1+i-1].active) {
				outputs[OUTPUT_1+i].value += outputs[OUTPUT_1+i-1].value;
			}

			//clamp
			outputs[OUTPUT_1+i].value = clamp(outputs[OUTPUT_1+i].value, -10.0, 10.0);
		}
	}

};





struct HolonicSystemsSwissCheeseKnifeWidget : ModuleWidget {

	HolonicSystemsSwissCheeseKnifeWidget(HolonicSystemsSwissCheeseKnifeModule *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HolonicSystems-SwissCheeseKnife.svg")));
		//screws
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		//channels
		for (int row=0; row<4; row++) {
			 
			
			//	IN	->	inv	->	*att	-> 	*VCA	->	LPF	->	SLEW	->	offset	->	OUT
			//	S/H -/		VCA	->	exp/lin	/
			
			int marginLeft = 15;
			int marginTop = 40;
			int spacingH = 32;
			int spacingV = 80;
			int col=0;
			//IN
			addInput(createInput<PJ301MPort>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV), module, HolonicSystemsSwissCheeseKnifeModule::INPUT_1+row));
			//S/H
			addInput(createInput<PJ301MPort>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV + 30), module, HolonicSystemsSwissCheeseKnifeModule::INPUT_SH_1+row));
			
			col = 1;
			//INV button
			addParam(createParam<CKSS>(Vec(marginLeft + col*spacingH-2, marginTop + row*spacingV+3), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_INV_1+row));
			//VCA IN
			addInput(createInput<PJ301MPort>(Vec(marginLeft + col*spacingH-7, marginTop + row*spacingV + 30 + 17), module, HolonicSystemsSwissCheeseKnifeModule::INPUT_VCA_1+row));

			col = 2;
			//ATT knob
			addParam(createParam<RoundSmallBlackKnob>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_ATT_1+row));
			//VCA response knob
			addParam(createParam<Trimpot>(Vec(marginLeft + col*spacingH-6, marginTop + row*spacingV + 31), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_VCA_RESPONSE_1+row));
			
			col = 3;
			//LPF knob
			addParam(createParam<RoundSmallBlackKnob>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_LPF_1+row));

			col = 4;
			//Slew UP
			addParam(createParam<Trimpot>(Vec(marginLeft + col*spacingH+5, marginTop + row*spacingV), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_SLEW_UP_1+row));
			//Slew Down
			addParam(createParam<Trimpot>(Vec(marginLeft + col*spacingH+5, marginTop + row*spacingV + 20), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_SLEW_DOWN_1+row));
			//Slew curve
			addParam(createParam<Trimpot>(Vec(marginLeft + col*spacingH+5, marginTop + row*spacingV + 40), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_SLEW_SHAPE_1+row));


			col = 5;
			//OFFSET knob
			addParam(createParam<RoundSmallBlackKnob>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_OFFSET_1+row));
			//AC_DC
			addParam(createParam<CKSS>(Vec(marginLeft + col*spacingH+4, marginTop + row*spacingV+45), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_AC_DC_1+row));

			col = 6;
			//Out
			addOutput(createOutput<PJ301MPort>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV), module, HolonicSystemsSwissCheeseKnifeModule::OUTPUT_1+row));
		}
	}

};


Model *modelSwissCheeseKnife = 
	createModel<HolonicSystemsSwissCheeseKnifeModule, HolonicSystemsSwissCheeseKnifeWidget>("HolonicSystems-SwissCheeseKnife");
