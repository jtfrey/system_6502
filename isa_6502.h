
#ifndef __ISA_6502_H__
#define __ISA_6502_H__

#include "system_6502_config.h"
#include "libmembus/membus.h"
#include "registers.h"

/*
 * @typedef isa_6502_opcode_t
 *
 * Overlay of an 8-bit 6502 opcode with field breakout.
 */
typedef union {
    uint8_t     BYTE;
    struct {
        uint8_t C:2;
        uint8_t B:3;
        uint8_t A:3;
    } FIELDS;
} isa_6502_opcode_t;

/*
 * @function isa_6502_opcode_null
 *
 * Returns a isa_6502_opcode_t initialized to a zero opcode.
 */
static inline isa_6502_opcode_t
isa_6502_opcode_null(void)
{
    isa_6502_opcode_t   new_opcode = { .BYTE = 0 };
    return new_opcode;
}

/*
 * @enum 6502 addressing modes
 *
 * Enumerates the 6502 operand addressing modes.
 */
enum {
    isa_6502_addressing_undefined = -1,                 /* Used for illegal opcodes     */
    isa_6502_addressing_accumulator = 0,                /* ADC A                        */
    isa_6502_addressing_absolute,                       /* ADC $XXXX                    */
    isa_6502_addressing_absolute_x_indexed,             /* ADC $XXXX,X                  */
    isa_6502_addressing_absolute_y_indexed,             /* ADC $XXXX,Y                  */
    isa_6502_addressing_immediate,                      /* ADC #$XX                     */
    isa_6502_addressing_implied,                        /* TXA                          */
    isa_6502_addressing_indirect,                       /* ADC ($XXXX)                  */
    isa_6502_addressing_x_indexed_indirect,             /* ADC ($XXXX,X)                */
    isa_6502_addressing_indirect_y_indexed,             /* ADC ($XXXX),Y                */
    isa_6502_addressing_relative,                       /* BEQ *±N                     */
    isa_6502_addressing_zeropage,                       /* ADC $XX                      */
    isa_6502_addressing_zeropage_x_indexed,             /* ADC $XX,X                    */
    isa_6502_addressing_zeropage_y_indexed,             /* ADC $XX,Y                    */
    isa_6502_addressing_zeropage_indirect,              /* ADC ($XX)              65C02 */
    isa_6502_addressing_absolute_x_indexed_indirect,    /* ADC ($XXXX,X)          65C02 */
    isa_6502_addressing_max
};

/*
 * @typedef isa_6502_addressing_t
 *
 * Type to hold 6502 addressing modes.
 */
typedef int isa_6502_addressing_t;

/*
 * @constant isa_6502_addressing_modes_descriptions
 *
 * Human-readable decription of the addressing modes
 */
extern const char* isa_6502_addressing_modes_descriptions[isa_6502_addressing_max];

/*
 * @constant isa_6502_addressing_operand_counts
 *
 * Number of operand bytes required by each of the 6502 addressing
 * modes.
 */
extern const int isa_6502_addressing_operand_counts[isa_6502_addressing_max];

/*
 * @typedef isa_6502_instr_context_t
 *
 * A data structure that wraps all of the state for evaluation
 * of an instruction.
 *
 */
typedef struct {
    isa_6502_opcode_t       opcode;
    isa_6502_addressing_t   addressing_mode;
    uint64_t                cycle_count;
    membus_t                *memory;
    registers_t             *registers;
} isa_6502_instr_context_t;

/*
 * @enum 6502 instruction dispatch stages
 *
 * Models the state machine for instruction dispatching
 * on the virtual processor.
 */
