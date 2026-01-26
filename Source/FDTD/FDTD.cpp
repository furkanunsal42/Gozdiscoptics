#include "FDTD.h"
#include "Application/ProgramSourcePaths.h"
#include "PrimitiveRenderer.h"

void FDTD::initialzie_fields(
	std::function<void(glm::ivec3, ElectroMagneticProperty&)> initialization_lambda,
	glm::ivec3 grid_resolution,
	glm::ivec2 pml_thickness_x,
	glm::ivec2 pml_thickness_y,
	glm::ivec2 pml_thickness_z
) {

	this->grid_resolution = grid_resolution;
	this->pml_thickness_x = pml_thickness_x;
	this->pml_thickness_y = pml_thickness_y;
	this->pml_thickness_z = pml_thickness_z;

	generate_textures();

	electric_field_texture->clear(glm::vec4(0));
	magnetic_field_texture->clear(glm::vec4(0));

	std::vector<glm::vec4> property_buffer(grid_resolution.x * grid_resolution.y * grid_resolution.z, glm::vec4(0));

	for (int32_t z = 0; z < grid_resolution.z; z++){
		for (int32_t y = 0; y < grid_resolution.y; y++){
			for (int32_t x = 0; x < grid_resolution.x; x++){
	
				ElectroMagneticProperty property;
				initialization_lambda(glm::ivec3(x, y, z), property);

				glm::vec4 property_vec4;
				property_vec4.x = property.voxel_type;
				property_vec4.y = property.source_frequency;
				property_vec4.z = property.source_amplitude;
				property_vec4.w = property.source_phase;
				property_buffer[z * grid_resolution.y * grid_resolution.x + y * grid_resolution.x + x] = property_vec4;
			}
		}
	}

	property_field_texture->load_data((void*)property_buffer.data(), Texture3D::ColorFormat::RGBA, Texture3D::Type::FLOAT, 0);

	compile_shaders();

}

void FDTD::iterate_time(float target_tick_per_second)
{
	if (tick == 0) {
		simulation_begin = std::chrono::system_clock::now();
	}

	size_t targeted_tick_count = target_tick_per_second * get_total_time_elapsed().count() / 1000.0f;
	if (target_tick_per_second <= 0 || tick < targeted_tick_count || tick == 0) {

		step();

	}
}

void FDTD::step() {

	{
		ComputeProgram& kernel = *cp_magnetic_update;
	
		kernel.update_uniform_as_image("electric_texture", *electric_field_texture, 0);
		kernel.update_uniform_as_image("magnetic_texture", *magnetic_field_texture, 0);
		kernel.update_uniform_as_image("property_texture", *property_field_texture, 0);
	
		kernel.update_uniform("grid_resolution", grid_resolution);
	
		kernel.dispatch_thread(grid_resolution);
	}

	{
		ComputeProgram& kernel = *cp_electric_update;
	
		kernel.update_uniform_as_image("electric_texture", *electric_field_texture, 0);
		kernel.update_uniform_as_image("magnetic_texture", *magnetic_field_texture, 0);
		kernel.update_uniform_as_image("property_texture", *property_field_texture, 0);
		
		kernel.update_uniform("grid_resolution", grid_resolution);
		kernel.update_uniform("pml_thickness_x", pml_thickness_x);
		kernel.update_uniform("pml_thickness_y", pml_thickness_y);
		kernel.update_uniform("pml_thickness_z", pml_thickness_z);

		kernel.update_uniform("tick", tick);
	
		kernel.dispatch_thread(grid_resolution);
	}

	tick++;
}

int32_t FDTD::get_total_ticks_elapsed()
{
	return tick;
}

std::chrono::duration<double, std::milli> FDTD::get_total_time_elapsed()
{
	return std::chrono::system_clock::now() - simulation_begin;
}

void FDTD::render2d_electromagnetic()
{
	Program& program = *program_render2d_electromagnetic;

	program.update_uniform("electric_texture", *electric_field_texture);
	program.update_uniform("magnetic_texture", *magnetic_field_texture);
	program.update_uniform("property_texture", *property_field_texture);

	program.update_uniform("model", glm::identity<glm::mat4>());
	program.update_uniform("view", glm::identity<glm::mat4>());
	program.update_uniform("projection", glm::identity<glm::mat4>());
	program.update_uniform("texture_resolution", glm::vec3(electric_field_texture->get_size()));
	program.update_uniform("render_depth", 0);

	RenderParameters params(true);
	
	primitive_renderer::render(
		program,
		*plane_mesh->get_mesh(0),
		RenderParameters(),
		1,
		0
	);
}

std::vector<std::pair<std::string, std::string>> FDTD::generate_macros() {
	
	std::vector<std::pair<std::string, std::string>> definitions{
		{"fdtd_electric_internal_format",		Texture3D::ColorTextureFormat_to_OpenGL_compute_Image_format(electric_field_internal_format)},
		{"fdtd_magnetic_internal_format",		Texture3D::ColorTextureFormat_to_OpenGL_compute_Image_format(magnetic_field_internal_format)},
		{"fdtd_property_internal_format",		Texture3D::ColorTextureFormat_to_OpenGL_compute_Image_format(property_field_internal_format)},
		{"dimentionality",						grid_resolution.z == 1 ? "2" : "3"},
	};

	return definitions;
}

