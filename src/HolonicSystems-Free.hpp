#include "rack.hpp"
#include <vector>

using namespace rack;

struct HolonicSystemsLabel : Widget {
	std::string text = "xx";
	int fontSize;
	HolonicSystemsLabel(int _fontSize = 12) {
		fontSize = _fontSize;
		box.size.y = BND_WIDGET_HEIGHT;
	}
	void draw(NVGcontext *vg) override {
		//nvgTextAlign(vg, NVG_ALIGN_CENTER);
		nvgFillColor(vg, nvgRGB(0, 0, 0));
		nvgFontSize(vg, fontSize);
		nvgText(vg, box.pos.x, box.pos.y, text.c_str(), NULL);
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

	void onChange(EventChange &e) override {
		RoundSmallBlackKnob::onChange(e);
		if (linkedLabel) {
			linkedLabel->text = formatCurrentValue();
		}
	}

	
	HolonicSystemsKnob(){
		snap = true;
	}
	
	std::string formatCurrentValue() {
		return names[int(value)];
	}
};

extern Plugin *plugin;

extern Model *modelHolonicSystemsHolonist;
extern Model *modelHolonicSystemsSwissCheeseKnife;
extern Model *modelHolonicSystemsGaps;
extern Model *modelHolonicSystemsPast;
extern Model *modelHolonicSystemsJunctions;


