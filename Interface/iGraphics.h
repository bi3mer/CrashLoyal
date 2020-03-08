#pragma once

#include "SDL_image.h"
#include "SDL_ttf.h"
#include "Singleton.h"

class iGraphics {

public: 

	virtual SDL_Renderer* getRenderer();

};