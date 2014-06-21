
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"


typedef struct A_BLOCK_LINK
{
	struct A_BLOCK_LINK * next_free_block;
	size_t block_size;
} block_link_t;


static const unsigned short  heapSTRUCT_SIZE = (sizeof(block_link_t) + portBYTE_ALIGNMENT - (sizeof(block_link_t) % portBYTE_ALIGNMENT));


_PTR malloc(size_t size)
{
	return pvPortMalloc(size);
}


_PTR realloc(_PTR data_ptr, size_t size)
{
	void * new_data_ptr = pvPortMalloc(size);

	if (new_data_ptr && data_ptr) {
		block_link_t * data_block = (block_link_t *) (((uint8_t *)data_ptr) - heapSTRUCT_SIZE);
		size_t to_copy = (size < data_block->block_size) ? size : data_block->block_size;
		memcpy((void *) new_data_ptr, (const void *) data_ptr, to_copy);

		vPortFree(data_ptr);
	}

	return new_data_ptr;
}


void free(_PTR ptr)
{
	vPortFree(ptr);
}
