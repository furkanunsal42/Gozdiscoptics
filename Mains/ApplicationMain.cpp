#include "GozdiscopticsMax.h"

void main() {

	//gozdiscoptics::init();
	//
	//FDTD solver;

	//solver.initialize_fields([](glm::ivec3 id, FDTD::ElectroMagneticProperties& properties) {
	//	properties.electric_field = glm::vec3(0);
	//	properties.magnetic_field = glm::vec3(0);
	//	
	//	properties.source_excitation =
	//		FDTD::cos(FDTD::window(0, 100) * 2 * 3.14);
	//	
	//	},
	//	glm::ivec3(1024, 1024, 1)
	//	);


	FDTD::SourceExcitation source(1024);
	source = glm::pi<float>() + source;
	std::cout << source << std::endl;

	//gozdiscoptics::launch_realtime_window(solver);
	//
	//gozdiscoptics::release();
}
