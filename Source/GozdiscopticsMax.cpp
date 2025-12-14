#include "GozdiscopticsMax.h"

std::shared_ptr<Window> gozdiscoptics::context = nullptr;

void gozdiscoptics::init(const std::vector<std::filesystem::path>& shader_paths) {
	WindowDescription desc;
	desc.w_resolution = glm::ivec2(1024);
	desc.w_name = "Gozdiscoptics Max";
	desc.w_scale_framebuffer_size = false;
	desc.w_scale_window_size = false;
	desc.w_visible = false;

	context = std::make_shared<Window>(desc);

	std::filesystem::path executable_directory = gozdiscoptics::get_executable_path_windows().parent_path();

	std::filesystem::path hbv_filename = "gostiscoptics.hbv";
	Package p;

	for (const std::filesystem::path& path : shader_paths)
		p.add(path);

	p.set_default_obfuscation();
	p.save_to_disk(executable_directory / hbv_filename);

	if (!Package::load_package(executable_directory / hbv_filename)) {
		std::cout << "[Gozdiscoptics Error] gozdiscoptics.hbv couldn't found" << std::endl;
		ASSERT(false);
	}
	
	//if (verbose) {
		Package::loaded_package->print_headers();
	//}
}

void gozdiscoptics::init_from_package(bool verbose) {
	
	WindowDescription desc;
	desc.w_resolution = glm::ivec2(1024);
	desc.w_name = "Gozdiscoptics Max";
	desc.w_scale_framebuffer_size = false;
	desc.w_scale_window_size = false;
	desc.w_visible = false;

	context = std::make_shared<Window>(desc);

	std::filesystem::path executable_directory = gozdiscoptics::get_executable_path_windows().parent_path();

	if (!Package::load_package(executable_directory / "gozdiscoptics.hbv")) {
		std::cout << "[Gozdiscoptics Error] gozdiscoptics.hbv couldn't found" << std::endl;
		ASSERT(false);
	}
	if (verbose) {
		Package::loaded_package->print_headers();
	}

	//shader_directory::fbp_shader_directory = "CTReconstructor/Source/GLSL/Reconstruction/FBP/";
	//shader_directory::ffft_shader_directory = "CTReconstructor/Source/GLSL/Reconstruction/FFT/";
	//shader_directory::util_shader_directory = "CTReconstructor/Source/GLSL/Reconstruction/Util/";
	//shader_directory::graph_rendering_shader_directory = "CTReconstructor/Source/GLSL/GraphRendering/";
	//shader_directory::texture_arithmatic_shader_directory = "GraphicsCortex/Source/GLSL/TextureArithmatic/";

	// temp
	//graphics_operation_shader_path = "GraphicsCortex/Source/GLSL/TextureArithmatic/graphics_operation.comp";
}

void gozdiscoptics::release() {
	context = nullptr;
}

bool gozdiscoptics::launch_realtime_window(FDTD& solver, const char* window_name)
{
	if (context == nullptr) {
		std::cout << "[Gozdiscoptics Error] gozdiscoptics::launch_realtime_window() is called but gozdiscoptics wasn't properly initialized. Call gozdiscoptics::init() function before calling this function" << std::endl;
		ASSERT(false);
	}

	bool old_visibility = context->is_window_visible();

	context->set_window_visibility(true);

	while (!gozdiscoptics::context->should_close()) {
		gozdiscoptics::context->handle_events(true);

		primitive_renderer::clear(0, 0, 0, 1);

		gozdiscoptics::context->swap_buffers();
	}
	
	context->set_window_visibility(old_visibility);

	return true;
}


