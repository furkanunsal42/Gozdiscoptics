#pragma once

#include <memory>
#include <stdint.h>
#include <functional>
#include <filesystem>
#include <string>
#include <type_traits>

#include "ComputeProgram.h"
#include "Mesh.h"
#include "Camera.h"
#include "Texture3D.h"

#include "FDTDConstants.h"

class FDTD {
public:

	constexpr static uint32_t not_a_boundry = 0;
	constexpr static uint32_t max_boundry_count = 255;
	constexpr static float referance_permittivity = 1.0;
	constexpr static float referance_permeability = 1.0;

	enum BoundryCondition {
		PerfactlyMatched,
		Dirichlet,
		Periodic,
	};

	class SourceExcitation {
	public:
		template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
		SourceExcitation(const T& constant);
		SourceExcitation(const std::string& fdtd_excitation_glsl = "");
		SourceExcitation(std::string&& fdtd_excitation_glsl);
		SourceExcitation& operator=(const std::string& fdtd_excitation_glsl);
		SourceExcitation& operator=(std::string&& fdtd_excitation_glsl);

		friend std::ostream& operator<<(std::ostream& stream, const SourceExcitation& object);

		friend SourceExcitation operator+(const SourceExcitation& a, const SourceExcitation& b);
		friend SourceExcitation operator-(const SourceExcitation& a, const SourceExcitation& b);
		friend SourceExcitation operator/(const SourceExcitation& a, const SourceExcitation& b);
		friend SourceExcitation operator*(const SourceExcitation& a, const SourceExcitation& b);

		template<typename T, std::enable_if_t<std::is_arithmetic_v<T>>>
		friend SourceExcitation operator+(const SourceExcitation& a, const T& b);
		template<typename T, std::enable_if_t<std::is_arithmetic_v<T>>>
		friend SourceExcitation operator-(const SourceExcitation& a, const T& b);
		template<typename T, std::enable_if_t<std::is_arithmetic_v<T>>>
		friend SourceExcitation operator/(const SourceExcitation& a, const T& b);
		template<typename T, std::enable_if_t<std::is_arithmetic_v<T>>>
		friend SourceExcitation operator*(const SourceExcitation& a, const T& b);

		template<typename T, std::enable_if_t<std::is_arithmetic_v<T>>>
		friend SourceExcitation operator+(const T& a, const SourceExcitation& b);
		template<typename T, std::enable_if_t<std::is_arithmetic_v<T>>>
		friend SourceExcitation operator-(const T& a, const SourceExcitation& b);
		template<typename T, std::enable_if_t<std::is_arithmetic_v<T>>>
		friend SourceExcitation operator/(const T& a, const SourceExcitation& b);
		template<typename T, std::enable_if_t<std::is_arithmetic_v<T>>>
		friend SourceExcitation operator*(const T& a, const SourceExcitation& b);

	private:
		std::string fdtd_excitation_glsl;
	};

