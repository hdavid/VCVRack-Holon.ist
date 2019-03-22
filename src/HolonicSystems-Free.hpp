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
		//nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
		nvgFillColor(args.vg, nvgRGB(0, 0, 0));
		nvgFontSize(args.vg, fontSize);
		nvgText(args.vg, box.pos.x, box.pos.y, text.c_str(), NULL);
	}
};

struct HolonicSystemsKnob : RoundSmallBlackKnob {
	
 	std::vector<std::string> names; 
	
	HolonicSystemsLabel* linkedLabel = nullptr;
	
	void connectLabel(HolonicSystemsLabel* label) {
		linkedLabel = label;
		if (linkedLabel) {
			linkedLabel->text = formatCurrentValue();
		}
	}

	void onDragMove(const widget::DragMoveEvent &e) override {
		RoundSmallBlackKnob::onDragMove(e);
		if (linkedLabel) {
			linkedLabel->text = formatCurrentValue();
		}
	}
	void onDragEnd(const widget::DragEndEvent &e) override {
		RoundSmallBlackKnob::onDragEnd(e);
		if (linkedLabel) {
			linkedLabel->text = formatCurrentValue();
		}
	}

	
	HolonicSystemsKnob(){
		snap = true;
	}
	
	std::string formatCurrentValue() {
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

extern Model *modelHolonicSystemsHolonicSource;
extern Model *modelHolonicSystemsSwissCheeseKnife;
extern Model *modelHolonicSystemsGaps;
extern Model *modelHolonicSystemsPantry;
extern Model *modelHolonicSystemsJunctions;
extern Model *modelHolonicSystemsSequence;
extern Model *modelHolonicSystemsQuantiser;


