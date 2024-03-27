
#include "isa_6502.h"

const char* isa_6502_addressing_modes_descriptions[isa_6502_addressing_max] = {
            "accumulator",
            "absolute",
            "x-indexed absolute",
            "y-indexed absolute",
            "immediate",
            "implied",
            "indirect",
            "(zero page) x-indexed indirect",
            "(zero page) indirect y-indexed",
            "relative",
            "zero page",
            "x-indexed zero page",
            "y-indexed zero page"
        };

//

const int isa_6502_addressing_operand_counts[isa_6502_addressing_max] = {
            0,  /* accumulator */
            2,  /* absolute */
            2,  /* x-indexed absolute */
            2,  /* y-indexed absolute */
            1,  /* immediate */
            0,  /* implied */
            2,  /* indirect */
            1,  /* (zero page) x-indexed indirect */
            1,  /* (zero page) indirect y-indexed */
            1,  /* relative */
            1,  /* zero page */
            1,  /* x-indexed zero page */
            1   /* y-indexed zero page */
        };

//

uint8_t
__isa_6502_pop(
    registers_t *the_registers,
    memory_t    *the_memory
)
{
    return memory_read(the_memory, 0x0100 + (++the_registers->SP & 0x00FF));
}

//

void
__isa_6502_push(
    registers_t *the_registers,
    memory_t    *the_memory,
    uint8_t     value
)
{
    memory_write(the_memory, 0x0100 + (the_registers->SP-- & 0x00FF), value);
}

//

#include "isa_6502/ADC.c"
#include "isa_6502/AND.c"
#include "isa_6502/ASL.c"
#include "isa_6502/BCC.c"
#include "isa_6502/BCS.c"
#include "isa_6502/BEQ.c"
#include "isa_6502/BIT.c"
#include "isa_6502/BMI.c"
#include "isa_6502/BNE.c"
#include "isa_6502/BPL.c"
#include "isa_6502/BRK.c"
#include "isa_6502/BVC.c"
#include "isa_6502/BVS.c"
#include "isa_6502/CLC.c"
#include "isa_6502/CLD.c"
#include "isa_6502/CLI.c"
#include "isa_6502/CLV.c"
#include "isa_6502/CMP.c"
#include "isa_6502/CPX.c"
#include "isa_6502/CPY.c"
#include "isa_6502/DEC.c"
#include "isa_6502/DEX.c"
#include "isa_6502/DEY.c"
#include "isa_6502/EOR.c"
#include "isa_6502/INC.c"
#include "isa_6502/INX.c"
#include "isa_6502/INY.c"
#include "isa_6502/LDA.c"
#include "isa_6502/LDX.c"
#include "isa_6502/LDY.c"
#include "isa_6502/LSR.c"
#include "isa_6502/JMP.c"
#include "isa_6502/JSR.c"
#include "isa_6502/NOP.c"
#include "isa_6502/ORA.c"
#include "isa_6502/PHA.c"
#include "isa_6502/PHP.c"
#include "isa_6502/PLA.c"
#include "isa_6502/PLP.c"
#include "isa_6502/ROL.c"
#include "isa_6502/ROR.c"
#include "isa_6502/RTI.c"
#include "isa_6502/RTS.c"
#include "isa_6502/SBC.c"
#include "isa_6502/SEC.c"
#include "isa_6502/SED.c"
#include "isa_6502/SEI.c"
#include "isa_6502/STA.c"
#include "isa_6502/STX.c"
#include "isa_6502/STY.c"
#include "isa_6502/TAX.c"
#include "isa_6502/TAY.c"
#include "isa_6502/TSX.c"
#include "isa_6502/TXA.c"
#include "isa_6502/TXS.c"
#include "isa_6502/TYA.c"

//

#define UNUSED_OPCODE   { NULL, "???", isa_6502_addressing_undefined }

