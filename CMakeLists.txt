#
# CMake build configuration for system_6502
#
cmake_minimum_required(VERSION 3.20)

project(system_6502
        VERSION 1.0.0
        DESCRIPTION "A 6502 emulator written in C"
        LANGUAGES C
    )

option(ENABLE_REGISTERS_TEST "Compile the registers test program" OFF)
option(ENABLE_ISA_6502_TEST "Compile the ISA 6502 test program" OFF)
option(ENABLE_NES_MOCKUP "Compile the NES mockup program" OFF)
option(ENABLE_EXECUTOR_LOCKS "Executor is thread-safe" ON)
option(ENABLE_DISASSEMBLY "Include callbacks to disassemble executed instructions" ON)
option(DISABLE_JMP_PAGE_BOUNDARY_BUG "Do not include the JMP page wrap-around h/w bug" OFF)

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
# NMI/IRQ handling is gated by pthread locks:
#
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#
# Locate the curses tui library:
#
set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

# The standard CMake curses module does not locate the
# menu library.  Find the header file and add its path
# to CURSES_INCLUDE_DIRS if not present:
find_path(CURSES_MENU_INCLUDE_PATH
        NAMES ncursesw/menu.h ncursesw/menu.h menu.h
        PATHS ${CURSES_INCLUDE_DIRS}
        NO_DEFAULT_PATH
    )
if ( CURSES_MENU_INCLUDE_PATH-NOT_FOUND )
    message(FATAL_ERROR "No curses menu header found")
endif ()
if ( NOT ${CURSES_MENU_INCLUDE_PATH} IN_LIST CURSES_INCLUDE_DIRS )
    list(APPEND CURSES_INCLUDE_DIRS ${CURSES_MENU_INCLUDE_PATH})
endif ()

# Now find the menu library and add it to CURSES_LIBRARIES if
# not present:
find_library(CURSES_MENU_LIBRARY menu)
if ( CURSES_MENU_LIBRARY-NOT_FOUND )
    message(FATAL_ERROR "No curses menu library found")
endif ()
list(APPEND CURSES_LIBRARIES ${CURSES_MENU_LIBRARY})

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

#
# Build the membus library:
#
add_subdirectory(libmembus)

#
# For disassembly to work, the memory caching must be enabled:
#
if (ENABLE_DISASSEMBLY AND NOT ENABLE_MEMBUS_CACHE)
    message(FATAL "Disassembly enabled but membus cache is not")
endif()

#
# The 6502 system emulator:
#
add_executable(system_6502 registers.c isa_6502.c executor.c tui_window.c system_6502.c)
target_include_directories(system_6502 PRIVATE ${CURSES_INCLUDE_DIRS})
target_compile_options(system_6502 PRIVATE ${CURSES_CFLAGS})
target_link_libraries(system_6502 PRIVATE ${CURSES_LIBRARIES} Threads::Threads libmembus)

#
# The various test programs at this level:
#
if ( ENABLE_REGISTERS_TEST )
    add_executable(registers_test registers.c)
    target_compile_definitions(registers_test PRIVATE ENABLE_REGISTERS_TEST)
endif ()

if ( ENABLE_ISA_6502_TEST )
    add_executable(isa_6502_test registers.c isa_6502.c)
    target_compile_definitions(isa_6502_test PRIVATE ENABLE_ISA_6502_TEST)
    target_link_libraries(isa_6502_test PRIVATE libmembus)
endif ()

if ( ENABLE_NES_MOCKUP ) 
    add_executable(nes_mockup nes_mockup.c)
    target_link_libraries(nes_mockup PRIVATE Threads::Threads libmembus)
endif ()

#
# Build the examples?
#
option(ENABLE_EXAMPLES_BUILD "Also assemble and link the code examples (requires ca65/cl65)" OFF)
if ( ENABLE_EXAMPLES_BUILD )
    include(ExternalProject)
    ExternalProject_Add(6502_examples
            DOWNLOAD_COMMAND ""
            SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples"
            INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/examples"
            CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
        )
endif ()
