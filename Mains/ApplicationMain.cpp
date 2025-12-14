#include "GozdiscopticsMax.h"

void main() {

	gozdiscoptics::init();

	FDTD solver;

	solver.initialize_fields([](glm::ivec3 id, FDTD::ElectroMagneticProperties& properties) {
		properties.electric_field = glm::vec3(0);
		properties.magnetic_field = glm::vec3(0);
		
		properties.source_excitation =
			FDTD::cos(2)
			"cos(2*pi*frequency + phase);"
		;



		},
		glm::ivec3(1024, 1024, 1),
		true, true, true,
		FloatingPointAccuracy::fp32
		);



	gozdiscoptics::launch_realtime_window(solver);

	gozdiscoptics::release();
}