static isa_6502_opcode_blocks_t __isa_6502_table = {
        {
            /* C = 0 */
            {
                /* A = 0 */
                { __isa_6502_BRK, "BRK", isa_6502_addressing_implied },
                UNUSED_OPCODE,
                { __isa_6502_PHP, "PHP", isa_6502_addressing_implied },
                UNUSED_OPCODE,
                { __isa_6502_BPL, "BPL $XX", isa_6502_addressing_relative },
                UNUSED_OPCODE,
                { __isa_6502_CLC, "CLC", isa_6502_addressing_implied },
                UNUSED_OPCODE
            },{
                /* A = 1 */
                { __isa_6502_JSR, "JSR $XXXX", isa_6502_addressing_absolute },
                { __isa_6502_BIT, "BIT $XX", isa_6502_addressing_zeropage },
                { __isa_6502_PLP, "PLP", isa_6502_addressing_implied },
                { __isa_6502_BIT, "BIT $XXX", isa_6502_addressing_absolute },
                { __isa_6502_BMI, "BMI $XX", isa_6502_addressing_relative },
                UNUSED_OPCODE,
                { __isa_6502_SEC, "SEC", isa_6502_addressing_implied },
                UNUSED_OPCODE
            },{
                /* A = 2 */
                { __isa_6502_RTI, "RTI", isa_6502_addressing_implied },
                UNUSED_OPCODE,
                { __isa_6502_PHA, "PHA", isa_6502_addressing_implied },
                { __isa_6502_JMP, "JMP $XXXX", isa_6502_addressing_absolute },
                { __isa_6502_BVC, "BVC $XX", isa_6502_addressing_relative },
                UNUSED_OPCODE,
                { __isa_6502_CLI, "CLI", isa_6502_addressing_implied },
                UNUSED_OPCODE
            },{
                /* A = 3 */
                { __isa_6502_RTS, "RTS", isa_6502_addressing_implied },
                UNUSED_OPCODE,
                { __isa_6502_PLA, "PLA", isa_6502_addressing_implied },
                { __isa_6502_JMP, "JMP ($XXXX)", isa_6502_addressing_indirect },
                { __isa_6502_BVS, "BVS $XX", isa_6502_addressing_relative },
                UNUSED_OPCODE,
                { __isa_6502_SEI, "SEI", isa_6502_addressing_implied },
                UNUSED_OPCODE
            },{
                /* A = 4 */
                UNUSED_OPCODE,
                { __isa_6502_STY, "STY $XX", isa_6502_addressing_zeropage },
                { __isa_6502_DEY, "DEY", isa_6502_addressing_implied },
                { __isa_6502_STY, "STY $XXXX", isa_6502_addressing_absolute },
                { __isa_6502_BCC, "BCC $XX", isa_6502_addressing_relative },
                { __isa_6502_STY, "STY $XX", isa_6502_addressing_zeropage },
                { __isa_6502_TYA, "TYA", isa_6502_addressing_implied },
                UNUSED_OPCODE
            },{
                /* A = 5 */
                { __isa_6502_LDY, "LDY #$XX", isa_6502_addressing_immediate },
                { __isa_6502_LDY, "LDY $XX", isa_6502_addressing_zeropage },
                { __isa_6502_TAY, "TAY", isa_6502_addressing_implied },
                { __isa_6502_LDY, "LDY $XXXX", isa_6502_addressing_absolute },
                { __isa_6502_BCS, "BCS $XX", isa_6502_addressing_relative },
                { __isa_6502_LDY, "LDY $XX,X", isa_6502_addressing_zeropage_x_indexed },
                { __isa_6502_CLV, "CLV", isa_6502_addressing_implied },
                { __isa_6502_LDY, "LDY $XXXX,X", isa_6502_addressing_absolute_x_indexed }
            },{
                /* A = 6 */
                { __isa_6502_CPY, "CPY #$XX", isa_6502_addressing_immediate },
                { __isa_6502_CPY, "CPY $XX", isa_6502_addressing_zeropage },
                { __isa_6502_INY, "INY", isa_6502_addressing_implied },
                { __isa_6502_CPY, "CPY $XXXX", isa_6502_addressing_absolute },
                { __isa_6502_BNE, "BNE $XX", isa_6502_addressing_relative },
                UNUSED_OPCODE,
                { __isa_6502_CLD, "CLD", isa_6502_addressing_implied },
                UNUSED_OPCODE
            },{
                /* A = 7 */
                { __isa_6502_CPX, "CPX #$XX", isa_6502_addressing_immediate },
                { __isa_6502_CPX, "CPX $XX", isa_6502_addressing_zeropage },
                { __isa_6502_INX, "INX", isa_6502_addressing_implied },
                { __isa_6502_CPX, "CPX $XXXX", isa_6502_addressing_absolute },
                { __isa_6502_BEQ, "BEQ $XX", isa_6502_addressing_relative },
                UNUSED_OPCODE,
                { __isa_6502_SED, "SED", isa_6502_addressing_implied },
                UNUSED_OPCODE
            }
        }, {
            /* C = 1 */
            {
                /* A = 0 */
                { __isa_6502_ORA, "ORA ($XX,X)", isa_6502_addressing_x_indexed_indirect },
                { __isa_6502_ORA, "ORA $XX", isa_6502_addressing_zeropage },
                { __isa_6502_ORA, "ORA #$XX", isa_6502_addressing_immediate },
                { __isa_6502_ORA, "ORA $XXXX", isa_6502_addressing_absolute },
                { __isa_6502_ORA, "ORA ($XXXX),Y", isa_6502_addressing_indirect_y_indexed },
                { __isa_6502_ORA, "ORA $XX,X", isa_6502_addressing_zeropage_x_indexed },
                { __isa_6502_ORA, "ORA $XXXX,Y", isa_6502_addressing_absolute_y_indexed },
                { __isa_6502_ORA, "ORA $XXXX,X", isa_6502_addressing_absolute_x_indexed }
            },{
                /* A = 1 */
                { __isa_6502_AND, "AND ($XX,X)", isa_6502_addressing_x_indexed_indirect },
                { __isa_6502_AND, "AND $XX", isa_6502_addressing_zeropage },
                { __isa_6502_AND, "AND #$XX", isa_6502_addressing_immediate },
                { __isa_6502_AND, "AND $XXXX", isa_6502_addressing_absolute },
                { __isa_6502_AND, "AND ($XXXX),Y", isa_6502_addressing_indirect_y_indexed },
                { __isa_6502_AND, "AND $XX,X", isa_6502_addressing_zeropage_x_indexed },
                { __isa_6502_AND, "AND $XXXX,Y", isa_6502_addressing_absolute_y_indexed },
                { __isa_6502_AND, "AND $XXXX,X", isa_6502_addressing_absolute_x_indexed }
            },{
                /* A = 2 */
                { __isa_6502_EOR, "EOR ($XX,X)", isa_6502_addressing_x_indexed_indirect },
                { __isa_6502_EOR, "EOR $XX", isa_6502_addressing_zeropage },
                { __isa_6502_EOR, "EOR #$XX", isa_6502_addressing_immediate },
                { __isa_6502_EOR, "EOR $XXXX", isa_6502_addressing_absolute },
                { __isa_6502_EOR, "EOR ($XXXX),Y", isa_6502_addressing_indirect_y_indexed },
                { __isa_6502_EOR, "EOR $XX,X", isa_6502_addressing_zeropage_x_indexed },
                { __isa_6502_EOR, "EOR $XXXX,Y", isa_6502_addressing_absolute_y_indexed },
                { __isa_6502_EOR, "EOR $XXXX,X", isa_6502_addressing_absolute_x_indexed }
            },{
                /* A = 3 */
                { __isa_6502_ADC, "ADC ($XX,X)", isa_6502_addressing_x_indexed_indirect },
                { __isa_6502_ADC, "ADC $XX", isa_6502_addressing_zeropage },
                { __isa_6502_ADC, "ADC #$XX", isa_6502_addressing_immediate },
                { __isa_6502_ADC, "ADC $XXXX", isa_6502_addressing_absolute },
                { __isa_6502_ADC, "ADC ($XXXX),Y", isa_6502_addressing_indirect_y_indexed },
                { __isa_6502_ADC, "ADC $XX,X", isa_6502_addressing_zeropage_x_indexed },
                { __isa_6502_ADC, "ADC $XXXX,Y", isa_6502_addressing_absolute_y_indexed },
                { __isa_6502_ADC, "ADC $XXXX, X", isa_6502_addressing_absolute_x_indexed }
            },{
                /* A = 4 */
                { __isa_6502_STA, "STA ($XX,X)", isa_6502_addressing_x_indexed_indirect },
                { __isa_6502_STA, "STA $XX", isa_6502_addressing_zeropage },
                UNUSED_OPCODE,
                { __isa_6502_STA, "STA $XXXX", isa_6502_addressing_absolute },
                { __isa_6502_STA, "STA ($XXXX),Y", isa_6502_addressing_indirect_y_indexed },
                { __isa_6502_STA, "STA $XX,X", isa_6502_addressing_zeropage_x_indexed },
                { __isa_6502_STA, "STA $XXXX,Y", isa_6502_addressing_absolute_y_indexed },
                { __isa_6502_STA, "STA $XXXX,X", isa_6502_addressing_absolute_x_indexed }
            },{
                /* A = 5 */
                { __isa_6502_LDA, "LDA ($XX,X)", isa_6502_addressing_x_indexed_indirect },
                { __isa_6502_LDA, "LDA $XX", isa_6502_addressing_zeropage },
                { __isa_6502_LDA, "LDA #$XX", isa_6502_addressing_immediate },
                { __isa_6502_LDA, "LDA $XXXX", isa_6502_addressing_absolute },
                { __isa_6502_LDA, "LDA ($XXXX),Y", isa_6502_addressing_indirect_y_indexed },
                { __isa_6502_LDA, "LDA $XX,X", isa_6502_addressing_zeropage_x_indexed },
                { __isa_6502_LDA, "LDA $XXXX,Y", isa_6502_addressing_absolute_y_indexed },
                { __isa_6502_LDA, "LDA $XXXX,X", isa_6502_addressing_absolute_x_indexed }
            },{
                /* A = 6 */
                { __isa_6502_CMP, "CMP ($XX,X)", isa_6502_addressing_x_indexed_indirect },
                { __isa_6502_CMP, "CMP $XX", isa_6502_addressing_zeropage },
                { __isa_6502_CMP, "CMP #$XX", isa_6502_addressing_immediate },
                { __isa_6502_CMP, "CMP $XXXX", isa_6502_addressing_absolute },
                { __isa_6502_CMP, "CMP ($XXXX),Y", isa_6502_addressing_indirect_y_indexed },
                { __isa_6502_CMP, "CMP $XX,X", isa_6502_addressing_zeropage_x_indexed },
                { __isa_6502_CMP, "CMP $XXXX,Y", isa_6502_addressing_absolute_y_indexed },
                { __isa_6502_CMP, "CMP $XXXX,X", isa_6502_addressing_absolute_x_indexed }
            },{
                /* A = 7 */
                { __isa_6502_SBC, "SBC ($XX,X)", isa_6502_addressing_x_indexed_indirect },
                { __isa_6502_SBC, "SBC $XX", isa_6502_addressing_zeropage },
                { __isa_6502_SBC, "SBC #$XX", isa_6502_addressing_immediate },
                { __isa_6502_SBC, "SBC $XXXX", isa_6502_addressing_absolute },
                { __isa_6502_SBC, "SBC ($XXXX),Y", isa_6502_addressing_indirect_y_indexed },
                { __isa_6502_SBC, "SBC $XX,X", isa_6502_addressing_zeropage_x_indexed },
                { __isa_6502_SBC, "SBC $XXXX,Y", isa_6502_addressing_absolute_y_indexed },
                { __isa_6502_SBC, "SBC $XXXX,X", isa_6502_addressing_absolute_x_indexed }
            }
        },{
            /* C = 2 */
            {
                /* A = 0 */
                UNUSED_OPCODE,
                { __isa_6502_ASL, "ASL $XX", isa_6502_addressing_zeropage },
                { __isa_6502_ASL, "ASL A", isa_6502_addressing_accumulator },
                { __isa_6502_ASL, "ASL $XXXX", isa_6502_addressing_absolute },
                UNUSED_OPCODE,
                { __isa_6502_ASL, "ASL $XX,X", isa_6502_addressing_zeropage_x_indexed },
                UNUSED_OPCODE,
                { __isa_6502_ASL, "ASL $XXXX,X", isa_6502_addressing_absolute_x_indexed }
            },{
                /* A = 1 */
                UNUSED_OPCODE,
                { __isa_6502_ROL, "ROL $XX", isa_6502_addressing_zeropage },
                { __isa_6502_ROL, "ROL A", isa_6502_addressing_accumulator },
                { __isa_6502_ROL, "ROL $XXXX", isa_6502_addressing_absolute },
                UNUSED_OPCODE,
                { __isa_6502_ROL, "ROL $XX,X", isa_6502_addressing_zeropage_x_indexed },
                UNUSED_OPCODE,
                { __isa_6502_ROL, "ROL $XXXX,X", isa_6502_addressing_absolute_x_indexed }
            },{
                /* A = 2 */
                UNUSED_OPCODE,
                { __isa_6502_LSR, "LSR $XX", isa_6502_addressing_zeropage },
                { __isa_6502_LSR, "LSR A", isa_6502_addressing_accumulator },
                { __isa_6502_LSR, "LSR $XXXX", isa_6502_addressing_absolute },
                UNUSED_OPCODE,
                { __isa_6502_LSR, "LSR $XX,X", isa_6502_addressing_zeropage_x_indexed },
                UNUSED_OPCODE,
                { __isa_6502_LSR, "LSR $XXXX,X", isa_6502_addressing_absolute_x_indexed }
            },{
                /* A = 3 */
                UNUSED_OPCODE,
                { __isa_6502_ROR, "ROR $XX", isa_6502_addressing_zeropage },
                { __isa_6502_ROR, "ROR #$XX", isa_6502_addressing_accumulator },
                { __isa_6502_ROR, "ROR $XXXX", isa_6502_addressing_absolute },
                UNUSED_OPCODE,
                { __isa_6502_ROR, "ROR $XX,X", isa_6502_addressing_zeropage_x_indexed },
                UNUSED_OPCODE,
                { __isa_6502_ROR, "ROR $XXXX,X", isa_6502_addressing_absolute_x_indexed }
            },{
                /* A = 4 */
                UNUSED_OPCODE,
                { __isa_6502_STX, "STX $XX", isa_6502_addressing_zeropage },
                { __isa_6502_TXA, "TXA", isa_6502_addressing_implied },
                { __isa_6502_STX, "STX $XXXX", isa_6502_addressing_absolute },
                UNUSED_OPCODE,
                { __isa_6502_STX, "STX $XX,Y", isa_6502_addressing_zeropage_y_indexed },
                { __isa_6502_TXS, "TXS", isa_6502_addressing_implied },
                UNUSED_OPCODE
            },{
                /* A = 5 */
                { __isa_6502_LDX, "LDX #$XX", isa_6502_addressing_immediate },
                { __isa_6502_LDX, "LDX $XX", isa_6502_addressing_zeropage },
                { __isa_6502_TAX, "TAX", isa_6502_addressing_implied },
                { __isa_6502_LDX, "LDX $XXXX", isa_6502_addressing_absolute },
                UNUSED_OPCODE,
                { __isa_6502_LDX, "LDX $XX,Y", isa_6502_addressing_zeropage_y_indexed },
                { __isa_6502_TSX, "TSX", isa_6502_addressing_implied },
                { __isa_6502_LDX, "LDX $XXXX,Y", isa_6502_addressing_absolute_y_indexed }
            },{
                /* A = 6 */
                UNUSED_OPCODE,
                { __isa_6502_DEC, "DEC $XX", isa_6502_addressing_zeropage },
                { __isa_6502_DEX, "DEX", isa_6502_addressing_implied },
                { __isa_6502_DEC, "DEC $XXXX", isa_6502_addressing_absolute },
                UNUSED_OPCODE,
                { __isa_6502_DEC, "DEC $XX,X", isa_6502_addressing_zeropage_x_indexed },
                UNUSED_OPCODE,
                { __isa_6502_DEC, "DEC $XXXX,X", isa_6502_addressing_absolute_x_indexed }
            },{
                /* A = 7 */
                UNUSED_OPCODE,
                { __isa_6502_INC, "INC $XX", isa_6502_addressing_zeropage },
                { __isa_6502_NOP, "NOP", isa_6502_addressing_implied },
                { __isa_6502_INC, "INC $XXXX", isa_6502_addressing_absolute },
                UNUSED_OPCODE,
                { __isa_6502_INC, "INC $XX,X", isa_6502_addressing_zeropage_x_indexed },
                UNUSED_OPCODE,
                { __isa_6502_INC, "INC $XXXX,X", isa_6502_addressing_absolute_x_indexed }
            }
        }
    };

