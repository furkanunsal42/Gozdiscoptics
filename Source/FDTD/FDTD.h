#pragma once

#include <memory>
#include <stdint.h>
#include <functional>
#include <filesystem>
#include <string>

//#include "LBMConstants/VelocitySet.h"
//#include "LBMConstants/FloatingPointAccuracy.h"
//#include "Tools/GraphicsOperation/GraphicsOperation.h"

#include "ComputeProgram.h"
#include "Mesh.h"
#include "Camera.h"

class FDTD {
public:

	constexpr static uint32_t not_a_boundry = 0;
	constexpr static uint32_t max_boundry_count = 255;
	constexpr static float referance_permittivity = 1.0;
	constexpr static float referance_permeability = 1.0;

	enum FloatingPointAccuracy {
		fp16 = 0,
		fp32 = 1,
	};

	int32_t get_FloatingPointAccuracy_size_in_bytes(FloatingPointAccuracy floating_accuracy);
	std::string get_FloatingPointAccuracy_to_macro(FloatingPointAccuracy floating_accuracy);
	
	enum FDTDGrid {
		Yee2D,
		Yee3D
	};

	int32_t get_FDTDGrid_size_in_bytes(FDTDGrid FDTDGrid);
	std::string get_FDTDGrid_to_macro(FDTDGrid FDTDGrid);

	// simulation controls
	void iterate_time(float target_tick_per_second = 0);
	int32_t get_total_ticks_elapsed();
	std::chrono::duration<double, std::milli> get_total_time_elapsed();
	glm::ivec3 get_resolution();
	int32_t get_velocity_set_vector_count();

	// high level field initialization api
	struct ElectroMagneticProperties {
		glm::vec3 electric_field = glm::vec3(0);
		glm::vec3 magnetic_field = glm::vec3(0);
		float electric_charge = 0;
		float magnetic_charge = 0;
		uint32_t boundry_id = not_a_boundry;
	};
	void initialize_fields(
		std::function<void(glm::ivec3, ElectroMagneticProperties&)> initialization_lambda,
		glm::ivec3 resolution,
		float relaxation_time,
		bool periodic_x = true,
		bool periodic_y = true,
		bool periodic_z = true,
		FDTDGrid velocity_set = FDTDGrid::Yee2D,
		FloatingPointAccuracy fp_accuracy = FloatingPointAccuracy::fp32
	);

	void set_boundry_properties(
		uint32_t boundry_id,
		float permittivity = referance_permittivity,
		float permeability = referance_permeability
	);

	void clear_boundry_properties();

	// high level visualization api
	void render2d_electric();
	void render2d_magnetic();
	void render2d_electromagnetic();
	void render2d_permittivity();
	void render2d_permeability();
	void render2d_reflection_index();
	void render2d_optic();

	void render3d_electric(Camera& camera, int32_t sample_count = 128);
	void render3d_magnetic(Camera& camera, int32_t sample_count = 128);
	void render3d_electromagnetic(Camera& camera, int32_t sample_count = 128);
	void render3d_permittivity(Camera& camera, int32_t sample_count = 128);
	void render3d_permeability(Camera& camera, int32_t sample_count = 128);
	void render3d_reflection_index(Camera& camera, int32_t sample_count = 128);
	void render3d_optic(Camera& camera, int32_t sample_count = 128);

	std::shared_ptr<Texture3D> get_magnetic_field_texture();
	std::shared_ptr<Texture3D> get_electric_field_texture();
	std::shared_ptr<Texture3D> get_permitivity_permeability_texture();

	// low level visualization api
	//void copy_to_texture_population(Texture2D& target_texture, int32_t population_index);

	// low level field initialization api
	FDTDGrid get_fdtd_grid();
	int32_t get_dimentionality();
	FloatingPointAccuracy get_floating_point_accuracy();
	bool get_periodic_boundry_x();
	bool get_periodic_boundry_y();
	bool get_periodic_boundry_z();
	
	// high level visualization api
	void save_current_tick_macroscopic(std::filesystem::path save_path);
	void save_current_tick_mesoscropic(std::filesystem::path save_path);
	void load_tick_macroscopic(std::filesystem::path save_path, int32_t target_tick);
	void load_tick_mesoscropic(std::filesystem::path save_path, int32_t target_tick);

private:

