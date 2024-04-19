
#include "executor.h"

#include <getopt.h>
#include <ctype.h>
#include <sys/time.h>

//

void
our_executor_stage_callback(
    executor_t                  *the_executor,
    isa_6502_instr_stage_t      the_stage,
    isa_6502_opcode_t           the_opcode,
    isa_6502_addressing_t       the_addressing_mode,
    isa_6502_opcode_dispatch_t  *the_dispatch, 
    uint64_t                    the_cycle_count,
    const char                  *disasm,
    int                         disasm_len
)
{
    #define REGISTERS       the_executor->registers
    #define MEMORY          the_executor->memory
    #define ISA             the_executor->isa
    
    executor_stage_callback_default(the_executor, the_stage, the_opcode, the_addressing_mode, the_dispatch, the_cycle_count, disasm, disasm_len);
    
    switch ( the_stage ) {
            
        case isa_6502_instr_stage_end:
            printf("[%04X] MEMORY:         ", the_stage); membus_fprintf(MEMORY, stdout, 0, memory_addr_range_with_lo_and_hi(0x0000, 0x000F));
            break;
        
        case isa_6502_instr_stage_disasm:
            if ( disasm && disasm_len ) printf("[%04X] DISASM:         %s\n", the_stage, disasm);
            break;
            
        case isa_6502_instr_stage_enter_nmi:
            printf("[--->] ENTER NMI\n");
            break;
            
        case isa_6502_instr_stage_enter_irq:
            printf("[--->] ENTER IRQ\n");
            break;
            
        case isa_6502_instr_stage_exec_nmi:
            printf("[<---] EXEC NMI @ $%04hX\n", REGISTERS->PC);
            break;
            
        case isa_6502_instr_stage_exec_irq:
            printf("[<---] EXEC IRQ @ $%04hX\n", REGISTERS->PC);
            break;
            
    }
    
    #undef REGISTERS
    #undef MEMORY
    #undef ISA
}

//

const struct option cli_options[] = {
        { "help",       no_argument,            NULL,   'h' },
        { "version",    no_argument,            NULL,   'V' },
        { "bload",      required_argument,      NULL,   'l' },
        { "save",       required_argument,      NULL,   's' },
        { "poke",       required_argument,      NULL,   'p' },
        { "dump",       required_argument,      NULL,   'd' },
        { "new-vm",     no_argument,            NULL,   'n' },
        { "reset",      no_argument,            NULL,   'R' },
        { "boot",       no_argument,            NULL,   'b' },
        { "exec",       required_argument,      NULL,   'x' },
        { "verbose",    no_argument,            NULL,   'v' },
        { "quiet",      no_argument,            NULL,   'q' },
        { "irq+nmi",    no_argument,            NULL,   '1' },
        { "isa",        required_argument,      NULL,   'i' },
        { "disasm",     required_argument,      NULL,   'D' },
        { "brun",       required_argument,      NULL,   'r' },
        { "mode",       required_argument,      NULL,   'm' },
        { NULL,         no_argument,            NULL,    0  }
    };
const char *cli_options_str = "hVl:s:p:d:nRbx:vq1i:D:r:m:";

//

