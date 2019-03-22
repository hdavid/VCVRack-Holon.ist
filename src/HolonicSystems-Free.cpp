#include "HolonicSystems-Free.hpp"

Plugin *pluginInstance;

void init(Plugin *p) {
	pluginInstance = p;
	p->addModel(modelHolonicSystemsHolonicSource);
	p->addModel(modelHolonicSystemsSwissCheeseKnife);
	p->addModel(modelHolonicSystemsPantry);
	p->addModel(modelHolonicSystemsGaps);
	p->addModel(modelHolonicSystemsJunctions);
	p->addModel(modelHolonicSystemsSequence);
	p->addModel(modelHolonicSystemsQuantiser);
}
