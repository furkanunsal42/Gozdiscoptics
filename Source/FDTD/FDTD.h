#pragma once

#include "ComputeProgram.h"
#include <memory>

#include "Texture3D.h"
#include "VertexAttributeBuffer.h"

class FDTD {
public:

	enum VoxelType {
		Normal				= 0,
		PEC					= 1,
		SourceSinosoidal	= 2,
		SourceImpulse		= 3,
	};

	struct ElectroMagneticProperty {
		VoxelType voxel_type = Normal;
		float source_frequency = 1;
		float source_amplitude = 1;
		float source_phase = 0;
	};

	void initialzie_fields(
		std::function<void(glm::ivec3, ElectroMagneticProperty&)> initialization_lambda,
		glm::ivec3 grid_resolution,
		glm::ivec2 pml_thickness_x = glm::ivec2(10),
		glm::ivec2 pml_thickness_y = glm::ivec2(10),
		glm::ivec2 pml_thickness_z = glm::ivec2(10)
	);

	void iterate_time(float target_tick_per_second);

	void render2d_electromagnetic();

	int32_t get_total_ticks_elapsed();
	std::chrono::duration<double, std::milli> get_total_time_elapsed();

	std::shared_ptr<Texture3D>	electric_field_texture;
	std::shared_ptr<Texture3D>	magnetic_field_texture;
	std::shared_ptr<Texture3D>	property_field_texture;

private:

	void step();

	glm::ivec3 grid_resolution = glm::ivec3(0);
	glm::ivec2 pml_thickness_x = glm::ivec2(0);
	glm::ivec2 pml_thickness_y = glm::ivec2(0);
	glm::ivec2 pml_thickness_z = glm::ivec2(0);

	std::vector<std::pair<std::string, std::string>> generate_macros();
	void compile_shaders();
	void generate_textures();

	Texture3D::ColorTextureFormat electric_field_internal_format = Texture3D::ColorTextureFormat::RG32F;
	Texture3D::ColorTextureFormat magnetic_field_internal_format = Texture3D::ColorTextureFormat::RG32F;
	Texture3D::ColorTextureFormat property_field_internal_format = Texture3D::ColorTextureFormat::RGBA32F;

	int32_t tick = 0;
	std::chrono::time_point<std::chrono::system_clock> simulation_begin;

	std::shared_ptr<ComputeProgram> cp_magnetic_update;
	std::shared_ptr<ComputeProgram> cp_electric_update;

	std::shared_ptr<Mesh> plane_mesh;
	std::shared_ptr<Mesh> cube_mesh;
	
	std::shared_ptr<Program> program_render2d_electromagnetic;
};