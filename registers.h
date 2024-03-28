
#ifndef __REGISTERS_H__
#define __REGISTERS_H__

#include "system_6502_config.h"

/*
 * @enum 6502 status register bit values
 *
 * Bit values of each of the components of the SR (P) register.
 *
 *     C = carry
 *     Z = zero
 *     I = interrupt inhibit
 *     D = decimal mode
 *     B = break
 *     IGN = ignored
 *     V = overflow
 *     N = sign/negative
 */
enum {
    register_SR_Bit_C = 1 << 0,
    register_SR_Bit_Z = 1 << 1,
    register_SR_Bit_I = 1 << 2,
    register_SR_Bit_D = 1 << 3,
    register_SR_Bit_B = 1 << 4,
    register_SR_Bit_IGN = 1 << 5,
    register_SR_Bit_V = 1 << 6,
    register_SR_Bit_N = 1 << 7,
};

/*
 * @typedef registers_t
 *
 * Data structure containing all of the 6502 registers.
 *
 *     A = accumulator
 *     X = X-index
 *     Y = Y-index
 *     SP = stack pointer
 *     PC = program counter (instruction pointer)
 *     SR = status register (a.k.a. P)
 */
typedef struct registers {
    uint8_t     A, X, Y;
    uint8_t     SP;
    uint16_t    PC;
    uint8_t     SR;
} registers_t;

static inline int registers_SR_get_bit(registers_t *the_registers, int which_bit)
                    { return (the_registers->SR & which_bit) ? 1 : 0; }
static inline void registers_SR_set_bit(registers_t *the_registers, int which_bit, int value)
                    { if ( value ) the_registers->SR |= which_bit; else the_registers->SR &= ~which_bit; }

/*
 * @function registers_alloc
 *
 * Dynamically-allocate a register set.  The register set should be
 * initialized by calling the registers_init() function.
 */
registers_t* registers_alloc(void);

/*
 * @function registers_init
 *
 * Initialize the_registers register set.  If the_registers is NULL, a
 * new register set will be allocated and returned.
 */
registers_t* registers_init(registers_t *the_registers);

/*
 * @function registers_free
 *
 * Deallocate a dynamically-allocated the_registers.
 */
void registers_free(registers_t *the_registers);

/*
 * @function registers_reset
 *
 * Reinitialize the_registers to the nominal starting state.  All registers
 * are returned to zero ($00) and the program counter (PC) is set to the
 * reset vector address ($FFFC).
 */
void registers_reset(registers_t *the_registers);

/*
 * @function registers_snprintf
 *
 * Write a summary of the_registers' state to the given character buffer,
 * p, of size p_len.  Returns the number of characters written to p.
 */
int registers_snprintf(registers_t *the_registers, char *p, size_t p_len);

/*
 * @function registers_fprintf
 *
 * Output a summary of the_registers' state to the given file stream.
 * Returns the number of characters written to p.
 */
int registers_fprintf(registers_t *the_registers, FILE *stream);

/*
 * @enum carry flag disposition for SR updates
 *
 * Specifies the alteration to be made to the carry bit in the status
 * register.
 *
 *     registers_Carry_clear = clear the bit
 *     registers_Carry_set = set the bit
 *     registers_Carry_ignore = do nothing to the bit
 */
enum {
    registers_Carry_clear = 0,
    registers_Carry_set,
    registers_Carry_ignore
};


/*
 * @function registers_did_set_A
 *
 * Update SR bits according to the value of the accumulator (A) and the
 * carry_status.
 */
static inline void
registers_did_set_A(
    registers_t *the_registers,
    int         carry_status
)
{
    registers_SR_set_bit(the_registers, register_SR_Bit_Z, ( the_registers->A == 0 ));
    registers_SR_set_bit(the_registers, register_SR_Bit_N, ( (the_registers->A & 0x80) == 0x80 ));
    if ( carry_status < registers_Carry_ignore )
        registers_SR_set_bit(the_registers, register_SR_Bit_C, carry_status);
}

/*
 * @function registers_did_set_X
 *
 * Update SR bits according to the value of the X-index register (X) and the
 * carry_status.
 */
static inline void
registers_did_set_X(
    registers_t *the_registers,
    int         carry_status
)
{
    registers_SR_set_bit(the_registers, register_SR_Bit_Z, ( the_registers->X == 0 ));
    registers_SR_set_bit(the_registers, register_SR_Bit_N, ( (the_registers->X & 0x80) == 0x80 ));
    if ( carry_status < registers_Carry_ignore )
        registers_SR_set_bit(the_registers, register_SR_Bit_C, carry_status);
}

/*
 * @function registers_did_set_Y
 *
 * Update SR bits according to the value of the Y-index register (Y) and the
 * carry_status.
 */
static inline void
registers_did_set_Y(
    registers_t *the_registers,
    int         carry_status
)
{
    registers_SR_set_bit(the_registers, register_SR_Bit_Z, ( the_registers->Y == 0 ));
    registers_SR_set_bit(the_registers, register_SR_Bit_N, ( (the_registers->Y & 0x80) == 0x80 ));
    if ( carry_status < registers_Carry_ignore )
        registers_SR_set_bit(the_registers, register_SR_Bit_C, carry_status);
}

/*
 * @function registers_status_with_value
 *
 * Update SR bits according to the given immediate value.
 */
static inline void
registers_status_with_value(
    registers_t *the_registers,
    uint8_t     value,
    int         carry_status
)
{
    registers_SR_set_bit(the_registers, register_SR_Bit_Z, ( value == 0 ));
    registers_SR_set_bit(the_registers, register_SR_Bit_N, ( (value & 0x80) == 0x80 ));
    if ( carry_status < registers_Carry_ignore )
        registers_SR_set_bit(the_registers, register_SR_Bit_C, carry_status);
}

#endif /* __REGISTERS_H__ */