#undef UNUSED_OPCODE

//

isa_6502_table_t*
isa_6502_table_alloc(void)
{
    return (isa_6502_table_t*)malloc(sizeof(isa_6502_table_t));
}

//

isa_6502_table_t*
isa_6502_table_init(
    isa_6502_table_t    *isa_table,
     isa_6502_dialect_t use_dialect
)
{
    switch ( use_dialect ) {
        
        case isa_6502_dialect_base: {
            if ( ! isa_table ) isa_table = isa_6502_table_alloc();
            if ( isa_table ) {
                memcpy(&isa_table->table, &__isa_6502_table, sizeof(__isa_6502_table));
            }
            break;
        }
        
        default:
            isa_table = NULL;
            break;
            
    }
    return isa_table;
}

//

void
isa_6502_table_free(
    isa_6502_table_t    *isa_table
)
{
    free((void*)isa_table);
}

//

isa_6502_opcode_dispatch_t*
isa_6502_table_lookup_dispatch(
    isa_6502_table_t    *isa_table,
    isa_6502_opcode_t   *opcode
)
{
    return &isa_table->table[opcode->FIELDS.C][opcode->FIELDS.A][opcode->FIELDS.B];
}

//

#ifdef ENABLE_ISA_6502_TEST

#include <stdio.h>