void
usage(
    const char  *exe
)
{
    printf(
        "usage:\n"
        "\n"
        "    %s {options}\n"
        "\n"
        "  basic options:\n"
        "\n"
        "    --help/-h                      show this help\n"
        "\n"
        "  data manipulation options:\n"
        "\n"
        "    --bload/-l <file-spec>         fill memory with bytes from a file\n"
        "    --save/-s {+}<file-spec>       write contents of memory to a file; a leading\n"
        "                                   plus sign indicates append to the file\n"
        "    --poke/-p <fill-spec>          write a byte or word to a memory region\n"
        "    --dump/-d <mem-spec>           do a hexdump of a memory region\n"
        "    --disasm/-D <mem-spec>         do a static disassembly of a memory region\n"
        "\n"
        "  execution options:\n"
        "\n"
        "    --new-vm/-n                    destroy and recreate the VM\n"
        "    --isa/-i <isa-dialect>         choose the 6502 dialect; default is 6502\n"
        "    --reset/-R                     reset the virtual machine\n"
        "    --boot/-b                      execute a boot from the RES vector\n"
        "    --exec/-x <mem-spec>           execute instructions in the specified\n"
        "                                   memory region\n"
        "    --verbose/-v                   display verbose execution status\n"
        "                                   (the default)\n"
        "    --quiet/-q                     display as little execution status as\n"
        "                                   possible\n"
        "    --irq+nmi/-1                   display only IRQ and DMA events\n"
        "    --mode/-m <exec-mode>          select execution mode options; the default\n"
        "                                   is +staged,+verbose,+locks\n"
        "\n"
        "  combined options:\n"
        "\n"
        "    --brun/-r <file>{@<addr>}     load all bytes from the given file either at 0x2000\n"
        "                                  or the given address and execute in from the base\n"
        "                                  address through the last byte loaded\n"
        "\n"
        "    <addr> = $X{X..} | 0xX{X..} | 0#{#..} | #{#..} | IRQ{±#} | NMI{±#} | RES{±#}\n"
        "    <len> = $X{X..} | 0xX{X..} | 0#{#..} | #{#..} | *\n"
        "    <mem-spec> = <addr>{:<len>} | <addr>{-<addr>}\n"
        "    <file-spec> = <file-path>@<mem-spec>\n"
        "    <byte-word> = $XX | $XXXX | 0xXX | 0xXXXX\n"
        "    <fill-spec> = <byte-word>@<mem-spec>\n"
        "    <isa-dialect> = 6502 | 65C02\n"
        "    <exec-mode-opt> = {+|-}staged | {+|-}verbose} | {+|-}locks\n"
        "    <exec-mode> = {<exec-mode-opt>}{,<exec-mode-opt>{,...}}\n"
        "\n",
        exe
    );
}

//

const char*
parse_exec_mode(
    const char                  *s,
    isa_6502_opcode_exec_mode_t *exec_mode
)
{
    isa_6502_opcode_exec_mode_t m = 0;
    
    while ( *s ) {
        bool        is_okay;
        bool        is_negated = false;
        
        if ( *s == '+' ) {
            s++;
        } else if ( *s == '-' ) {
            is_negated = true;
            s++;
        }
        if ( strncasecmp(s, "staged", 6) == 0 ) {
            if ( is_negated )
                m = (m & ~isa_6502_opcode_exec_mode_mask) | isa_6502_opcode_exec_mode_static;
            else
                m = (m & ~isa_6502_opcode_exec_mode_mask) | isa_6502_opcode_exec_mode_staged;
            s += 6;
        }
        else if ( strncasecmp(s, "static", 6) == 0 ) {
            if ( ! is_negated )
                m = (m & ~isa_6502_opcode_exec_mode_mask) | isa_6502_opcode_exec_mode_static;
            else
                m = (m & ~isa_6502_opcode_exec_mode_mask) | isa_6502_opcode_exec_mode_staged;
            s += 6;
        }
        else if ( strncasecmp(s, "verbose", 7) == 0 ) {
            if ( is_negated )
                m = (m & ~isa_6502_opcode_exec_mode_verbose);
            else
                m = (m & ~isa_6502_opcode_exec_mode_verbose) | isa_6502_opcode_exec_mode_verbose;
            s += 7;
        }
        else if ( strncasecmp(s, "normal", 6) == 0 ) {
            if ( ! is_negated )
                m = (m & ~isa_6502_opcode_exec_mode_verbose);
            else
                m = (m & ~isa_6502_opcode_exec_mode_verbose) | isa_6502_opcode_exec_mode_verbose;
            s += 6;
        }
        else if ( strncasecmp(s, "locks", 5) == 0 ) {
            if ( is_negated )
                m = (m & ~isa_6502_opcode_exec_mode_no_locking) | isa_6502_opcode_exec_mode_no_locking;
            else
                m = (m & ~isa_6502_opcode_exec_mode_no_locking);
            s += 5;
        }
        else if ( strncasecmp(s, "nolocks", 7) == 0 ) {
            if ( ! is_negated )
                m = (m & ~isa_6502_opcode_exec_mode_no_locking) | isa_6502_opcode_exec_mode_no_locking;
            else
                m = (m & ~isa_6502_opcode_exec_mode_no_locking);
            s += 7;
        }
        else {
            fprintf(stderr, "WARNING:  invalid execution mode: %s\n", s);
            return NULL;
        }
        
        // Skip past any punctuation:
        is_okay = true;
        while ( is_okay && *s ) {
            switch ( *s ) {
                case ',':
                case ' ':
                case '\t':
                case ';':
                case ':':
                    s++;
                    break;
                default:
                    is_okay = false;
                    break;
            }
        }
    }
    if ( *s ) {
        fprintf(stderr, "WARNING:  invalid execution mode: %s\n", s);
        return NULL;
    }
    *exec_mode = m;
    return s;
}

