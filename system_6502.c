
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
            printf("[%04X] MEMORY:         ", the_stage); memory_fprintf(MEMORY, stdout, 0, 0x0000, 0x000F);
            break;
        
        case isa_6502_instr_stage_disasm:
            if ( disasm && disasm_len ) printf("[%04X] DISASM:         %s\n", the_stage, disasm);
            break;
            
        case isa_6502_instr_stage_nmi:
            printf("CAUGHT NMI\n");
            break;
            
        case isa_6502_instr_stage_irq:
            printf("CAUGHT IRQ\n");
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
        { "load",       required_argument,      NULL,   'l' },
        { "save",       required_argument,      NULL,   's' },
        { "poke",       required_argument,      NULL,   'p' },
        { "dump",       required_argument,      NULL,   'd' },
        { "new-vm",     no_argument,            NULL,   'n' },
        { "reset",      no_argument,            NULL,   'r' },
        { "boot",       no_argument,            NULL,   'b' },
        { "exec",       required_argument,      NULL,   'x' },
        { "verbose",    no_argument,            NULL,   'v' },
        { "quiet",      no_argument,            NULL,   'q' },
        { "irq+nmi",    no_argument,            NULL,   '1' },
        { NULL,         no_argument,            NULL,    0  }
    };
const char *cli_options_str = "hVl:s:p:d:nrbx:vq1";

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
        "    --load/-l <file-spec>          fill memory with bytes from a file\n"
        "    --save/-s {+}<file-spec>       write contents of memory to a file; a leading\n"
        "                                   plus sign indicates append to the file\n"
        "    --poke/-p <fill-spec>          write a byte or word to a memory region\n"
        "    --dump/-d <mem-spec>           do a hexdump of a memory region\n"
        "\n"
        "  execution options:\n"
        "\n"
        "    --new-vm/-n                    destroy and recreate the VM\n"
        "    --reset/-r                     reset the virtual machine\n"
        "    --boot/-b                      execute a boot from the RES vector\n"
        "    --exec/-x <mem-spec>           execute instructions in the specified\n"
        "                                   memory region\n"
        "    --verbose/-v                   display verbose execution status\n"
        "                                   (the default)\n"
        "    --quiet/-q                     display as little execution status as\n"
        "                                   possible\n"
        "\n"
        "    <addr> = $X{X..} | 0xX{X..} | 0#{#..} | #{#..} | IRQ{±#} | NMI{±#} | RES{±#}\n"
        "    <len> = $X{X..} | 0xX{X..} | 0#{#..} | #{#..} | *\n"
        "    <mem-spec> = <addr>{:<len>} | <addr>{-<addr>}\n"
        "    <file-spec> = <file-path>@<mem-spec>\n"
        "    <byte-word> = $XX | $XXXX | 0xXX | 0xXXXX\n"
        "    <fill-spec> = <byte-word>@<mem-spec>\n"
        "\n",
        exe
    );
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
            case 'N':
            case 'n':
                printf("Setting NMI\n");
                executor_set_nmi(*the_executor);
                printf("Did set NMI\n");
                break;
            case 'I':
            case 'i':
                printf("Setting IRQ\n");
                executor_set_irq(*the_executor);
                printf("Did set IRQ\n");
                break;
            case 'q':
                is_running = false;
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
    executor_stage_callback_t   exec_callback = our_executor_stage_callback;
    isa_6502_instr_stage_t      exec_callback_event_mask = executor_stage_callback_default_stage_mask | isa_6502_instr_stage_disasm | isa_6502_instr_stage_nmi | isa_6502_instr_stage_irq;
    pthread_t                   tui_input_thread;
    
    pthread_create(&tui_input_thread, NULL, tui_input_thread_run, &the_vm);
    
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
                break;
            
            case 'r':
                printf("INFO:  hardware reset of the VM...\n");
                executor_hard_reset(the_vm);
                break;
            
            case 'b':
                printf("INFO:  software reset (from RES vector)...\n");
                executor_soft_reset(
                        the_vm,
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
                    total_cycles = executor_launch_at_address_range(
                                            the_vm,
                                            exec_callback,
                                            exec_callback_event_mask,
                                            addr_start,
                                            addr_end
                                        );
                    gettimeofday(&t1, NULL);
                    cycles_per_sec = (double)total_cycles / ((double)(t1.tv_sec - t0.tv_sec) + 1e-6 * (double)(t1.tv_usec - t0.tv_usec));
                    printf("INFO:  %.2lg cycles per second\n", cycles_per_sec);
                }
                break;
            }
            
            case 'v':
                exec_callback = our_executor_stage_callback;
                exec_callback_event_mask = executor_stage_callback_default_stage_mask | isa_6502_instr_stage_disasm | isa_6502_instr_stage_nmi | isa_6502_instr_stage_irq;
                break;
            case 'q':
                exec_callback = NULL;
                exec_callback_event_mask = 0;
                break;
            case '1':
                exec_callback = our_executor_stage_callback;
                exec_callback_event_mask = isa_6502_instr_stage_nmi | isa_6502_instr_stage_irq;
                break;
            
            case 'l': {
                char        *file_path = NULL;
                uint16_t    addr_start, addr_end;
                
                if ( parse_file_spec(optarg, &file_path, &addr_start, &addr_end) ) {
                    int         bin_fd = open(file_path, O_RDONLY);
                    
                    if ( bin_fd ) {
                        ssize_t did_read = memory_load_from_fd(the_vm->memory, addr_start, (addr_end - addr_start + 1), bin_fd);
                        
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
                        ssize_t did_write = memory_save_to_fd(the_vm->memory, addr_start, (addr_end - addr_start + 1), bin_fd);
                        
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
                            memset(
                                    &the_vm->memory->RAM.BYTES[addr_start],
                                    fill_value,
                                    addr_end - addr_start + 1
                                );
                            break;
                        }
                        case 2: {
                            uint8_t     *p = &the_vm->memory->RAM.BYTES[addr_start];
                            uint16_t    l = addr_end - addr_start + 1;
                            
                            /* Do at least one byte for our range: */
                            if ( l == 0 ) l = 1;
                            while ( l > 1 ) {
#ifdef ISA_6502_HOST_IS_LE
                                *((uint16_t*)p) = fill_value;
                                p += 2;
#else
                                *p++ = *((uint8_t*)&fill_value + 1);
                                *p++ = *((uint8_t*)&fill_value);
#endif
                                l -= 2;
                            }
                            if ( l > 0 ) {
#ifdef ISA_6502_HOST_IS_LE
                                *p++ = *((uint8_t*)&fill_value);
#else
                                *p++ = *((uint8_t*)&fill_value + 1);
#endif
                                l--;
                            }
                            break;
                        }
                    }
                }
                break;
            }
            
            case 'd': {
                uint16_t        addr_start, addr_end;
                
                if ( parse_mem_spec(optarg, &addr_start, &addr_end) ) {
                    memory_fprintf(the_vm->memory, stdout, 0, addr_start, addr_end);
                }
                break;
            }
        }
    }
    return rc;
}
