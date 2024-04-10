
#ifndef __EXECUTOR_H__
#define __EXECUTOR_H__

#include "system_6502_config.h"
#include "registers.h"
#include "membus.h"
#include "isa_6502.h"
    

/*
 * @typedef executor_t
 *
 * Data structure that bundles-together the components of the virtual
 * machine.
 */
typedef struct executor {
    uint32_t            flags;
    pthread_mutex_t     state_lock;
    registers_t         *registers;
    membus_t            *memory;
    isa_6502_table_t    *isa;
} executor_t;

/*
 * @function executor_alloc_with_default_components
 *
 * Convenience function that dynamically-allocates an executor containing:
 *
 *     - The default 64 KiB RAM
 *     - The default register set
 *     - The default baseline 6502 ISA
 */
executor_t* executor_alloc_with_default_components(void);

/*
 * @function executor_alloc_with_components
 *
 * Returns a dynamically-allocated executor containing the given register set,
 * memory array, and ISA.  All components should already be initialized before
 * being passed to this function.
 */
executor_t* executor_alloc_with_components(registers_t *the_registers, membus_t *the_memory, isa_6502_table_t *the_isa);

/*
 * @function executor_free
 *
 * Deallocate a dynamically-allocated the_executor.
 */
void executor_free(executor_t *the_executor);

/*
 * @function executor_hard_reset
 *
 * Reset the_executor's memory array (to all $00 bytes) and reset its
 * register set to defaults.
 */
void executor_hard_reset(executor_t *the_executor);

/*
 * @function executor_stage_callback_t
 *
 * The type of a function that can be called by the_executor as the instruction
 * pipeline executes.  The callback function can update status displays, log information,
 * etc.
 *
 * The function receives the_executor itself; the current instruction processing stage;
 * the opcode being dispatched; the addressing mode of the opcode; and a pointer to the
 * opcode's dispatch record.
 *
 * For all but the isa_6502_instr_stage_execution_complete stage, the_cycle_count represents
 * the number of cycles consumed by the current instruction being processed.  For the
 * isa_6502_instr_stage_execution_complete stage, the_cycle_count is the total cycles the
 * virtual machine has executed.
 */
typedef void (*executor_stage_callback_t)(executor_t *the_executor, isa_6502_instr_stage_t the_stage, isa_6502_opcode_t the_opcode, isa_6502_addressing_t the_addressing_mode, isa_6502_opcode_dispatch_t *the_dispatch, uint64_t the_cycle_count, const char *disasm, int disasm_len);

/*
 * @function executor_stage_callback_default
 *
 * The default executor stage callback function.  Information is output to stdout for all
 * but the isa_6502_instr_stage_illegal_instruction stage (which outputs to stderr).  A
 * useful summary is output after the fetch, decode, execution cycles, and completion of
 * the processing pipeline.
 */
void executor_stage_callback_default(executor_t *the_executor, isa_6502_instr_stage_t the_stage, isa_6502_opcode_t the_opcode, isa_6502_addressing_t the_addressing_mode, isa_6502_opcode_dispatch_t *the_dispatch, uint64_t the_cycle_count, const char *disasm, int disasm_len);

/*
 * @defined executor_stage_callback_default_stage_mask
 *
 * The stages for which the default stage callback produces output.  Use this mask
 * (possibly with bits removed) to limit the callback's usage to only those events
 * that will produce output.
 */
#define executor_stage_callback_default_stage_mask \
            isa_6502_instr_stage_post_load_PC | isa_6502_instr_stage_post_fetch_opcode | \
            isa_6502_instr_stage_post_decode_opcode | isa_6502_instr_stage_next_cycle | isa_6502_instr_stage_end | \
            isa_6502_instr_stage_execution_complete | isa_6502_instr_stage_illegal_instruction

/*
 * @function executor_launch_at_address
 *
 * Before calling this function the_executor should have been reset.
 *
 * Begin processing instructions at the start_addr in the executor's
 * memory array.  Execution does not complete until an illegal instruction
 * is encountered.
 *
 * Returns the total number of cycles executed.
 */
uint64_t executor_launch_at_address(
                executor_t                  *the_executor,
                executor_stage_callback_t   callback_fn,
                isa_6502_instr_stage_t      callback_stage_mask,
                uint16_t                    start_addr
            );

/*
 * @function executor_launch_at_address_range
 *
 * Before calling this function the_executor should have been reset.
 *
 * Begin processing instructions at the start_addr in the executor's
 * memory array.  Execution does not complete until an illegal instruction
 * or the program counter has gone beyond end_addr.
 *
 * Returns the total number of cycles executed.
 */
uint64_t executor_launch_at_address_range(
                executor_t                  *the_executor,
                executor_stage_callback_t   callback_fn,
                isa_6502_instr_stage_t      callback_stage_mask,
                uint16_t                    start_addr,
                uint16_t                    end_addr
            );

/*
 * @function executor_soft_reset
 *
 * The RESET vector in the executor's memory array should have been
 * set to the execution starting address.
 *
 * Essentially performs a "JMP ($FFFC)" to execute the code pointed-to by
 * the RESET vector.  Execution does not complete until an illegal
 * instruction is encountered.
 *
 * Returns the total number of cycles executed.
 */
uint64_t executor_soft_reset(
                executor_t                  *the_executor,
                executor_stage_callback_t   callback_fn,
                isa_6502_instr_stage_t      callback_stage_mask
            );

/*
 * @function executor_set_irq
 *
 * Signal the executor that an IRQ has been received.
 */
void executor_set_irq(
                executor_t  *the_executor
            );

/*
 * @function executor_set_nmi
 *
 * Signal the executor that an NMI has been received.
 */
void executor_set_nmi(
                executor_t  *the_executor
            );

#endif /* __EXECUTOR_H__ */
