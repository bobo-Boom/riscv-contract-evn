#include <stdint.h>
#include "tiny_mm.h"
#define MAX_MEM_SIZE	8192
uint8_t mem_pool[MAX_MEM_SIZE];
void main(void)
{
	uint8_t *ptr;
	uint32_t i;
	mm_init(mem_pool, MAX_MEM_SIZE);
	ptr = (uint8_t *)mm_alloc(mem_pool, 1024);
	if( !ptr )
		return;
	for(i = 0; i < 32; i++)
	{
		*ptr++ = i & 0xff;
	}
}
