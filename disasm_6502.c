
#include "libmembus/membus.h"
#include "libmembus/membus_module_std64k.h"
#include "isa_6502.h"

#include <getopt.h>
#include <ctype.h>
#include <sys/time.h>

//

const struct option cli_options[] = {
        { "help",       no_argument,            NULL,   'h' },
        { "version",    no_argument,            NULL,   'V' },
        { "load",       required_argument,      NULL,   'l' },
        { "poke",       required_argument,      NULL,   'p' },
        { "dump",       required_argument,      NULL,   'd' },
        { "isa",        required_argument,      NULL,   'i' },
        { "disasm",     required_argument,      NULL,   'D' },
        { "origin",     required_argument,      NULL,   'O' },
        { NULL,         no_argument,            NULL,    0  }
    };
const char *cli_options_str = "hVl:p:d:i:D:O:";

//

void
usage(
    const char  *exe
)
{
    printf(
        "usage:\n"
        "\n"
        "    %s {options} {<file-path> {<file-path> ..}}\n"
        "\n"
        "  basic options:\n"
        "\n"
        "    --help/-h                      show this help\n"
        "\n"
        "  data manipulation options:\n"
        "\n"
        "    --load/-l <file-spec>          fill memory with bytes from a file\n"
        "    --poke/-p <fill-spec>          write a byte or word to a memory region\n"
        "    --dump/-d <mem-spec>           do a hexdump of a memory region\n"
        "    --disasm/-D <mem-spec>         do a static disassembly of a memory region\n"
        "\n"
        "  execution options:\n"
        "\n"
        "    --isa/-i <isa-dialect>         choose the 6502 dialect; default is 6502\n"
        "    --origin/-O <addr>             load location for bare <file-path> programs\n"
        "                                   (default: 0x2000)\n"
        "\n"
        "    <addr> = $X{X..} | 0xX{X..} | 0#{#..} | #{#..} | IRQ{±#} | NMI{±#} | RES{±#}\n"
        "    <len> = $X{X..} | 0xX{X..} | 0#{#..} | #{#..} | *\n"
        "    <mem-spec> = <addr>{:<len>} | <addr>{-<addr>}\n"
        "    <file-spec> = <file-path>@<mem-spec>\n"
        "    <byte-word> = $XX | $XXXX | 0xXX | 0xXXXX\n"
        "    <fill-spec> = <byte-word>@<mem-spec>\n"
        "    <isa-dialect> = 6502 | 65C02\n"
        "\n"
        "    File paths as bare arguments outside of the options will be loaded at the\n"
        "    address provided by --origin/-O and have their full length disassembled.\n"
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

int
main(
    int                         argc,
    char* const                 argv[]
)
{
    int                         rc = 0, optch;
    isa_6502_dialect_t          the_dialect = isa_6502_dialect_base;
    membus_t                    *ram = membus_alloc();
    membus_module_ref           base64k = membus_module_std64k_alloc();
    isa_6502_table_t            *isa = isa_6502_table_alloc();
    uint16_t                    origin = 0x2000;
    
    membus_register_module(ram, 0, base64k);
    isa_6502_table_init(isa, the_dialect);
    
    srandom(time(NULL));
    
    while ( (optch = getopt_long(argc, argv, cli_options_str, cli_options, NULL)) != -1 ) {
        switch ( optch ) {
            case 'h':
                usage(argv[0]);
                exit(0);
            
            case 'V': 
                printf("%s\n", SYSTEM_6502_NAME " version " SYSTEM_6502_VERSION);
                exit(0);
            
            case 'l': {
                char        *file_path = NULL;
                uint16_t    addr_start, addr_end;
                
                if ( parse_file_spec(optarg, &file_path, &addr_start, &addr_end) ) {
                    int         bin_fd = open(file_path, O_RDONLY);
                    
                    if ( bin_fd ) {
                        ssize_t did_read = membus_load_from_fd(
                                                ram,
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
            
            case 'p': {
                uint16_t        addr_start, addr_end;
                uint16_t        fill_value;
                size_t          fill_size;
                
                if ( parse_fill_spec(optarg, &fill_value, &fill_size, &addr_start, &addr_end) ) {
                    switch ( fill_size ) {
                        case 1: {
                            membus_write_byte_to_range(ram, memory_addr_range_with_lo_and_hi(addr_start, addr_end), fill_value);
                            break;
                        }
                        case 2: {
                            membus_write_word_to_range(ram, memory_addr_range_with_lo_and_hi(addr_start, addr_end), fill_value);
                            break;
                        }
                    }
                }
                break;
            }
            
            case 'd': {
                uint16_t        addr_start, addr_end;
                
                if ( parse_mem_spec(optarg, &addr_start, &addr_end) ) {
                    membus_fprintf(ram, stdout, 0, memory_addr_range_with_lo_and_hi(addr_start, addr_end));
                }
                break;
            }
            
            case 'D': {
                uint16_t        addr_start, addr_end;
                
                if ( parse_mem_spec(optarg, &addr_start, &addr_end) ) {
                    isa_6502_static_disassembly(isa, ram, memory_addr_range_with_lo_and_hi(addr_start, addr_end), stdout);
                }
                break;
            }
            
            case 'i':
                if ( parse_isa_dialect(optarg, &the_dialect) ) {
                    isa_6502_table_init(isa, the_dialect);
                }
                break;
            
            case 'O':
                parse_addr(optarg, &origin);
                break;
        }
    }
    
    // Any bare file paths left?
    argc -= optind;
    argv += optind;
    
    printf("%d\n", argc);
    optch = 0;
    while ( optch < argc ) {
        int     bin_fd = open(argv[optch], O_RDONLY);
                    
        if ( bin_fd ) {
            ssize_t did_read = membus_load_from_fd(
                                    ram,
                                    memory_addr_range_with_lo_and_hi(origin, 0xFFFF),
                                    bin_fd);
            
            printf("INFO:  read %ld ($%04hX) bytes into memory range $%04hX-$%04hX\n",
                    did_read,
                    (uint16_t)did_read,
                    origin,
                    origin + (uint16_t)did_read - 1
                );
            close(bin_fd);
            isa_6502_static_disassembly(isa, ram, memory_addr_range_with_lo_and_len(origin, did_read), stdout);
        }
        optch++;
    }
    
    return rc;
}
