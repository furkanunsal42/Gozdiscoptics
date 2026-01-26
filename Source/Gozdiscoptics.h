#pragma once
#include "GraphicsCortex.h"

#include <memory>
#include "Window.h"

namespace gozdiscoptics {

	void init();
	void release();

	extern std::shared_ptr<Window> context;
}