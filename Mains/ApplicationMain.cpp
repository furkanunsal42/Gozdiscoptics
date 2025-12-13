#include "GozdiscopticsMax.h"

void main() {

	gozdiscoptics::init();

	while (!gozdiscoptics::context->should_close()){
		gozdiscoptics::context->handle_events(true);

		primitive_renderer::clear(0, 0, 0, 1);

		gozdiscoptics::context->swap_buffers();
	}

	gozdiscoptics::release();
}
