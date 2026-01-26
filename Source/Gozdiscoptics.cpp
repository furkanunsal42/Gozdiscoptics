#include "Gozdiscoptics.h"
#include "Application/ProgramSourcePaths.h"

std::shared_ptr<Window> gozdiscoptics::context = nullptr;

void gozdiscoptics::init() {

	WindowDescription desc;
	desc.w_resolution = glm::ivec2(1024);
	desc.w_name = "CTReconstructor";
	desc.w_scale_framebuffer_size = false;
	desc.w_scale_window_size = false;
	desc.w_visible = false;

	context = std::make_shared<Window>(desc);

	//shader_directory::fdtd_shader_directory = kernels_parent_directory / "Reconstruction/FBP/";

}

void gozdiscoptics::release() {
	context = nullptr;
}