void FDTD::compile_shaders()
{
	std::vector<std::pair<std::string, std::string>> macros = generate_macros();

	cp_electric_update = std::make_shared<ComputeProgram>(Shader(shader_directory::fdtd_shader_directory / "electric_update.comp"), macros);
	cp_magnetic_update = std::make_shared<ComputeProgram>(Shader(shader_directory::fdtd_shader_directory / "magnetic_update.comp"), macros);

	program_render2d_electromagnetic = std::make_shared<Program>(Shader(shader_directory::renderer2d_shader_directory / "basic.vert", shader_directory::renderer2d_shader_directory / "electromagnetic_2d.frag"));

	SingleModel plane_model;
	plane_model.verticies = {
		glm::vec3(-1, -1, 0),
		glm::vec3(1, -1, 0),
		glm::vec3(-1,  1, 0),
		glm::vec3(1,  1, 0),
	};
	plane_model.texture_coordinates_0 = {
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		glm::vec2(0, 1),
		glm::vec2(1, 1),
	};
	plane_model.indicies = {
		0, 1, 2,
		2, 1, 3
	};

	plane_mesh = std::make_shared<Mesh>();
	plane_mesh->load_model(plane_model);

	glm::vec3 scale(1, 1, 1);

	SingleModel cube_model;
	cube_model.verticies = {
		glm::vec3(-0.5f * scale.x, -0.5f * scale.y,  0.5f * scale.z),//front
		glm::vec3(0.5f * scale.x, -0.5f * scale.y,  0.5f * scale.z),
		glm::vec3(0.5f * scale.x,  0.5f * scale.y,  0.5f * scale.z),
		glm::vec3(-0.5f * scale.x,  0.5f * scale.y,  0.5f * scale.z),

		glm::vec3(0.5f * scale.x, -0.5f * scale.y,  0.5f * scale.z),//right
		glm::vec3(0.5f * scale.x, -0.5f * scale.y, -0.5f * scale.z),
		glm::vec3(0.5f * scale.x,  0.5f * scale.y, -0.5f * scale.z),
		glm::vec3(0.5f * scale.x,  0.5f * scale.y,  0.5f * scale.z),

		glm::vec3(-0.5f * scale.x,  0.5f * scale.y, -0.5f * scale.z),//top
		glm::vec3(-0.5f * scale.x,  0.5f * scale.y,  0.5f * scale.z),
		glm::vec3(0.5f * scale.x,  0.5f * scale.y,  0.5f * scale.z),
		glm::vec3(0.5f * scale.x,  0.5f * scale.y, -0.5f * scale.z),

		glm::vec3(0.5f * scale.x, -0.5f * scale.y, -0.5f * scale.z),//back
		glm::vec3(-0.5f * scale.x, -0.5f * scale.y, -0.5f * scale.z),
		glm::vec3(-0.5f * scale.x,  0.5f * scale.y, -0.5f * scale.z),
		glm::vec3(0.5f * scale.x,  0.5f * scale.y, -0.5f * scale.z),

		glm::vec3(-0.5f * scale.x, -0.5f * scale.y, -0.5f * scale.z),//left
		glm::vec3(-0.5f * scale.x, -0.5f * scale.y,  0.5f * scale.z),
		glm::vec3(-0.5f * scale.x,  0.5f * scale.y,  0.5f * scale.z),
		glm::vec3(-0.5f * scale.x,  0.5f * scale.y, -0.5f * scale.z),

		glm::vec3(0.5f * scale.x,  -0.5f * scale.y,  0.5f * scale.z),//bottom
		glm::vec3(-0.5f * scale.x,  -0.5f * scale.y,  0.5f * scale.z),
		glm::vec3(-0.5f * scale.x,  -0.5f * scale.y, -0.5f * scale.z),
		glm::vec3(0.5f * scale.x,  -0.5f * scale.y, -0.5f * scale.z),
	};

	cube_model.texture_coordinates_0 = {
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(0.0f, 1.0f),

		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(0.0f, 1.0f),

		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(0.0f, 1.0f),

		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(0.0f, 1.0f),

		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(0.0f, 1.0f),

		glm::vec2(0.0f, 0.0f),
		glm::vec2(0.0f, 1.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(1.0f, 0.0f),
	};

	cube_model.vertex_normals = {
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),

		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),

		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),

		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),

		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),

		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
	};

	cube_model.indicies = {
			0, 1, 2, 0, 2, 3,
			4, 5, 6, 4, 6, 7,
			8, 9, 10, 8, 10, 11,
			12, 13, 14, 12, 14, 15,
			16, 17, 18, 16, 18, 19,
			20, 21, 22, 20, 22, 23,
	};

	cube_mesh = std::make_shared<Mesh>();
	cube_mesh->load_model(cube_model);
}

void FDTD::generate_textures() {

	if (glm::any(glm::lessThanEqual(grid_resolution, glm::ivec3(0)))) {
		std::cout << "[FDTD Error] FDTD::generate_textures() is called with invalid grid_resolution" << std::endl;
		ASSERT(false);
	}

	if (glm::any(glm::lessThanEqual(pml_thickness_x, glm::ivec2(0))) ||
		glm::any(glm::lessThanEqual(pml_thickness_y, glm::ivec2(0))) ||
		glm::any(glm::lessThanEqual(pml_thickness_y, glm::ivec2(0)))
	) {

		std::cout << "[FDTD Error] FDTD::generate_textures() is called with invalid pml_thickness" << std::endl;
		ASSERT(false);

	}

	electric_field_texture = std::make_shared<Texture3D>(
		grid_resolution.x, grid_resolution.y, grid_resolution.z,
		electric_field_internal_format, 1, 0
	);
	
	magnetic_field_texture = std::make_shared<Texture3D>(
		grid_resolution.x, grid_resolution.y, grid_resolution.z,
		magnetic_field_internal_format, 1, 0
	);

	property_field_texture = std::make_shared<Texture3D>(
		grid_resolution.x, grid_resolution.y, grid_resolution.z,
		property_field_internal_format, 1, 0
	);


}