//

enum {
    system_6502_did_parse_unknown = 0,
    system_6502_did_parse_irq,
    system_6502_did_parse_nmi,
    system_6502_did_parse_res
};

int
__parse_addr_str(
    const char  **s,
    uint16_t    *addr
)
{
    if ( strncasecmp(*s, "IRQ", 3) == 0 ) {
        *s = *s + 3;
        *addr = MEMORY_ADDR_IRQ_VECTOR;
        return system_6502_did_parse_irq;
    }
    if ( strncasecmp(*s, "NMI", 3) == 0 ) {
        *s = *s + 3;
        *addr = MEMORY_ADDR_NMI_VECTOR;
        return system_6502_did_parse_nmi;
    }
    if ( strncasecmp(*s, "RES", 3) == 0 ) {
        *s = *s + 3;
        *addr = MEMORY_ADDR_RES_VECTOR;
        return system_6502_did_parse_res;
    }
    return system_6502_did_parse_unknown;
}

//

const char*
parse_addr(
    const char  *s,
    uint16_t    *addr
)
{
    const char  *s_save = s;
    int         addr_str_id;
    
    if ( *s == '$' ) {
        char            *endptr = NULL;
        unsigned long   v = strtoul(++s, &endptr, 16);
        if ( endptr > s ) {
            if ( v > 0x0000FFFF )
                fprintf(stderr, "WARNING:  invalid 16-bit address, truncating:  $%08lX\n", v);
            *addr = v & 0x0000FFFF;
            return endptr;
        }
    }
    else if ( *s >= '0' && *s <= '9' ) {
        char            *endptr = NULL;
        unsigned long   v = strtoul(s, &endptr, 0);
        if ( endptr > s ) {
            if ( v > 0x0000FFFF )
                fprintf(stderr, "WARNING:  invalid 16-bit address, truncating:  $%08lX\n", v);
            *addr = v & 0x0000FFFF;
            return endptr;
        }
    }
    else if ( (addr_str_id = __parse_addr_str(&s, addr)) !=  system_6502_did_parse_unknown ) {
        char        *endptr = NULL;
        long        v = strtol(s, &endptr, 0);
        
        if ( (endptr > s) && (v != 0) ) {
            *addr += v;
            s = endptr;
        }
        return s;
    }
    fprintf(stderr, "WARNING:  invalid address: %s\n", s_save);
    return NULL;
}

//

