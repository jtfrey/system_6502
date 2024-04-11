
#ifndef __MEMBUS_MODULE_MIRROR_H__
#define __MEMBUS_MODULE_MIRROR_H__

#include "membus.h"

membus_module_ref membus_module_mirror_alloc(membus_module_ref original_module, uint16_t mirror_base_addr);

#endif /* __MEMBUS_MODULE_MIRROR_H__ */
