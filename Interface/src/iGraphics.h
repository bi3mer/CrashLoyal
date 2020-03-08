#pragma once

#include <vector>
#include "Singleton.h"
#include "EntityStats.h"
#include <assert.h>

class iGraphics : public Singleton<iGraphics> {

public:

	virtual void drawUIButtons(const std::vector<iEntityStats::MobType> mobsToDraw) {
		assert(false);
	}

};