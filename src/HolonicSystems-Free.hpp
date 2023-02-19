#include "rack.hpp"
#include <vector>

using namespace rack;


struct LooseSchmittTrigger  {
	// UNKNOWN is used to represent a stable state when the previous state is not yet set
	enum State {
		UNKNOWN,
		LOW,
		HIGH
	};
	State state = UNKNOWN;


	bool process(float in) {
		switch (state) {
			case LOW:
				if (in >= 1.f) {
					state = HIGH;
					return true;
				}
				break;
			case HIGH:
				if (in <= 0.5f) {
					state = LOW;
				}
				break;
			default:
				if (in >= 1.f) {
					state = HIGH;
				}
				else if (in <= 0.5f) {
					state = LOW;
				}
				break;
		}
		return false;
	}
	
	bool isHigh() {
		return state == HIGH;
	}
	void reset() {
		state = UNKNOWN;
	}
	
};

struct HolonicSystemsLabel : Widget {
	std::string text = "xx";
	int fontSize;
	HolonicSystemsLabel(int _fontSize = 12) {
		fontSize = _fontSize;
		box.size.y = BND_WIDGET_HEIGHT;
	}
	void draw(const DrawArgs &args) override {
		nvgFillColor(args.vg, nvgRGB(0, 0, 0));
		nvgFontSize(args.vg, fontSize);
		nvgText(args.vg, box.pos.x, box.pos.y, text.c_str(), NULL);
	}
};
#ifdef MIRACK
struct HolonicSystemsKnob : V1_COMPAT_W<RoundSmallBlackKnob> {
#else
struct HolonicSystemsKnob : RoundSmallBlackKnob {
#endif
 	std::vector<std::string> names; 
	
	HolonicSystemsLabel* linkedLabel = nullptr;
	
	void connectLabel(HolonicSystemsLabel* label) {
		linkedLabel = label;
		if (linkedLabel) {
				linkedLabel->text = formatCurrentValue();
		}
	}
#ifdef MIRACK
	void onChange(rack::event::Change &e) override {
#else
	void onChange(const rack::event::Change &e) override {
#endif
		RoundSmallBlackKnob::onChange(e);
		if (linkedLabel) {
			linkedLabel->text = formatCurrentValue();
		}
	}

	HolonicSystemsKnob(){
		snap = true;
	}
	
	std::string formatCurrentValue() {
#ifndef MIRACK
		ParamQuantity* paramQuantity = getParamQuantity();
#endif
		int index = int(paramQuantity->getValue());
		int size =  (int)names.size();
		if (size>0 && index < size && index >= 0) {
			return names[index];
		} else {
			return "";
		}
	}
};

extern Plugin *pluginInstance;

extern Model *modelHolonicSource;
extern Model *modelSwissCheeseKnife;
extern Model *modelGaps;
extern Model *modelPantry;
extern Model *modelJunctions;
extern Model *modelDumbwaiter;
extern Model *modelLazySusan;


