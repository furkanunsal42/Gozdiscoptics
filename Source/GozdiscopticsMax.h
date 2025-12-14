#pragma once

#include "GraphicsCortex.h"
#include "FDTD/FDTD.h"

#include <filesystem>
#include <memory>

namespace gozdiscoptics {

	void init_from_package(bool verbose = false);
	void init(
		const std::vector<std::filesystem::path>& shader_paths = 
		{
			"../GozdiscopticsMax/Source/GLSL/**.comp",
			"../GozdiscopticsMax/Source/GLSL/**.vert",
			"../GozdiscopticsMax/Source/GLSL/**.frags",
			"../GraphicsCortex/Source/GLSL/**.comp",
			"../GraphicsCortex/Source/GLSL/**.vert",
			"../GraphicsCortex/Source/GLSL/**.frag",	
		}
	);
	void release();

	bool launch_controls_window(FDTD& solver, const char* window_name = "Gozdiscoptics");
	bool launch_realtime_window(FDTD& solver, const char* window_name = "Gozdiscoptics");

	extern std::shared_ptr<Window> context;

	std::filesystem::path get_executable_path_windows();
}

#include "FDTD/FDTD.h"