	std::vector<std::pair<std::string, std::string>> _generate_shader_macros();
	void _compile_shaders();
	void _generate_grid(glm::ivec3 resolution);

	size_t _coord_to_id(glm::uvec3 coord);
	size_t _coord_to_id(uint32_t x, uint32_t y, uint32_t z);
	size_t _get_voxel_count();

	void _propagate_electric(bool save_macrsoscopic_results);
	void _propagate_magnetic(bool save_macrsoscopic_results);
	void _generate_grid_buffers();

	// initialization functions
	void _set_electromagnetic_fields_to_equilibrium(Buffer& electric_field, Buffer& magnetic_field);

	void _initialize_fields_default_pass(
		std::function<void(glm::ivec3, ElectroMagneticProperties&)> initialization_lambda,
		std::shared_ptr<Buffer>& out_electric_field,
		std::shared_ptr<Buffer>& out_magnetic_field
	);
	void _initialize_fields_boundries_pass(
		std::function<void(glm::ivec3, ElectroMagneticProperties&)> initialization_lambda
	);

	// simulation time controls
	bool first_iteration = true;
	size_t total_ticks_elapsed = 0;
	std::chrono::time_point<std::chrono::system_clock> simulation_begin;
	std::chrono::time_point<std::chrono::system_clock> last_visual_update;

	// LBM simulation parameters
	FDTDGrid fdtd_grid = Yee2D;
	FloatingPointAccuracy floating_point_accuracy = fp32;

	glm::ivec3 resolution = glm::ivec3(0);

	void _set_fdtd_grid(FDTDGrid fdtd_grid);
	void _set_floating_point_accuracy(FloatingPointAccuracy floating_point_accuracy);

	// forces control flags
	bool is_forcing_scheme = false;
	bool is_force_field_constant = true;
	glm::vec3 constant_force = glm::vec3(0);
	void _set_is_forcing_scheme(bool value);
	void _set_is_force_field_constant(bool value);

	// moving/stationary boundries control flags
	bool periodic_x = true;
	bool periodic_y = true;
	bool periodic_z = true;
	void _set_periodic_boundry_x(bool value);
	void _set_periodic_boundry_y(bool value);
	void _set_periodic_boundry_z(bool value);

	struct _object_desc {
	public:
		_object_desc(
			glm::vec3 velocity_translational = glm::vec3(0),
			glm::vec3 velocity_angular = glm::vec3(0),
			glm::vec3 center_of_mass = glm::vec3(0),
			float temperature = referance_temperature,
			float effective_density = referance_boundry_density
		);

		glm::vec3 velocity_translational;
		glm::vec3 velocity_angular;
		glm::vec3 center_of_mass;
		float temperature;
		float effective_density;
	};

	// boundries buffer holds the id of the object it is a part of (0 means not_a_boundry)
	// number of bits per voxel can change dynamically basad on how many objects are defined
	// velocity information of each object is held in another buffer in device called "objects"
	// objects buffer schema is [vec4 translational_velcoity, vec4 rotational_velocity, vec4 center_of_mass] 
	std::vector<_object_desc> objects_cpu;
	int32_t bits_per_boundry = 0;
	void _set_bits_per_boundry(int32_t value);
	int32_t _get_bits_per_boundry(int32_t value);

	// thermal flow control flags
	bool is_flow_thermal = false;
	SimplifiedVelocitySet thermal_lattice_velocity_set = SimplifiedVelocitySet::D2Q5;
	float thermal_relaxation_time = 0.53;
	float thermal_expension_coeficient = 0.5f;

	void _set_is_flow_thermal(bool value);
	bool _get_is_flow_thermal();

	void _set_thermal_lattice_velocity_set(SimplifiedVelocitySet set);
	SimplifiedVelocitySet _get_thermal_lattice_velocity_set();

	// thermal flow physics
	void _stream_thermal();
	void _set_populations_to_equilibrium_thermal(Buffer& temperature_field, Buffer& velocity_field);

	// multiphase flow control flags
	bool is_flow_multiphase = true;
	float intermolecular_interaction_strength = -6.0f;
	void _set_is_flow_multiphase(bool value);

