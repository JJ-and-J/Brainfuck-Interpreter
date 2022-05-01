#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MEMSIZE 65536

typedef struct {
    uint8_t *mem;
    uint8_t *prg;

    size_t ms, ps;
    size_t mc, pc;
} brainfuck_t;

int bf_reset(brainfuck_t *bf, size_t memsz);
int bf_load(brainfuck_t *bf, FILE *f);

int bf_run(brainfuck_t *bf);

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "usage: %s [progname.b]\n", argv[0]);
        return 1;
    }

    brainfuck_t bf;
    if(bf_reset(&bf, MEMSIZE)) {
        fprintf(stderr, "failed to allocate 0x%x bytes memory\n", MEMSIZE);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if(bf_load(&bf, f)) {
        fprintf(stderr, "failed to open file %s for reading\n", argv[1]);
        fclose(f);

        return 1;
    }

    fclose(f);

    while(!bf_run(&bf));

    return 0;
}

int bf_reset(brainfuck_t *bf, size_t memsz) {
    if(!bf) return 1;

    if(bf->prg) free(bf->prg);
    if(bf->mem) free(bf->mem);

    bf->pc = 0;
    bf->mc = 0;

    bf->mem = malloc(memsz);
    if(!bf->mem) return 1;

    bf->ms = memsz;

    for(int ix = 0; ix < memsz; ++ ix) {
        bf->mem[ix] = 0;
    }

    return 0;
}

int bf_load(brainfuck_t *bf, FILE *f) {
    if(!bf || !f) return 1;

    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    bf->prg = malloc(fsize);
    if(!bf->prg) return 1;

    bf->ps = fsize;

    fread(bf->prg, fsize, 1, f);

    return 0;
}

int bf_run(brainfuck_t *bf) {
    if(bf->pc >= bf->ps) return 1;

    switch(bf->prg[bf->pc]) {
        case '<': {
            -- bf->mc;

            if(bf->mc < 0) {
                bf->mc = bf->ms - 1;
            }

            break;
        }

        case '>': {
            ++ bf->mc;

            if(bf->mc >= bf->ms) {
                bf->mc = 0;
            }

            break;
        }

        case '+': {
            ++ bf->mem[bf->mc];
            break;
        }

        case '-': {
            -- bf->mem[bf->mc];
            break;
        }

        case '.': {
            fprintf(stdout, "%c", bf->mem[bf->mc]);
            fflush(stdout);
            break;
        }

        case ',': {
            bf->mem[bf->mc] = fgetc(stdin);
            break;
        }

        case '[': {
            if(bf->mem[bf->mc]) break;

            size_t brackets = 1;
            for(size_t of = 1; of + bf->pc < bf->ps; ++ of) {
                switch(bf->prg[bf->pc + of]) {
                    case '[': {
                        ++ brackets;
                        break;
                    }

                    case ']': {
                        -- brackets;
                        break;
                    }
                }

                if(brackets == 0) {
                    bf->pc += of;
                    break;
                }
            }

            if(brackets != 0) return 1;

            break;
        }

        case ']': {
            if(!bf->mem[bf->mc]) break;

            size_t brackets = 1;
            for(size_t of = -1; bf->pc - of >= 0; -- of) {
                switch(bf->prg[bf->pc + of]) {
                    case ']': {
                        ++ brackets;
                        break;
                    }

                    case '[': {
                        -- brackets;
                        break;
                    }
                }

                if(brackets == 0) {
                    bf->pc += of;
                    break;
                }
            }

            if(brackets != 0) return 1;

            break;
        }
    }

    ++ bf->pc;

    return 0;
}
