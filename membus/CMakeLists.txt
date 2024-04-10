#
# Build the membus library
#
set(system_6502_membus_SOURCES
        membus.c
        membus_module_std64k.c
        membus_module_paged.c
        membus_module_byte.c
        membus_module_word.c
        membus_module_watchpoints.c
    )

add_library(system_6502_membus STATIC ${system_6502_membus_SOURCES})
target_include_directories(system_6502_membus PRIVATE ${CMAKE_BINARY_DIR})
        

if ( ENABLE_MEMBUS_TEST )
    add_executable(membus_test membus_test.c)
    target_include_directories(membus_test PRIVATE ${CMAKE_BINARY_DIR})
    target_link_libraries(membus_test system_6502_membus)
endif ()
