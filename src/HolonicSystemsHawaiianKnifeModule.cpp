#include "HolonicSystems-Free.hpp"
#include "dsp/digital.hpp"

struct HolonicSystemsHawaiianKnifeModule : Module {

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
	
	HolonicSystemsHawaiianKnifeModule();
	~HolonicSystemsHawaiianKnifeModule();
	
	void step() override;
	
	void onReset() override {
	}

	SchmittTrigger sampleAndHoldTrigger[NUM_OUTPUTS];
	float in[NUM_OUTPUTS];
	float preLPF[NUM_OUTPUTS];
	float preSlew[NUM_OUTPUTS];
	float postSlew[NUM_OUTPUTS];
	float out[NUM_OUTPUTS];
	double avg[NUM_OUTPUTS];

};


HolonicSystemsHawaiianKnifeModule::HolonicSystemsHawaiianKnifeModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	onReset();
}


HolonicSystemsHawaiianKnifeModule::~HolonicSystemsHawaiianKnifeModule() {
}


void HolonicSystemsHawaiianKnifeModule::step() {
		
	//compute channels
	for (int i=0; i<NUM_OUTPUTS; i++) {
		
		// input
		if (!inputs[INPUT_1+i].active) {
			inputs[INPUT_1+i].value = 0;
		}

		//sample and hold
		float att = params[PARAM_ATT_1+i].value;
		//TODO: label invert on UI is wrong
		float inv = params[PARAM_INV_1+i].value>=0.5?-1:1;
		bool sample = !inputs[INPUT_SH_1+i].active || sampleAndHoldTrigger[i].process(inputs[INPUT_SH_1+i].value);
		if (sample){
			in[i] = inputs[INPUT_1+i].value * inv * att;
		}
		
		
		//VCA
		//TODO: implement VCA response curve
		//float vca_response = params[PARAM_VCA_RESPONSE_1+i].value;
		float vca = inputs[INPUT_VCA_1+i].active?inputs[INPUT_VCA_1+i].value/5.0:1;
		//float linear = fmaxf(inputs[CV1_INPUT + i].value / 5.0, 0.0);
		//linear = clamp(linear, 0.0f, 2.0f);
		//const float base = 200.0;
		//float exponential = rescale(powf(base, linear / 2.0f), 1.0f, base, 0.0f, 10.0f);
		//in *= crossfade(exponential, linear, params[RESPONSE1_PARAM + i].value);

		preLPF[i] =  in[i] * inv * att * vca;
		
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
		//TODO: implement slew
		//out[i] = slew(preSlew[i], out[i]);
		postSlew[i] = preSlew[i];


		//offset
		float offset = params[PARAM_OFFSET_1+i].value;

		out[i] = postSlew[i] + offset;
		
		

		//remove dc offset
		//TODO: label DC offset is wrong on UI
		double alpha = 0.00001;
		avg[i] = alpha * ((double)out[i]) + (1-alpha) * avg[i];
		if (params[PARAM_AC_DC_1+i].value>0.4){
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


struct HolonicSystemsHawaiianKnifeWidget : ModuleWidget {

	HolonicSystemsHawaiianKnifeWidget(HolonicSystemsHawaiianKnifeModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/HolonicSystems-HawaiianKnife.svg")));
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
			addInput(Port::create<PJ301MPort>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV), Port::INPUT, module, HolonicSystemsHawaiianKnifeModule::INPUT_1+row));
			//S/H
			addInput(Port::create<PJ301MPort>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV + 30), Port::INPUT, module, HolonicSystemsHawaiianKnifeModule::INPUT_SH_1+row));
			
			col = 1;
			//INV button
			addParam(ParamWidget::create<CKSS>(Vec(marginLeft + col*spacingH-2, marginTop + row*spacingV+3), module, HolonicSystemsHawaiianKnifeModule::PARAM_INV_1+row, 0.0, 1.0, 1.0));
			//VCA IN
			addInput(Port::create<PJ301MPort>(Vec(marginLeft + col*spacingH-7, marginTop + row*spacingV + 30 + 17), Port::INPUT, module, HolonicSystemsHawaiianKnifeModule::INPUT_VCA_1+row));

			col = 2;
			//ATT knob
			addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV), module, HolonicSystemsHawaiianKnifeModule::PARAM_ATT_1+row, 0, 1.0, 1.0));
			//VCA response knob
			addParam(ParamWidget::create<Trimpot>(Vec(marginLeft + col*spacingH-6, marginTop + row*spacingV + 31), module, HolonicSystemsHawaiianKnifeModule::PARAM_VCA_RESPONSE_1+row, 0, 1.0, 1.0));
			
			col = 3;
			//LPF knob
			addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV), module, HolonicSystemsHawaiianKnifeModule::PARAM_LPF_1+row, 1.0, 0.0, 1.0));

			col = 4;
			//Slew UP
			addParam(ParamWidget::create<Trimpot>(Vec(marginLeft + col*spacingH+5, marginTop + row*spacingV), module, HolonicSystemsHawaiianKnifeModule::PARAM_SLEW_UP_1+row, 0, 1.0, 1.0));
			//Slew Down
			addParam(ParamWidget::create<Trimpot>(Vec(marginLeft + col*spacingH+5, marginTop + row*spacingV + 30), module, HolonicSystemsHawaiianKnifeModule::PARAM_SLEW_DOWN_1+row, 0, 1.0, 1.0));

			col = 5;
			//OFFSET knob
			addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV), module, HolonicSystemsHawaiianKnifeModule::PARAM_OFFSET_1+row, -5.0, 5.0, 0.0));
			//AC_DC
			addParam(ParamWidget::create<CKSS>(Vec(marginLeft + col*spacingH+4, marginTop + row*spacingV+30), module, HolonicSystemsHawaiianKnifeModule::PARAM_AC_DC_1+row, 0.0, 1.0, 1.0));

			col = 6;
			//Out
			addOutput(Port::create<PJ301MPort>(Vec(marginLeft + col*spacingH, marginTop + row*spacingV), Port::OUTPUT, module, HolonicSystemsHawaiianKnifeModule::OUTPUT_1+row));
		}
	}

};


Model *modelHolonicSystemsHawaiianKnife = 
	Model::create<HolonicSystemsHawaiianKnifeModule, HolonicSystemsHawaiianKnifeWidget>(
		"Holonic Systems",
	 	"HolonicSystems-HawaiianKnife", 
		"Hawaiian Knife",
		UTILITY_TAG
);