const char*
parse_len(
    const char  *s,
    uint16_t    *len
)
{
    const char  *s_save = s;
    
    if ( *s == '$' ) {
        char            *endptr = NULL;
        unsigned long   v = strtoul(++s, &endptr, 16);
        if ( endptr > s ) {
            if ( v > 0x0000FFFF )
                fprintf(stderr, "WARNING:  invalid 16-bit length, truncating $%08lX\n", v);
            *len = v & 0x0000FFFF;
            return endptr;
        }
    }
    else if ( *s >= '0' && *s <= '9' ) {
        char            *endptr = NULL;
        unsigned long   v = strtoul(s, &endptr, 0);
        if ( endptr > s ) {
            if ( v > 0x0000FFFF )
                fprintf(stderr, "WARNING:  invalid 16-bit length, truncating $%08lX\n", v);
            *len = v & 0x0000FFFF;
            return endptr;
        }
    }
    else if ( (*s == '*') && (*(s + 1) == '\0') ) {
        *len = 0xFFFF;
        return s + 1;
    }
    fprintf(stderr, "WARNING:  invalid length: %s\n", s_save);
    return NULL;
}

//

const char*
parse_mem_spec(
    const char  *s,
    uint16_t    *addr_start,
    uint16_t    *addr_end
)
{
    const char  *s_save = s;
    const char  *s_prime = parse_addr(s, addr_start);
    
    if ( s_prime ) {
        /* A colon means a length, a dash means an address, a NUL means single byte: */
        switch ( *s_prime ) {
            case '\0':
                *addr_end = *addr_start;
                return s_prime;
            case ':':
                s_prime = parse_len(++s_prime, addr_end);
                if ( s_prime ) {
                    uint16_t    addr = *addr_start + *addr_end - 1;
                    
                    if ( addr < *addr_start ) addr = 0xFFFF;
                    *addr_end = addr;
                    return s_prime;
                }
                break;
            case '-':
                s_prime = parse_addr(++s_prime, addr_end);
                if ( s_prime ) {
                    if ( *addr_end < *addr_start ) *addr_end = 0xFFFF;
                    return s_prime;
                }
                break;
        }
    }
    fprintf(stderr, "WARNING:  invalid memory spec: %s\n", s_save);
    return NULL;
}

//

const char*
parse_file_spec(
    const char  *s,
    char        **file_path,
    uint16_t    *addr_start,
    uint16_t    *addr_end
)
{
    const char  *s_save = s;
    char        *s_prime = strrchr(s, '@');
    
    if ( s_prime > s ) {
        const char  *s_prime_prime;
        
        /* Handle the mem spec first: */
        s_prime_prime = parse_mem_spec(s_prime + 1, addr_start, addr_end);
        if ( ! s_prime_prime ) return NULL;
        
        *file_path = (char*)malloc(s_prime - s + 1);
        if ( ! *file_path ) return NULL;
        memcpy(*file_path, s, s_prime - s);
        (*file_path)[s_prime - s] = '\0';
        return s_prime_prime;
    }
    fprintf(stderr, "WARNING:  invalid file spec: %s\n", s_save);
    return NULL;
}    

//

const char*
parse_byte_word(
    const char  *s,
    uint16_t    *value,
    size_t      *value_len
)
{
    const char  *s_save = s;
    
    if ( (*s == '$') || (*s == '0' && *(s + 1) == 'x') ) {
        char            *endptr = NULL;
        int             digits = 0;
        
        ++s;
        if ( *s == 'x' ) s++;
        endptr = (char*)s;
        while ( isxdigit(*endptr) ) endptr++;
        digits = endptr - s;
        
        if ( digits <= 4 && digits >= 1 ) {
            unsigned long   v = strtoul(s, &endptr, 16);
            if ( endptr > s ) {
                *value = v & 0x0000FFFF;
                *value_len = (digits > 2) ? 2 : 1;
                return endptr;
            }
        } else {
            fprintf(stderr, "WARNING:  invalid byte-word: too many digits: %d\n", digits);
        }
    }
    fprintf(stderr, "WARNING:  invalid byte-word: %s\n", s_save);
    return NULL;
}

//

const char*
parse_fill_spec(
    const char  *s,
    uint16_t    *value,
    size_t      *value_len,
    uint16_t    *addr_start,
    uint16_t    *addr_end
)
{
    const char  *s_save = s;
    const char  *s_prime = parse_byte_word(s, value, value_len);
    
    if ( s_prime && (*s_prime == '@') ) {
        return parse_mem_spec(++s_prime, addr_start, addr_end);
    }
    fprintf(stderr, "WARNING:  invalid memory fill spec: %s\n", s_save);
    return NULL;
}