	static SourceExcitation impulse(float t);
	static SourceExcitation window(float begin, float end);
	static SourceExcitation sin(SourceExcitation signal);
	static SourceExcitation cos(SourceExcitation signal);
	static SourceExcitation exp(SourceExcitation signal, float value);
	static SourceExcitation log(SourceExcitation signal);

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
		float permittivity = referance_permittivity;
		float permeability = referance_permeability;
		SourceExcitation source_excitation;
	};

	void initialize_fields(
		std::function<void(glm::ivec3, ElectroMagneticProperties&)> initialization_lambda,
		glm::ivec3 resolution,
		FloatingPointAccuracy fp_accuracy = FloatingPointAccuracy::fp32,
		BoundryCondition x_pn_boundry = PerfactlyMatched,
		BoundryCondition y_pn_boundry = PerfactlyMatched,
		BoundryCondition z_pn_boundry = PerfactlyMatched
	);

	void initialize_fields(
		std::function<void(glm::ivec3, ElectroMagneticProperties&)> initialization_lambda,
		glm::ivec3 resolution,
		FloatingPointAccuracy fp_accuracy,
		BoundryCondition x_p_boundry,
		BoundryCondition x_n_boundry,
		BoundryCondition y_p_boundry,
		BoundryCondition y_n_boundry,
		BoundryCondition z_p_boundry,
		BoundryCondition z_n_boundry
	);

	//void set_boundry_properties(
	//	uint32_t boundry_id,
	//	float permittivity = referance_permittivity,
	//	float permeability = referance_permeability
	//);
	//void clear_boundry_properties();

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
	bool is_even_timestep = true;
	size_t total_ticks_elapsed = 0;
	std::chrono::time_point<std::chrono::system_clock> simulation_begin;
	std::chrono::time_point<std::chrono::system_clock> last_visual_update;

	// LBM simulation parameters
	FDTDGrid fdtd_grid = Yee2D;
	FloatingPointAccuracy floating_point_accuracy = fp32;

	glm::ivec3 resolution = glm::ivec3(0);

	void _set_fdtd_grid(FDTDGrid fdtd_grid);
	void _set_floating_point_accuracy(FloatingPointAccuracy floating_point_accuracy);

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
			float permittivity = referance_permittivity,
			float permeability = referance_permeability
		);

		float permittivity;
		float permeability;
	};

	// boundries buffer holds the id of the object it is a part of (0 means not_a_boundry)
	// number of bits per voxel can change dynamically basad on how many objects are defined
	// velocity information of each object is held in another buffer in device called "objects"
	// objects buffer schema is [vec2 permittivity_permability] 
	std::vector<_object_desc> objects_cpu;
	int32_t bits_per_boundry = 0;
	void _set_bits_per_boundry(int32_t value);
	int32_t _get_bits_per_boundry(int32_t value);

	// device buffers
	bool is_grid_texture3d = true;
	Texture3D::ColorTextureFormat grid_tex_internal_format = Texture3D::ColorTextureFormat::R16F;

	std::shared_ptr<Texture3D> electric_tex = nullptr;
	std::shared_ptr<Texture3D> magnetic_tex = nullptr;
	std::shared_ptr<Buffer> electric_buffer = nullptr;
	std::shared_ptr<Buffer> magnetic_buffer = nullptr;
	std::shared_ptr<Buffer> boundries = nullptr;
	std::shared_ptr<UniformBuffer> objects = nullptr;

	// kernels
	bool is_programs_compiled = false;
	std::shared_ptr<ComputeProgram> fdtd_propagate_electric = nullptr;
	std::shared_ptr<ComputeProgram> fdtd_propagate_magnetic = nullptr;

	// renderers
	std::shared_ptr<Program> program_render2d_electric = nullptr;
	std::shared_ptr<Program> program_render2d_magnetic = nullptr;
	std::shared_ptr<Program> program_render2d_electromagnetic = nullptr;
	std::shared_ptr<Program> program_render2d_permittivity = nullptr;
	std::shared_ptr<Program> program_render2d_permeability = nullptr;
	std::shared_ptr<Program> program_render2d_reflection_index = nullptr;
	std::shared_ptr<Program> program_render2d_optic = nullptr;
	
	std::shared_ptr<Program> program_render3d_electric = nullptr;
	std::shared_ptr<Program> program_render3d_magnetic = nullptr;
	std::shared_ptr<Program> program_render3d_electromagnetic = nullptr;
	std::shared_ptr<Program> program_render3d_permittivity = nullptr;
	std::shared_ptr<Program> program_render3d_permeability = nullptr;
	std::shared_ptr<Program> program_render3d_reflection_index = nullptr;
	std::shared_ptr<Program> program_render3d_optic = nullptr;
	
	std::shared_ptr<Mesh> plane_mesh = nullptr;
	std::shared_ptr<Mesh> plane_cube = nullptr;

};

#include "FDTD_SourceExcitation_Templated.h"
