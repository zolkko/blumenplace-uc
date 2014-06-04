#include <stdint.h>
#include <stdbool.h>
#include "rf.h"


uint8_t rf_t::version(void) {
	while (!acquire_lock()) ;

	hw.chip_select();
	hw.wait_ready();

	uint8_t version = 0;
	read(CCx_VERSION, &version);

	hw.chip_release(hw);
	release_lock();

	return version;
}