//

const char*
parse_isa_dialect(
    const char*         s,
    isa_6502_dialect_t  *dialect
)
{
    if ( strcmp(s, "6502") == 0 ) {
        *dialect = isa_6502_dialect_base;
        return s + 4;
    }
    if ( strcasecmp(s, "65C02") == 0 ) {
        *dialect = isa_6502_dialect_65C02;
        return s + 5;
    }
    fprintf(stderr, "WARNING:  invalid 6502 dialect: %s\n", s);
    return NULL;
}

//

bool
dma_consumer(
    const void  *context,
    uint16_t    byte_index,
    uint8_t     next_byte
)
{
    printf("%04hX : %02hhX\n", byte_index, next_byte);
    return true;
}

//

bool
dma_provider(
    const void  *context,
    uint16_t    byte_index,
    uint8_t     *next_byte
)
{
    uint8_t     rng = random();
    
    printf("%04hX : %02hhX\n", byte_index, rng);
    *next_byte = rng;
    return true;
}

//

void*
tui_input_thread_run(
    void    *context
)
{
    executor_t  **the_executor = (executor_t**)context;
    bool        is_running = true;
    
    while ( is_running ) {
        int     c = fgetc(stdin);
        
        switch ( c ) {
            case '>':
                executor_set_dma_copyout(*the_executor, memory_addr_range_with_lo_and_len(0x2000, 0x0010), dma_consumer, NULL);
                break;
            case '<':
                executor_set_dma_copyin(*the_executor, memory_addr_range_with_lo_and_len(0x4000, 0x0010), dma_provider, NULL);
                break;
            case '4':
                membus_fprintf((*the_executor)->memory, stdout, 0, memory_addr_range_with_lo_and_len(0x4000, 0x0018));
                break;
            case 'N':
            case 'n':
                executor_set_nmi(*the_executor);
                break;
            case 'I':
            case 'i':
                executor_set_irq(*the_executor);
                break;
            case 'Q':
            case 'q':
                is_running = false;
                executor_set_exec_stop(*the_executor);
                break;
        }
    }
    return NULL;
}

//