enum {
    isa_6502_instr_stage_pre_load_PC = 1 << 0,
    isa_6502_instr_stage_post_load_PC = 1 << 1,
    isa_6502_instr_stage_pre_fetch_opcode = 1 << 2,
    isa_6502_instr_stage_post_fetch_opcode = 1 << 3,
    isa_6502_instr_stage_pre_decode_opcode = 1 << 4,
    isa_6502_instr_stage_post_decode_opcode = 1 << 5,
    isa_6502_instr_stage_pre_next_cycle = 1 << 6,
    isa_6502_instr_stage_next_cycle = 1 << 7,
    isa_6502_instr_stage_end = 1 << 8,
    isa_6502_instr_stage_disasm = 1 << 9,
    isa_6502_instr_stage_execution_complete = 1 << 10,
    /**/
    isa_6502_instr_stage_enter_nmi = 1 << 11,
    isa_6502_instr_stage_exec_nmi = 1 << 12,
    isa_6502_instr_stage_enter_irq= 1 << 13,
    isa_6502_instr_stage_exec_irq = 1 << 14,
    isa_6502_instr_stage_exec_dma = 1 << 15,
    isa_6502_instr_stage_illegal_instruction = 1 << 31,
    /**/
    isa_6502_instr_stage_all = 0xFFFFFFFF
};

/*
 * @typedef isa_6502_instr_stage_t
 *
 * Type to hold 6502 instruction dispatch stage.
 */
typedef uint32_t isa_6502_instr_stage_t;

/*
 * @typedef isa_6502_instr_exec_callback_t
 *
 * Type of the function that handles execution stages of a
 * 6502 opcode.  The function receives a pointer to the opcode
 * context structure and the current stage.
 *
 * The function must return the next stage value for the
 * processing of the instruction.
 */
typedef isa_6502_instr_stage_t (*isa_6502_instr_exec_callback_t)(isa_6502_instr_context_t *opcode_context, isa_6502_instr_stage_t at_stage);

/*
 * @typedef isa_6502_instr_inline_disasm_callback_t
 *
 * Type of the function that handles disassembly of an instruction
 * during execution.  This only works if the memory system was compiled
 * with read/write caches so that operands and outputs can be
 * retrieved quickly and easily.
 */
typedef int (*isa_6502_instr_inline_disasm_callback_t)(isa_6502_instr_context_t *opcode_context, char *buffer, int buffer_len);

/*
 * @enum 6502 opcode execution mode
 *
 * Every opcode can be executed in two modes:
 *
 * - staged:  each clock cycle is represented as a distinct call to
 *            the execution function; a 4 cycle instruction will require
 *            3 function calls to complete
 * - static:  the instruction is executed with a single function call
 *            and the opcode_context's cycle_count will be updated with
 *            the total real cycles that would have been consumed
 *  - normal: the callback_fn IS NOT invoked as the pipeline proceeds
 *  - verbose: the callback_fn IS invoked as the pipeline proceeds
 */
enum {
    isa_6502_opcode_exec_mode_staged = 0,
    isa_6502_opcode_exec_mode_static = 1,
    isa_6502_opcode_exec_mode_mask = 0x000F,
    //
    isa_6502_opcode_exec_mode_locking = 0 << 30,
    isa_6502_opcode_exec_mode_no_locking = 1 << 30,
    //
    isa_6502_opcode_exec_mode_normal = 0 << 31,
    isa_6502_opcode_exec_mode_verbose = 1 << 31,
    //
    isa_6502_opcode_exec_mode_default = isa_6502_opcode_exec_mode_staged | isa_6502_opcode_exec_mode_verbose
};

/*
 * @typedef isa_6502_opcode_exec_mode_t
 *
 * The type used to represent opcode execution mode values.
 */
typedef unsigned int isa_6502_opcode_exec_mode_t;

/*
 * @typedef isa_6502_opcode_dispatch_t
 *
 * Data structure to hold the callback functions associated with
 * an ISA table slot, mnemonic, mnemonic+operand description, and
 * the addressing mode of the opcode question.
 */
typedef struct isa_6502_opcode_dispatch {
    isa_6502_instr_exec_callback_t              exec_fn[2];
    isa_6502_instr_inline_disasm_callback_t     inline_disasm_fn;
    const char                                  mnemonic[6];
    const char                                  *description;
    isa_6502_addressing_t                       addressing_mode;
} isa_6502_opcode_dispatch_t;

/*
 * @typedef isa_6502_opcode_block_b_t
 *
 * A block of 8 dispatch slots associated with the "B" index of
 * the opcode.
 */
typedef isa_6502_opcode_dispatch_t isa_6502_opcode_block_b_t[8];

/*
 * @typedef isa_6502_opcode_block_c_t
 *
 * A block of 8 blocks of "B" slots associated with the "A" index
 * of the opcode.
 */
