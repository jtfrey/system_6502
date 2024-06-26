
#ifndef __EXECUTOR_H__
#define __EXECUTOR_H__

#include "system_6502_config.h"
#include "registers.h"
#include "libmembus/membus.h"
#include "isa_6502.h"
    

/*
 * @typedef executor_dma_provider_callback_t
 *
 * The type of a function that provides data to a Direct Memory Access
 * transfer (into system RAM) initiated on an executor.
 *
 * The byte_index starts at zero and increments with each call.
 *
 * The context is an opaque pointer that can be used to supply external
 * information to the provider.
 */
typedef bool (*executor_dma_provider_callback_t)(const void *context, uint16_t byte_index, uint8_t *next_byte);

/*
 * @typedef executor_dma_consumer_callback_t
 *
 * The type of a function that consumes data from a Direct Memory Access
 * transfer (out of system RAM) initiated on an executor.
 *
 * The byte_index starts at zero and increments with each call.
 *
 * The context is an opaque pointer that can be used to supply external
 * information to the consumer.
 */
typedef bool (*executor_dma_consumer_callback_t)(const void *context, uint16_t byte_index, uint8_t next_byte);

/*
 * @typedef executor_t
 *
 * Data structure that bundles-together the components of the virtual
 * machine.
 */
typedef struct executor {
    uint32_t                            flags;
    registers_t                         *registers;
    membus_t                            *memory;
    isa_6502_table_t                    *isa;
    
    /* DMA control: */
    memory_addr_range_t                 dma_range;
    executor_dma_provider_callback_t    dma_provider;
    executor_dma_consumer_callback_t    dma_consumer;
    const void                          *dma_context;
    
#ifdef ENABLE_EXECUTOR_LOCKS
    pthread_mutex_t                     state_lock;
#endif
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
 * @function executor_launch
 *
 * Before calling this function the_executor should have been reset.
 *
 * Begin processing instructions at the given address PC in the executor's
 * memory array.  Execution does not complete until an illegal instruction
 * is encountered.
 *
 * Returns the total number of cycles executed.
 */
uint64_t executor_launch_at_address(
                executor_t                  *the_executor,
                isa_6502_opcode_exec_mode_t exec_mode,
                executor_stage_callback_t   callback_fn,
                isa_6502_instr_stage_t      callback_stage_mask,
                uint16_t                    PC
            );

/*
 * @function executor_launch_in_address_range
 *
 * Before calling this function the_executor should have been reset.
 *
 * Begin processing instructions at the given address PC in the executor's
 * memory array.  Execution does not complete until an illegal instruction
 * or the program counter has gone outside addr_range.
 *
 * Returns the total number of cycles executed.
 */
uint64_t executor_launch_in_address_range(
                executor_t                  *the_executor,
                isa_6502_opcode_exec_mode_t exec_mode,
                executor_stage_callback_t   callback_fn,
                isa_6502_instr_stage_t      callback_stage_mask,
                memory_addr_range_t         addr_range,
                uint16_t                    PC
            );

/*
 * @function executor_soft_reset
 *
 * The RESET vector in the executor's membus memory array should have been
 * set to the execution starting address prior to calling this function.
 *
 * Essentially performs a "JMP ($FFFC)" to execute the code pointed-to by
 * the RESET vector.  Execution does not complete until an illegal
 * instruction is encountered.
 *
 * Returns the total number of cycles executed.
 */
uint64_t executor_soft_reset(
                executor_t                  *the_executor,
                isa_6502_opcode_exec_mode_t exec_mode,
                executor_stage_callback_t   callback_fn,
                isa_6502_instr_stage_t      callback_stage_mask
            );
            
/*
 * @function executor_set_irq
 *
 * Signal the executor that an IRQ has been received.
 */
void executor_set_irq(executor_t *the_executor);

/*
 * @function executor_set_nmi
 *
 * Signal the executor that an NMI has been received.
 */
void executor_set_nmi(executor_t *the_executor);

/*
 * @function executor_set_exec_stop
 *
 * Signal the executor that it should halt execution.
 */
void executor_set_exec_stop(executor_t *the_executor);

/*
 * @function executor_set_dma_copyin
 *
 * Signal the executor that a DMA operation from an external source
 * into RAM should happen.
 *
 * The provider_callback is a function that progressively supplies
 * the bytes to write into RAM.
 */
void executor_set_dma_copyin(
                executor_t                          *the_executor,
                memory_addr_range_t                 addr_range,
                executor_dma_provider_callback_t    provider_callback,
                const void                          *callback_context
            );

/*
 * @function executor_set_dma_copyout
 *
 * Signal the executor that a DMA operation moving data from RAM to
 * an external entity should happen.
 */
void executor_set_dma_copyout(
                executor_t                          *the_executor,
                memory_addr_range_t                 addr_range,
                executor_dma_consumer_callback_t    consumer_callback,
                const void                          *callback_context
            );

#endif /* __EXECUTOR_H__ */
