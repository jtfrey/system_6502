#
# Build the membus library
#

option(ENABLE_MEMBUS_LOCKS "Memory controller is thread-safe" ON)
option(ENABLE_MEMBUS_CACHE "Memory controller caches last 8 bytes read/written" ON)
option(ENABLE_MEMBUS_TEST "Compile the memory bus test program" ON)

set(system_6502_membus_SOURCES
        memory_addr_range.c
        membus.c
        membus_module_std64k.c
        membus_module_paged.c
        membus_module_byte.c
        membus_module_word.c
        membus_module_watchpoints.c
        membus_module_mirror.c
    )
    
#
# Generate our common header file and have all targets in this
# directory use the binary directory on the search path:
#
configure_file(system_6502_membus_config.h.in system_6502_membus_config.h)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(libmembus STATIC ${system_6502_membus_SOURCES})
target_include_directories(libmembus INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

if ( ENABLE_MEMBUS_TEST )
    add_executable(membus_test membus_test.c)
    target_link_libraries(membus_test libmembus)
endif ()
