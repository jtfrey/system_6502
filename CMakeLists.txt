#
# CMake build configuration for system_6502
#
cmake_minimum_required(VERSION 3.12)

project(system_6502
        VERSION 1.0.0
        DESCRIPTION "A 6502 emulator written in C"
        LANGUAGES C
    )

option(ENABLE_MEMORY_TEST "Compile the memory test program" OFF)
option(ENABLE_REGISTERS_TEST "Compile the registers test program" OFF)
option(ENABLE_ISA_6502_TEST "Compile the ISA 6502 test program" OFF)

option(ENABLE_MEMORY_CACHE "Memory controller caches last 8 bytes read/written" ON)
option(ENABLE_DISASSEMBLY "Include callbacks to disassemble executed instructions" ON)

#
# For disassembly to work, the memory caching must be enabled:
#
if (ENABLE_DISASSEMBLY AND NOT ENABLE_MEMORY_CACHE)
    message(WARNING "Disassembly enabled but memory cache is not; forcing memory cache ON")
    set(ENABLE_MEMORY_CACHE ON)
endif()

#
# For the sake of 16-bit processing we need to compile according
# to the endian nature of the system.
#
if (NOT DEFINED ISA_6502_HOST_IS_LE)
    include (TestBigEndian)
    test_big_endian(IS_BIG_ENDIAN)
    if (IS_BIG_ENDIAN)
        set(ISA_6502_HOST_IS_LE OFF CACHE BOOL "Compilation target is little-endian")
    else ()
        set(ISA_6502_HOST_IS_LE ON CACHE BOOL "Compilation target is little-endian")
    endif()
endif()

#
# Add project info/version variables for the sake of the configure file:
#
set(SYSTEM_6502_NAME ${PROJECT_NAME})
set(SYSTEM_6502_VERSION ${system_6502_VERSION})

#
# Generate our common header file and have all targets in this
# directory use the binary directory on the search path:
#
configure_file(system_6502_config.h.in system_6502_config.h)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if ( ENABLE_MEMORY_TEST )
    add_executable(memory_test memory.c)
    target_compile_definitions(memory_test PRIVATE ENABLE_MEMORY_TEST)
endif ()

if ( ENABLE_REGISTERS_TEST )
    add_executable(registers_test registers.c)
    target_compile_definitions(registers_test PRIVATE ENABLE_REGISTERS_TEST)
endif ()

if ( ENABLE_ISA_6502_TEST )
    add_executable(isa_6502_test memory.c registers.c isa_6502.c)
    target_compile_definitions(isa_6502_test PRIVATE ENABLE_ISA_6502_TEST)
endif ()

add_executable(system_6502 memory.c registers.c isa_6502.c executor.c system_6502.c)
