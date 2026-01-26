#include "Gozdiscoptics.h"
#include "FDTD/FDTD.h"

void point_source(FDTD::ElectroMagneticProperty& property, glm::ivec3 id, glm::ivec3 light_position, float frequency, float amplitude) {
	if (id == light_position) {
		property.voxel_type = FDTD::SourceSinosoidal;
		property.source_frequency = frequency;
		property.source_amplitude = amplitude;
	}
}

void plane_wave_source(FDTD::ElectroMagneticProperty& property, glm::ivec3 id, glm::ivec3 plane_begin, glm::ivec3 plane_size, float frequency, float amplitude) {
	if (glm::all(glm::greaterThanEqual(id - plane_begin, glm::ivec3(0))) && glm::all(glm::lessThan(id - plane_begin, plane_size))) {

		property.voxel_type = FDTD::SourceSinosoidal;
		property.source_frequency = frequency;
		property.source_amplitude = amplitude;

	}
}

bool in_range(float value, float min, float max) {
	return value >= min && value < max;
}

void main() {

	gozdiscoptics::init();

	FDTD solver;

	solver.initialzie_fields(
		[&](glm::ivec3 id, FDTD::ElectroMagneticProperty& property) {
			
			//point_source(property, id, glm::ivec3(512, 512, 0), 2.0 * glm::pi<float>() * 2e9, 0.02);
			plane_wave_source(property, id, glm::ivec3(100, 0, 0), glm::ivec3(1, 1024, 1), glm::pi<float>() * 1e10, 0.4);

			if (in_range(id.x, 400, 404) && !(in_range(id.y, 440, 490) || in_range(id.y, 510, 560)))
				property.voxel_type = FDTD::PEC;


		},
		glm::ivec3(1024, 1024, 1),
		glm::ivec2(40),
		glm::ivec2(40)
	);

	gozdiscoptics::context->set_window_visibility(true);
	while (!gozdiscoptics::context->should_close()) {

		gozdiscoptics::context->handle_events();
		primitive_renderer::clear(0, 0, 0, 1);
		
		solver.iterate_time(512);
		solver.render2d_electromagnetic();

		gozdiscoptics::context->swap_buffers();

	}

	gozdiscoptics::release();

}