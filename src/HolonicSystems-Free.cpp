#include "HolonicSystems-Free.hpp"

Plugin *plugin;

void init(Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);
	p->addModel(modelHolonicSystemsHolonist);
	p->addModel(modelHolonicSystemsSwissCheeseKnife);
}
