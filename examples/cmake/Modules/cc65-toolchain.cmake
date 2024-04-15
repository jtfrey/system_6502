# CMake toolchain file for cc65

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR 6502)

# Check for user provided CC65_PATH environment variable
list(APPEND CMAKE_PREFIX_PATH $ENV{CC65_PATH})

find_program(_CA65 ca65)
if ( _CA65-NOTFOUND )
    message(FATAL "ca65 assembler not found and is required")
endif ()
set(CMAKE_ASM_COMPILER ${_CA65})
set(CMAKE_ASM_COMPILER_ID ca65)

find_program(_CL65 cl65)
if ( _CA65-NOTFOUND )
    message(FATAL "cl65 linker not found and is required")
endif ()
set(CMAKE_LINKER ${_CL65})
set(CMAKE_LINKER_ID cl65)

set(CA65_TARGET_CPU "6502" CACHE STRING "Which ISA should ca65 target (default: 6502)")

# Overridable default flag values.
set(CC65_TARGET_FLAG "none" CACHE STRING "Target flag for CA65 (default: none)")

# Set default to the expected CMake variables
set(CMAKE_ASM_FLAGS_INIT "--cpu ${CA65_TARGET_CPU} -t ${CC65_TARGET_FLAG}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS_INIT}" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_DEBUG_INIT "")
set(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG_INIT}" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_RELEASE_INIT "")
set(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_ASM_FLAGS_RELEASE_INIT}" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_MINSIZEREL_INIT "")
set(CMAKE_ASM_FLAGS_MINSIZEREL "${CMAKE_ASM_FLAGS_MINSIZEREL_INIT}" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO_INIT "")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO "${CMAKE_ASM_FLAGS_RELWITHDEBINFO_INIT}" CACHE STRING "" FORCE)

# recognize typical assembly source file extensions:
set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS s;S;asm)

set(CMAKE_ASM_COMPILE_OBJECT
  "<CMAKE_ASM_COMPILER> <FLAGS> <DEFINES> <INCLUDES> -l <OBJECT>.lst -o <OBJECT> <SOURCE>"
)
set(CMAKE_INCLUDE_FLAG_ASM "--asm-include-dir ")
set(CMAKE_ASM_LINK_EXECUTABLE
    "<CMAKE_LINKER> <FLAGS> <LINK_FLAGS> -o <TARGET> <OBJECTS>"
)

# Create depfiles using ca65
set(CMAKE_DEPFILE_FLAGS_ASM "--create-dep <DEP_FILE>")
