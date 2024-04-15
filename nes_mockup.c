
#include "executor.h"

#include "libmembus/membus_module_paged.h"
#include "libmembus/membus_module_mirror.h"
#include "libmembus/membus_private.h"

//

const char      *ppu_register_names[] = {
                        "PPUCTRL",
                        "PPUMASK",
                        "PPUSTATUS",
                        "OAMADDR",
                        "OAMDATA",
                        "PPUSCROLL",
                        "PPUADDR",
                        "PPUDATA",
                        "OAMDMA",
                        NULL
                    };

//

typedef struct membus_module_PPU_registers {
    membus_module_t     header;
} membus_module_PPU_registers_t;

//

membus_module_op_result_t
__membus_module_PPU_registers_read_addr(
    membus_module_ref	module,
    uint16_t            addr,
    uint8_t             *value
)
{
    membus_module_PPU_registers_t   *MODULE = (membus_module_PPU_registers_t*)module;
    
    // Only 8 addresses matter (they repeat across the whole range):
    addr = (addr & 0x0007);
    printf("    [PPU] %s: READ \"%s\"\n", __func__, ppu_register_names[addr]);
    return membus_module_op_result_accepted;
}

//

membus_module_op_result_t
__membus_module_PPU_registers_write_addr(
    membus_module_ref	module,
    uint16_t            addr,
    uint8_t             value
)
{
    membus_module_PPU_registers_t   *MODULE = (membus_module_PPU_registers_t*)module;
    
    // Only 8 addresses matter (they repeat across the whole range):
    addr = (addr & 0x0007);
    printf("    [PPU] %s: WRITE \"%s\"\n", __func__, ppu_register_names[addr]);
    return membus_module_op_result_accepted;
}

//

static const membus_module_t membus_module_PPU_registers_header = {
                .module_id = "PPUREG",
                .addr_range = { .addr_lo = 0x2000, .addr_len = 0x2000 },
                .free_callback = NULL,
                .read_callback = __membus_module_PPU_registers_read_addr,
                .write_callback = __membus_module_PPU_registers_write_addr
            };

//

membus_module_ref
membus_module_PPU_registers_alloc()
{
    membus_module_PPU_registers_t   *new_module = (membus_module_PPU_registers_t*)malloc(sizeof(membus_module_PPU_registers_t));
    
    if ( new_module ) {
        new_module->header = membus_module_PPU_registers_header;
    }
    return (membus_module_ref)new_module;
}

//

const char      *apu_register_names[] = {
                        "PULSE1_DDLCVVVV",
                        "PULSE1_EPPPNSS",
                        "PULSE1_TTTTTTTT",
                        "PULSE1_LLLLLTTT",
                        "PULSE2_DDLCVVVV",
                        "PULSE2_EPPPNSS",
                        "PULSE2_TTTTTTTT",
                        "PULSE2_LLLLLTTT",
                        "TRIANGLE_CRRRRRRR",
                        "TRIANGLE_UNUNSED",
                        "TRIANGLE_TTTTTTTT",
                        "TRIANGLE_LLLLLTTT",
                        "NOISE_--LCVVVV",
                        "NOISE_UNUSED",
                        "NOISE_L---PPPP",
                        "NOISE_LLLLL---",
                        "DMC_IL--RRRR",
                        "DMC_-DDDDDDD",
                        "DMC_AAAAAAAA",
                        "DMC_LLLLLLLL",
                        "",
                        "STATUS_---DNT21",
                        "",
                        "FRAMECOUNTER_MI------",
                        "",
                        "",
                        "",
                        "",
                        "",
                        "",
                        "",
                        "",
                        NULL
                    };

//

typedef struct membus_module_APU_registers {
    membus_module_t     header;
} membus_module_APU_registers_t;

//

membus_module_op_result_t
__membus_module_APU_registers_read_addr(
    membus_module_ref	module,
    uint16_t            addr,
    uint8_t             *value
)
{
    membus_module_APU_registers_t   *MODULE = (membus_module_APU_registers_t*)module;
    
    // Only 8 addresses matter (they repeat across the whole range):
    addr = (addr & 0x001F);
    printf("    [APU] %s: READ \"%s\"\n", __func__, apu_register_names[addr]);
    return membus_module_op_result_accepted;
}

//

membus_module_op_result_t
__membus_module_APU_registers_write_addr(
    membus_module_ref	module,
    uint16_t            addr,
    uint8_t             value
)
{
    membus_module_APU_registers_t   *MODULE = (membus_module_APU_registers_t*)module;
    
    // Only 8 addresses matter (they repeat across the whole range):
    addr = (addr & 0x001F);
    printf("    [APU] %s: WRITE \"%s\"\n", __func__, apu_register_names[addr]);
    return membus_module_op_result_accepted;
}

//

static const membus_module_t membus_module_APU_registers_header = {
                .module_id = "APUREG",
                .addr_range = { .addr_lo = 0x4000, .addr_len = 0x0020 },
                .free_callback = NULL,
                .read_callback = __membus_module_APU_registers_read_addr,
                .write_callback = __membus_module_APU_registers_write_addr
            };

