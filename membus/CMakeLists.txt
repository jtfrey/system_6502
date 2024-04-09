
option(ENABLE_MEMBUS_TEST "Compile the memory bus test program" ON)

add_library(system_6502_membus SHARED
        membus.c
        membus_module_std64k.c
        membus_module_paged.c
        membus_module_byte.c
        membus_module_word.c
    )

if ( ENABLE_MEMBUS_TEST )
    add_executable(membus_test membus_test.c)
    target_link_libraries(membus_test system_6502_membus)
endif ()