typedef isa_6502_opcode_block_b_t isa_6502_opcode_block_a_t[8];

/*
 * @typedef isa_6502_opcode_blocks_t
 *
 * A block of 4 blocks of "A" index blocks of "B" slots associated
 * with the "C" index of the opcode.
 */
typedef isa_6502_opcode_block_a_t isa_6502_opcode_blocks_t[4];

/*
 * @typedef isa_6502_table_t
 *
 * Data structure that holds a 6502 instruction dispatch table
 * and pointers to the functions that handle the NMI, IRQ, and
 * RESET actions.
 */
typedef struct isa_table {
    isa_6502_opcode_blocks_t        table;
    isa_6502_instr_exec_callback_t  nmi, irq, reset;
} isa_6502_table_t;

/*
 * @enum ISA 6502 dialect selector
 *
 * For future expansion that implements refinements to the base
 * ISA (e.g. 65C02).
 */
enum {
    isa_6502_dialect_base = 0,
    isa_6502_dialect_65C02
};

/*
 * @typedef isa_6502_dialect_t
 *
 * Type to hold an ISA 6502 dialect selector.
 */
typedef int isa_6502_dialect_t;

/*
 * @function isa_6502_table_alloc
 *
 * Dynamically-allocate an empty isa 6502 table.  The result
 * must be initialized using the isa_6502_table_init()
 * function.
 */
isa_6502_table_t* isa_6502_table_alloc(void);

/*
 * @function isa_6502_table_init
 *
 * Initialize isa_table with the desired 6502 ISA dispatch
 * table.  If isa_table is NULL, a new table will be allocated and
 * returned.
 *
 * If an invalid use_dialect is presented, the function returns a
 * NULL pointer regardless of the value of isa_table passed to the
 * function.
 */
isa_6502_table_t* isa_6502_table_init(isa_6502_table_t *isa_table, isa_6502_dialect_t use_dialect);

/*
 * @function isa_6502_table_free
 *
 * Deallocate a dynamically-allocated isa_table.
 */
void isa_6502_table_free(isa_6502_table_t *isa_table);

/*
 * @function isa_6502_table_lookup_dispatch
 *
 * Given an opcode, lookup the associated instruction dispatch
 * record and return a pointer to it.
 *
 * The return value may point to a dispatch record for an
 * unimplemented instruction, for which the callback function
 * will be NULL and the addressing mode equal to
 * isa_6502_addressing_undefined.
 */
isa_6502_opcode_dispatch_t* isa_6502_table_lookup_dispatch(isa_6502_table_t *isa_table, isa_6502_opcode_t *opcode);

/*
 * @function isa_6502_pop
 *
 * Remove the last-pushed value from the stack managed by the_registers
 * and stored in the_membus memory array.
 *
 * This function is also not necessarily thread-safe w.r.t. an instruction
 * pipeline moving through the_registers and the_memory.  Only use this
 * function if you're absolutely sure you know what you're doing, as it
 * could alter the functioning of running code!
 */
uint8_t isa_6502_pop(registers_t *the_registers, membus_t *the_membus);

/*
 * @function isa_6502_push
 *
 * Add the given value to the stack managed by the_registers and stored
 * in the_membus memory array.
 *
 * This function is also not necessarily thread-safe w.r.t. an instruction
 * pipeline moving through the_registers and the_memory.  Only use this
 * function if you're absolutely sure you know what you're doing, as it
 * could alter the functioning of running code!
 */
void isa_6502_push(registers_t *the_registers, membus_t *the_membus, uint8_t value);

/*
 * @functon isa_6502_static_disassembly
 *
 * Given an ISA table, a memory array, and a range of addresses in that
 * memory array, disassemble the machine code.  The resulting assembly code
 * is written to the given FILE stream in the basic format:
 *
 *     <address> : <opcode-byte> <operand1-byte> {<operand2-byte>}    <mnemonic> <symbolic-operands>
 *
 */
void isa_6502_static_disassembly(isa_6502_table_t *isa_table, membus_t *the_membus, memory_addr_range_t addr_range, FILE *stream);


#endif /* __ISA_6502_H__ */