/*
 * LDA #$25
 * STA $0200
 * LDA #$30
 * STA $0201
 * LDA #$24
 * STA $0202
 * 
 * LDA #$c0  ;Load the hex value $c0 into the A register
 * TAX       ;Transfer the value in the A register to X
 * INX       ;Increment the value in the X register
 * ADC #$c4  ;Add the hex value $c4 to the A register
 * ADC #$00  ;Add the carry bit
 * BPL next
 * DEX
 *next:
 * SED
 * LDA #$17
 * ADC #$14
 * SEC
 * SBC #$18
 * CLD
 *
 * LDX #$08
 *decrement:
 * DEX
 * STX $0203
 * CPX #$03
 * BNE decrement
 * STX $0204
 *
 * LDA #$04
 * STA $02
 * LDA #$02
 * STA $03
 * LDX #$02
 * LDA ($00,X)
 * LDA #$FF
 * STA ($00,X)
 *
 * PHA
 * PHP
 * PLA
*/
static uint8_t hello_world[] = { 0xa9, 0x25, 0x8d, 0x00, 0x02, 0xa9, 0x30, 
                                 0x8d, 0x01, 0x02, 0xa9, 0x24, 0x8d, 0x02,
                                 0x02, 0xa9, 0xc0, 0xaa, 0xe8, 0x69, 0xc4,
                                 0x69, 0x00, 0x10, 0x01, 0xca, 0xf8, 0xa9,
                                 0x17, 0x69, 0x14, 0x38, 0xe9, 0x18, 0xd8,
                                 0xa2, 0x08, 0xca, 0x8e, 0x03, 0x02, 0xe0,
                                 0x03, 0xd0, 0xf8, 0x8e, 0x04, 0x02, 0xa9,
                                 0x04, 0x85, 0x02, 0xa9, 0x02, 0x85, 0x03,
                                 0xa2, 0x02, 0xa1, 0x00, 0xa9, 0xff, 0x81,
                                 0x00, 0x48, 0x08, 0x68 };