int
main(
    int                         argc,
    char* const                 argv[]
)
{
    int                         rc = 0, optch;
    executor_t                  *the_vm = executor_alloc_with_default_components();
    isa_6502_dialect_t          the_dialect = isa_6502_dialect_base;
    executor_stage_callback_t   exec_callback = our_executor_stage_callback;
    isa_6502_opcode_exec_mode_t exec_mode = isa_6502_opcode_exec_mode_default;
    isa_6502_instr_stage_t      exec_callback_event_mask = isa_6502_instr_stage_all;
    pthread_t                   tui_input_thread;
    
    srandom(time(NULL));
    
    pthread_create(&tui_input_thread, NULL, tui_input_thread_run, &the_vm);
    
    isa_6502_table_init(the_vm->isa, the_dialect);
    
    while ( (optch = getopt_long(argc, argv, cli_options_str, cli_options, NULL)) != -1 ) {
        switch ( optch ) {
            case 'h':
                usage(argv[0]);
                exit(0);
            
            case 'V': 
                printf("%s\n", SYSTEM_6502_NAME " version " SYSTEM_6502_VERSION);
                exit(0);
            
            case 'n':
                printf("INFO:  shutting down the VM...\n");
                executor_free(the_vm);
                printf("INFO:  ...starting a new VM\n");
                the_vm = executor_alloc_with_default_components();
                isa_6502_table_init(the_vm->isa, the_dialect);
                break;
            
            case 'R':
                printf("INFO:  hardware reset of the VM...\n");
                executor_hard_reset(the_vm);
                break;
            
            case 'b':
                printf("INFO:  software reset (from RES vector)...\n");
                executor_soft_reset(
                        the_vm,
                        exec_mode,
                        exec_callback,
                        exec_callback_event_mask
                    );
                break;
            
            case 'x': {
                uint16_t        addr_start, addr_end;
                
                if ( parse_mem_spec(optarg, &addr_start, &addr_end) ) {
                    struct timeval  t0, t1;
                    uint64_t        total_cycles;
                    double          cycles_per_sec;
                    
                    printf("INFO:  executing from $%04hX-$%04hX\n", addr_start, addr_end);
                    gettimeofday(&t0, NULL);
                    total_cycles = executor_launch_in_address_range(
                                            the_vm,
                                            exec_mode,
                                            exec_callback,
                                            exec_callback_event_mask,
                                            memory_addr_range_with_lo_and_hi(addr_start, addr_end),
                                            addr_start
                                        );
                    gettimeofday(&t1, NULL);
                    cycles_per_sec = (double)total_cycles / ((double)(t1.tv_sec - t0.tv_sec) + 1e-6 * (double)(t1.tv_usec - t0.tv_usec));
                    printf("INFO:  %.2lg cycles per second\n", cycles_per_sec);
                }
                break;
            }
            
            case 'v':
                exec_callback = our_executor_stage_callback;
                exec_callback_event_mask = isa_6502_instr_stage_all;
                break;
            case 'q':
                exec_callback = NULL;
                exec_callback_event_mask = 0;
                break;
            case '1':
                exec_callback = our_executor_stage_callback;
                exec_callback_event_mask = isa_6502_instr_stage_enter_nmi | isa_6502_instr_stage_enter_irq | isa_6502_instr_stage_exec_nmi | isa_6502_instr_stage_exec_irq;
                break;
            
            case 'l': {
                char        *file_path = NULL;
                uint16_t    addr_start, addr_end;
                
                if ( parse_file_spec(optarg, &file_path, &addr_start, &addr_end) ) {
                    int         bin_fd = open(file_path, O_RDONLY);
                    
                    if ( bin_fd ) {
                        ssize_t did_read = membus_load_from_fd(
                                                the_vm->memory,
                                                memory_addr_range_with_lo_and_hi(addr_start, addr_end),
                                                bin_fd);
                        
                        printf("INFO:  read %ld ($%04hX) bytes into memory range $%04hX-$%04hX\n",
                                did_read,
                                (uint16_t)did_read,
                                addr_start,
                                addr_start + (uint16_t)did_read - 1
                            );
                        close(bin_fd);
                    }
                    free(file_path);
                }
                break;
            }
            
            case 'r': {
                char        *file_path_copy, *file_path_end = optarg;
                uint16_t    addr_start;
                bool        is_ok = true;
                
                while ( *file_path_end && (*file_path_end != '@') ) file_path_end++;
                if ( *file_path_end == '@' ) {
                    file_path_copy = (char*)malloc(file_path_end - optarg + 1);
                    if ( file_path_copy ) {
                        memcpy(file_path_copy, optarg, file_path_end - optarg);
                        file_path_copy[file_path_end - optarg ] = '\0';
                        if ( ! parse_addr(file_path_end + 1, &addr_start) ) is_ok = false;
                    }
                } else {
                    addr_start = 0x2000;
                    file_path_copy = optarg;
                }
                if ( is_ok && file_path_copy && *file_path_copy ) {
                    int         bin_fd = open(file_path_copy, O_RDONLY);
                    
                    if ( bin_fd ) {
                        ssize_t             did_read = membus_load_from_fd(
                                                            the_vm->memory,
                                                            memory_addr_range_with_lo_and_hi(addr_start, 0xFFFF),
                                                            bin_fd);
                        
                        printf("INFO:  read %ld ($%04hX) bytes into memory range $%04hX-$%04hX\n",
                                did_read,
                                (uint16_t)did_read,
                                addr_start,
                                addr_start + (uint16_t)did_read - 1
                            );
                        close(bin_fd);
                        if ( did_read > 0 ) {
                            struct timeval          t0, t1;
                            uint64_t                total_cycles;
                            double                  cycles_per_sec;
                            memory_addr_range_t     exec_range = memory_addr_range_with_lo_and_len(addr_start, did_read);
                            
                            printf("INFO:  executing code in memory range $%04hX-$%04hX\n", addr_start, memory_addr_range_get_end(&exec_range));
                            gettimeofday(&t0, NULL);
                            total_cycles = executor_launch_in_address_range(
                                                    the_vm,
                                                    exec_mode,
                                                    exec_callback,
                                                    exec_callback_event_mask,
                                                    exec_range,
                                                    addr_start
                                                );
                            gettimeofday(&t1, NULL);
                            cycles_per_sec = (double)total_cycles / ((double)(t1.tv_sec - t0.tv_sec) + 1e-6 * (double)(t1.tv_usec - t0.tv_usec));
                            printf("INFO:  %.2lg cycles per second\n", cycles_per_sec);
                        }
                    }
                }
                if ( file_path_copy && (file_path_copy != optarg) ) free(file_path_copy);
                break;
            }
            
            case 's': {
                char        *file_path = NULL;
                uint16_t    addr_start, addr_end;
                int         should_append = 0;
                
                if ( *optarg == '+' ) {
                    should_append = 1;
                    optarg++;
                }
                if ( parse_file_spec(optarg, &file_path, &addr_start, &addr_end) ) {
                    int         bin_fd = open(file_path, O_WRONLY | O_CREAT | (should_append ? O_APPEND : O_TRUNC), 0666);
                    
                    if ( bin_fd ) {
                        ssize_t did_write = membus_save_to_fd(
                                                    the_vm->memory,
                                                    memory_addr_range_with_lo_and_hi(addr_start, addr_end),
                                                    bin_fd);
                        
                        printf("INFO:  wrote %ld ($%04hX) bytes from memory range $%04hX-$%04hX\n",
                                did_write,
                                (uint16_t)did_write,
                                addr_start,
                                addr_start + (uint16_t)did_write - 1
                            );
                        close(bin_fd);
                    }
                    free(file_path);
                }
                break;
            }
            
            case 'p': {
                uint16_t        addr_start, addr_end;
                uint16_t        fill_value;
                size_t          fill_size;
                
                if ( parse_fill_spec(optarg, &fill_value, &fill_size, &addr_start, &addr_end) ) {
                    switch ( fill_size ) {
                        case 1: {
                            membus_write_byte_to_range(the_vm->memory, memory_addr_range_with_lo_and_hi(addr_start, addr_end), fill_value);
                            break;
                        }
                        case 2: {
                            membus_write_word_to_range(the_vm->memory, memory_addr_range_with_lo_and_hi(addr_start, addr_end), fill_value);
                            break;
                        }
                    }
                }
                break;
            }
            
            case 'd': {
                uint16_t        addr_start, addr_end;
                
                if ( parse_mem_spec(optarg, &addr_start, &addr_end) ) {
                    membus_fprintf(the_vm->memory, stdout, 0, memory_addr_range_with_lo_and_hi(addr_start, addr_end));
                }
                break;
            }
            
            case 'D': {
                uint16_t        addr_start, addr_end;
                
                if ( parse_mem_spec(optarg, &addr_start, &addr_end) ) {
                    isa_6502_static_disassembly(the_vm->isa, the_vm->memory, memory_addr_range_with_lo_and_hi(addr_start, addr_end), stdout);
                }
                break;
            }
            
            case 'i':
                if ( parse_isa_dialect(optarg, &the_dialect) ) {
                    isa_6502_table_init(the_vm->isa, the_dialect);
                }
                break;
                
            case 'm':
                if ( ! parse_exec_mode(optarg, &exec_mode) ) exit(EINVAL);
                printf("INFO:  Execution mode %08X\n", exec_mode);
                break;
        }
    }
    return rc;
}
