#
# Example 6502 assembly code
#

cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
include(cc65-toolchain)

project(6502_examples
        VERSION 1.0.0
        DESCRIPTION "6502 example programs"
        LANGUAGES ASM
    )

add_executable(hello.bin src/hello.s)

add_executable(hello_alt.bin src/hello_alt.s)

add_executable(multiply_by_3.bin src/multiply_by_3.s)

add_executable(65C02_multiply_by_3.bin src/65C02_multiply_by_3.s)
target_compile_options(65C02_multiply_by_3.bin PRIVATE --cpu 65C02)
target_link_options(65C02_multiply_by_3.bin PRIVATE --cpu 65C02)

add_executable(bit_reorder_1.bin src/bit_reorder_1.s)

add_executable(bit_reorder_2.bin src/bit_reorder_2.s)

add_executable(bit_reorder_3.bin src/bit_reorder_3.s)

add_executable(bit_reorder_4.bin src/bit_reorder_4.s)

add_executable(bit_reorder_5.bin src/bit_reorder_5.s)

add_executable(bit_reorder_6.bin src/bit_reorder_6.s)

add_executable(bit_reorder_7.bin src/bit_reorder_7.s)

add_executable(fibonacci.bin src/fibonacci.s)

add_executable(nmi-test.bin src/nmi-test.s)

install(
    TARGETS
        hello.bin hello_alt.bin multiply_by_3.bin 65C02_multiply_by_3.bin
        bit_reorder_1.bin bit_reorder_2.bin bit_reorder_3.bin
        bit_reorder_4.bin bit_reorder_5.bin bit_reorder_6.bin
        bit_reorder_7.bin fibonacci.bin nmi-test.bin
    DESTINATION
        ${CMAKE_INSTALL_PREFIX}/bin
)
