#include "HolonicSystems-Free.hpp"

Plugin *pluginInstance;

void init(Plugin *p) {
	pluginInstance = p;
	p->addModel(modelHolonicSource);
	p->addModel(modelSwissCheeseKnife);
	p->addModel(modelPantry);
	p->addModel(modelGaps);
	p->addModel(modelJunctions);
	p->addModel(modelDumbwaiter);
	p->addModel(modelLazySusan);
}