	// device buffers
	bool is_collide_esoteric = false;
	bool is_lattice_texture3d = false;
	Texture3D::ColorTextureFormat lattice_tex_internal_format = Texture3D::ColorTextureFormat::R16F;

	std::shared_ptr<Texture3D> lattice0_tex = nullptr;
	std::shared_ptr<Texture3D> lattice1_tex = nullptr;
	std::shared_ptr<Buffer> lattice0 = nullptr;
	std::shared_ptr<Buffer> lattice1 = nullptr;
	std::shared_ptr<Buffer> boundries = nullptr;
	std::shared_ptr<Buffer> objects = nullptr;
	std::shared_ptr<Buffer> forces = nullptr;
	std::shared_ptr<Buffer> thermal_lattice0 = nullptr;
	std::shared_ptr<Buffer> thermal_lattice1 = nullptr;

	std::unique_ptr<UniformBuffer> lattice_velocity_set_buffer = nullptr;
	std::unique_ptr<UniformBuffer> thermal_lattice_velocity_set_buffer = nullptr;

	// dual buffer control
	bool is_lattice_0_is_source = true;
	std::shared_ptr<Buffer> _get_lattice_source();
	std::shared_ptr<Buffer> _get_lattice_target();
	std::shared_ptr<Texture3D> _get_lattice_tex_source();
	std::shared_ptr<Texture3D> _get_lattice_tex_target();
	void _swap_lattice_buffers();

	bool is_thermal_lattice_0_is_source = true;
	std::shared_ptr<Buffer> _get_thermal_lattice_source();
	std::shared_ptr<Buffer> _get_thermal_lattice_target();
	void _swap_thermal_lattice_buffers();

	// macroscopic variable textures
	std::shared_ptr<Texture3D> velocity_density_texture = nullptr;
	std::shared_ptr<Texture3D> boundry_texture = nullptr;
	std::shared_ptr<Texture3D> force_temperature_texture = nullptr;
	void _generate_macroscopic_textures();

	Texture3D::ColorTextureFormat velocity_density_texture_internal_format = Texture3D::ColorTextureFormat::RGBA32F;
	Texture3D::ColorTextureFormat boundry_texture_internal_format = Texture3D::ColorTextureFormat::R8;
	Texture3D::ColorTextureFormat force_temperature_texture_internal_format = Texture3D::ColorTextureFormat::RGBA32F;

	// kernels
	bool is_programs_compiled = false;
	std::shared_ptr<ComputeProgram> lbm_stream = nullptr;
	std::shared_ptr<ComputeProgram> lbm_stream_thermal = nullptr;
	std::shared_ptr<ComputeProgram> lbm_collide = nullptr;
	std::shared_ptr<ComputeProgram> lbm_collide_save = nullptr;
	std::shared_ptr<ComputeProgram> lbm_collide_with_precomputed_velocity = nullptr;
	std::shared_ptr<ComputeProgram> lbm_set_equilibrium_populations = nullptr;
	std::shared_ptr<ComputeProgram> lbm_set_equilibrium_populations_thermal = nullptr;
	std::shared_ptr<ComputeProgram> lbm_set_population = nullptr;
	std::shared_ptr<ComputeProgram> lbm_add_random_population = nullptr;
	std::shared_ptr<ComputeProgram> lbm_copy_population = nullptr;

	// renderers
	std::shared_ptr<Program> program_render2d_density = nullptr;
	std::shared_ptr<Program> program_render2d_velocity = nullptr;
	std::shared_ptr<Program> program_render2d_boundries = nullptr;
	std::shared_ptr<Program> program_render2d_forces = nullptr;
	std::shared_ptr<Program> program_render2d_temperature = nullptr;

	std::shared_ptr<Program> program_render_volumetric_density = nullptr;
	std::shared_ptr<Program> program_render_volumetric_velocity = nullptr;
	std::shared_ptr<Program> program_render_volumetric_boundries = nullptr;
	std::shared_ptr<Program> program_render_volumetric_forces = nullptr;
	std::shared_ptr<Program> program_render_volumetric_temperature = nullptr;

	std::shared_ptr<Mesh> plane_mesh = nullptr;
	std::shared_ptr<Mesh> plane_cube = nullptr;

};