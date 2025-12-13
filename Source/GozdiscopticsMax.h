#pragma once

#include "GraphicsCortex.h"

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

	std::filesystem::path get_executable_path_windows();

	//bool launch_preview_window_back(FBP3D& solver, ParameterParser& parser, const char* window_name = "CTReconstructor");

	extern std::shared_ptr<Window> context;
}

#include "FDTD/FDTD.h"