//

membus_module_ref
membus_module_APU_registers_alloc()
{
    membus_module_APU_registers_t   *new_module = (membus_module_APU_registers_t*)malloc(sizeof(membus_module_APU_registers_t));
    
    if ( new_module ) {
        new_module->header = membus_module_APU_registers_header;
    }
    return (membus_module_ref)new_module;
}

//

int
main()
{
    membus_t            *nes_memory = membus_alloc();
    int                 idx = 1;
    
    // The base 2 KiB is a paged module:
    membus_module_ref   base_2K = membus_module_paged_alloc(membus_module_mode_rw, 0x00, 0x08);
    
    // There are 3 subsequent mirrors of the base 2 KiB:
    membus_module_ref   base_2K_mirrors[3] = {
                                membus_module_mirror_alloc(base_2K, 0x0800),
                                membus_module_mirror_alloc(base_2K, 0x1000),
                                membus_module_mirror_alloc(base_2K, 0x1800)
                            };
    
    // The PPU registers are 8 bytes from 0x2000 - 0x2007.  But they're
    // mirrored 0x2008 - 0x3FFFF, too.  Rather than make 1023 mirrors, we
    // build the mirroring into the membus module:
    membus_module_ref   ppu_registers = membus_module_PPU_registers_alloc();
    
    // The APU registers are 32 bytes from 0x4000 - 0x401F.  No mirroring
    // involved.
    membus_module_ref   apu_registers = membus_module_APU_registers_alloc();
    
    printf("%02d. MAIN BUS AND ALL MEMBUS MODULES ALLOCATED\n", idx++);
    
    // Register all membus modules with the bus:
    printf("%02d. REGISTERING ALL MEMBUS MODULES WITH MAIN BUS\n", idx++);
    membus_register_module(nes_memory, 0, base_2K);
    membus_register_module(nes_memory, 0, base_2K_mirrors[0]);
    membus_register_module(nes_memory, 0, base_2K_mirrors[1]);
    membus_register_module(nes_memory, 0, base_2K_mirrors[2]);
    membus_register_module(nes_memory, 0, ppu_registers);
    membus_register_module(nes_memory, 0, apu_registers);
    
    // Do some tests:
    printf("%02d. READ $2000\n", idx++);
    membus_read_addr(nes_memory, 0x2000);
    
    printf("%02d. WRITE $2001\n", idx++);
    membus_write_addr(nes_memory, 0x2001, 0xFF);
    
    membus_write_addr(nes_memory, 0x0000, 0xCA);
    membus_write_addr(nes_memory, 0x0001, 0xFE);
    printf("%02d. WROTE {0xCA, 0xFE} to $0000\n", idx++);
    printf("%02d. READ $0000 = %02hhX\n", idx++, membus_read_addr(nes_memory, 0x0000));
    printf("%02d. READ $0001 = %02hhX\n", idx++, membus_read_addr(nes_memory, 0x0001));
    printf("%02d. READ $0800 = %02hhX\n", idx++, membus_read_addr(nes_memory, 0x0800));
    printf("%02d. READ $0801 = %02hhX\n", idx++, membus_read_addr(nes_memory, 0x0801));
    printf("%02d. READ $1800 = %02hhX\n", idx++, membus_read_addr(nes_memory, 0x1800));
    printf("%02d. READ $1801 = %02hhX\n", idx++, membus_read_addr(nes_memory, 0x1801));
    membus_write_addr(nes_memory, 0x17FE, 0xFE);
    membus_write_addr(nes_memory, 0x17FF, 0xED);
    printf("%02d. WROTE {0xFE, 0xED} to $17FE\n", idx++);
    printf("%02d. READ $07FE = %02hhX\n", idx++, membus_read_addr(nes_memory, 0x07FE));
    printf("%02d. READ $07FF = %02hhX\n", idx++, membus_read_addr(nes_memory, 0x07FF));
    printf("%02d. READ $1FFE = %02hhX\n", idx++, membus_read_addr(nes_memory, 0x1FFE));
    printf("%02d. READ $1FFF = %02hhX\n", idx++, membus_read_addr(nes_memory, 0x1FFF));
    
    printf("%02d. READ $4000\n", idx++);
    membus_read_addr(nes_memory, 0x4000);
    
    printf("%02d. WRITE $4012\n", idx++);
    membus_write_addr(nes_memory, 0x4012, 0xFF);
    
    printf("%02d. WRITE $4015\n", idx++);
    membus_write_addr(nes_memory, 0x4015, 0xFF);
    
    // Destroy the bus:
    printf("%02d. DEALLOCATING MAIN BUS\n", idx++);
    membus_free(nes_memory);
    
    // Release our own references to the modules:
    printf("%02d. DEALLOCATING MEMBUS MODULES\n", idx++);
    membus_module_release(base_2K);
    membus_module_release(base_2K_mirrors[0]);
    membus_module_release(base_2K_mirrors[1]);
    membus_module_release(base_2K_mirrors[2]);
    membus_module_release(ppu_registers);
    
    return 0;
}


