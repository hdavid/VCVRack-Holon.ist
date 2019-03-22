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
	
	HolonicSystemsSwissCheeseKnifeModule();
	~HolonicSystemsSwissCheeseKnifeModule();
	
	void step() override;
	
	void onReset() override {
	}

	LooseSchmittTrigger sampleAndHoldTrigger[NUM_OUTPUTS];
	float in[NUM_OUTPUTS];
	float preLPF[NUM_OUTPUTS];
	float preSlew[NUM_OUTPUTS];
	float postSlew[NUM_OUTPUTS];
	float out[NUM_OUTPUTS];
	double avg[NUM_OUTPUTS];

};


HolonicSystemsSwissCheeseKnifeModule::HolonicSystemsSwissCheeseKnifeModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	onReset();
}


HolonicSystemsSwissCheeseKnifeModule::~HolonicSystemsSwissCheeseKnifeModule() {
}


void HolonicSystemsSwissCheeseKnifeModule::step() {
		
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
			lpf = (lpf * lpf * lpf * lpf) * engineGetSampleTime() * 5000;
			preSlew[i] = preLPF[i] * (lpf) + preSlew[i] * (1 - lpf);
		} else {
			//ignore filtering at low values
			preSlew[i] = preLPF[i];
		}
		

		//slew
		if (params[PARAM_SLEW_UP_1+i].value>0.01 || params[PARAM_SLEW_DOWN_1+i].value>0.01) {
			float shape = params[PARAM_SLEW_SHAPE_1+i].value;
			// minimum and maximum slopes in volts per second
			const float slewMin = 0.1;
			const float slewMax = 10000.0;
			// Amount of extra slew per voltage difference
			const float shapeScale = 1/10.0;
			// Rise
			if (preSlew[i] > postSlew[i]) {
				float rise = params[PARAM_SLEW_UP_1+i].value;
				float slew = slewMax * powf(slewMin / slewMax, rise);
				postSlew[i] += slew * crossfade(1.0f, shapeScale * (preSlew[i] - postSlew[i]), shape) * engineGetSampleTime();
				if (postSlew[i] > preSlew[i]) {
					postSlew[i]  = preSlew[i];
				}
			} else if (preSlew[i] < postSlew[i]) {
				// Fall
				float fall = params[PARAM_SLEW_DOWN_1+i].value;
				float slew = slewMax * powf(slewMin / slewMax, fall);
				postSlew[i] -= slew * crossfade(1.0f, shapeScale * (postSlew[i] - preSlew[i]), shape) * engineGetSampleTime();
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


struct HolonicSystemsSwissCheeseKnifeWidget : ModuleWidget {

	HolonicSystemsSwissCheeseKnifeWidget(HolonicSystemsSwissCheeseKnifeModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/HolonicSystems-SwissCheeseKnife.svg")));
		//screws
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
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
			addInput(Port::create<PJ301MPort>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV), Port::INPUT, module, HolonicSystemsSwissCheeseKnifeModule::INPUT_1+row));
			//S/H
			addInput(Port::create<PJ301MPort>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV + 30), Port::INPUT, module, HolonicSystemsSwissCheeseKnifeModule::INPUT_SH_1+row));
			
			col = 1;
			//INV button
			addParam(ParamWidget::create<CKSS>(Vec(marginLeft + col*spacingH-2, marginTop + row*spacingV+3), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_INV_1+row, 0.0, 1.0, 1.0));
			//VCA IN
			addInput(Port::create<PJ301MPort>(Vec(marginLeft + col*spacingH-7, marginTop + row*spacingV + 30 + 17), Port::INPUT, module, HolonicSystemsSwissCheeseKnifeModule::INPUT_VCA_1+row));

			col = 2;
			//ATT knob
			addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_ATT_1+row, 0, 1.0, 1.0));
			//VCA response knob
			addParam(ParamWidget::create<Trimpot>(Vec(marginLeft + col*spacingH-6, marginTop + row*spacingV + 31), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_VCA_RESPONSE_1+row, 0, 1.0, 0.5));
			
			col = 3;
			//LPF knob
			addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_LPF_1+row, 1.0, 0.0, 1.0));

			col = 4;
			//Slew UP
			addParam(ParamWidget::create<Trimpot>(Vec(marginLeft + col*spacingH+5, marginTop + row*spacingV), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_SLEW_UP_1+row, 0, 1.0, 0.0));
			//Slew Down
			addParam(ParamWidget::create<Trimpot>(Vec(marginLeft + col*spacingH+5, marginTop + row*spacingV + 20), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_SLEW_DOWN_1+row, 0, 1.0, 0.0));
			//Slew curve
			addParam(ParamWidget::create<Trimpot>(Vec(marginLeft + col*spacingH+5, marginTop + row*spacingV + 40), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_SLEW_SHAPE_1+row, 0, 1.0, 0.5));


			col = 5;
			//OFFSET knob
			addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_OFFSET_1+row, -5.0, 5.0, 0.0));
			//AC_DC
			addParam(ParamWidget::create<CKSS>(Vec(marginLeft + col*spacingH+4, marginTop + row*spacingV+45), module, HolonicSystemsSwissCheeseKnifeModule::PARAM_AC_DC_1+row, 0.0, 1.0, 1.0));

			col = 6;
			//Out
			addOutput(Port::create<PJ301MPort>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV), Port::OUTPUT, module, HolonicSystemsSwissCheeseKnifeModule::OUTPUT_1+row));
		}
	}

};


Model *modelHolonicSystemsSwissCheeseKnife = 
	Model::create<HolonicSystemsSwissCheeseKnifeModule, HolonicSystemsSwissCheeseKnifeWidget>(
		"Holonic Systems",
	 	"HolonicSystems-SwissCheeseKnife", 
		"SwissCheeseKnife",
		UTILITY_TAG,
		ATTENUATOR_TAG,
		MIXER_TAG,
		QUAD_TAG,
		SAMPLE_AND_HOLD_TAG,
		SLEW_LIMITER_TAG
);