static size_t hello_world_len = sizeof(hello_world);

int
main()
{
    registers_t                 cpu_registers;
    memory_t                    ram;
    isa_6502_table_t            isa;
    uint16_t                    PC_end = 0x0600 + hello_world_len;
    uint64_t                    total_cycles = 0;
    
    registers_init(&cpu_registers);
    memory_init(&ram);
    isa_6502_table_init(&isa, isa_6502_dialect_base);
    
    /* Install program code: */
    memcpy(&ram.RAM.PAGES[6][0], hello_world, hello_world_len);
    
    /* Load program counter with 0x0600: */
    cpu_registers.PC = 0x0600;
    
    printf("REGISTERS:      ");registers_fprintf(&cpu_registers, stdout);
    printf("MEMORY:         "); memory_fprintf(&ram, stdout, 0x0000, 0x0003);
    printf("MEMORY:         "); memory_fprintf(&ram, stdout, 0x0200, 0x0204);
    printf("\n");
    
    /* Loop: */
    while ( cpu_registers.PC < PC_end ) {
        /* Read instruction: */
        isa_6502_instr_context_t    instr_context = {
                                            .cycle_count = 0,
                                            .memory = &ram,
                                            .registers = &cpu_registers
                                        };
        isa_6502_opcode_dispatch_t  *dispatch;
        isa_6502_instr_stage_t      next_stage;
        
        instr_context.opcode.BYTE = memory_read(&ram, cpu_registers.PC++);
        printf("FETCHED:        %02hhX => %d %d %d\n", instr_context.opcode.BYTE,
                instr_context.opcode.FIELDS.A, instr_context.opcode.FIELDS.B, instr_context.opcode.FIELDS.C);
        dispatch = &isa.table[instr_context.opcode.FIELDS.C][instr_context.opcode.FIELDS.A][instr_context.opcode.FIELDS.B];
        printf("DECODED:        \"%s\" (ADDRESSING MODE = %d)\n", dispatch->description, dispatch->addressing_mode);
        
        instr_context.addressing_mode = dispatch->addressing_mode;
        instr_context.cycle_count++, total_cycles++;
        
        do {
            next_stage = dispatch->callback_fn(&instr_context, isa_6502_instr_stage_next_cycle);
            instr_context.cycle_count++, total_cycles++;
        } while ( next_stage == isa_6502_instr_stage_next_cycle);
        printf("ELAPSED CYCLES: %llu\n", instr_context.cycle_count);
        printf("REGISTERS:      ");registers_fprintf(&cpu_registers, stdout);
        printf("MEMORY:         "); memory_fprintf(&ram, stdout, 0x0000, 0x0003);
        printf("MEMORY:         "); memory_fprintf(&ram, stdout, 0x0200, 0x0204);
        printf("\n");
    }
    printf("TOTAL CYCLES:   %llu\n", total_cycles);
    
    return 0;
}

#endif /* HAVE_ISA_6502_TEST */
