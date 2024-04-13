
#ifndef __MEMBUS_MODULE_PAGED_H__
#define __MEMBUS_MODULE_PAGED_H__

#include "membus.h"

membus_module_ref membus_module_paged_alloc(membus_module_mode_t mode, uint8_t base_page, uint8_t n_pages);

#endif /* __MEMBUS_MODULE_PAGED_H__ */
