#include "HolonicSystems-Free.hpp"

Plugin *plugin;

void init(Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);
	p->addModel(modelHolonicSystemsHolonicSource);
	p->addModel(modelHolonicSystemsSwissCheeseKnife);
	p->addModel(modelHolonicSystemsPantry);
	p->addModel(modelHolonicSystemsGaps);
	p->addModel(modelHolonicSystemsJunctions);
	p->addModel(modelHolonicSystemsDumbwaiter);
	p->addModel(modelHolonicSystemsLazySusan);
}
