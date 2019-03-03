#ifndef PHOS_CPU_H
#define PHOS_CPU_H

struct registers {
    union {
        struct {
            unsigned char f;
            unsigned char a;
        };
        unsigned short af;
    };

    union {
        struct {
            unsigned char c;
            unsigned char b;
        };
        unsigned short bc;
    };

    union {
        struct {
            unsigned char e;
            unsigned char d;
        };
        unsigned short de;
    };

    union {
        struct {
            unsigned char l;
            unsigned char h;
        };
        unsigned short hl;
    };

    unsigned short sp;
    unsigned short pc;
};

class CPU {
public:
    struct registers regs;
public:
    CPU();

};

#endif //PHOS_CPU_H
