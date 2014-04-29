
#include <stdbool.h>
#include <stdint.h>

#include "main_task.h"


void main_task(void * params)
{
	int i = 0;
	while (true) {
		if (i > 1000) {
			i = 0;
		}
		i++;
	}
}


uint32_t test_func(void)
{
	return 0UL;
}
