#include <stdint.h>

typedef struct {
    volatile uint32_t URXD;
    volatile uint32_t reserved1_[15];
    volatile uint32_t UTXD;
    volatile uint32_t reserved2_[15];
    volatile uint32_t UCR1;
    volatile uint32_t UCR2;
    volatile uint32_t UCR3;
    volatile uint32_t UCR4;
    volatile uint32_t UFCR;
    volatile uint32_t USR1;
    volatile uint32_t USR2;
    volatile uint32_t UESC;
    volatile uint32_t UTIM;
    volatile uint32_t UBIR;
    volatile uint32_t UBMR;
    volatile uint32_t UBRC;
    volatile uint32_t ONEMS;
    volatile uint32_t UTS;
} uart_regs_t;

#define UART_UCR1_TRDYEN_MASK   (1 << 13)
#define UART_UCR1_UARTEN_MASK   (1 << 0)

#define UART_UCR2_TXEN_MASK     (1 << 2)

#define UART_USR1_TRDY_MASK     (1 << 13)

#define UART_1  ((uart_regs_t *) 0x53FBC000)
#define UART_2  ((uart_regs_t *) 0x53FC0000)
#define UART_3  ((uart_regs_t *) 0x5000C000)
#define UART_4  ((uart_regs_t *) 0x53FF0000)
#define UART_5  ((uart_regs_t *) 0x63F90000)

#define UART    UART_1

static void printc(char c)
{
    UART->UCR1 |= UART_UCR1_UARTEN_MASK | UART_UCR1_TRDYEN_MASK;
    UART->UCR2 |= UART_UCR2_TXEN_MASK;

    while ((UART->USR1 & UART_USR1_TRDY_MASK) == 0);

    UART->UTXD = c;
}

static void printraw(const char *msg)
{
    while (*msg != '\0') {
        printc(*msg);
        msg += 1;
    }
}

extern void print(const char *msg)
{
    printraw("bootrom: ");
    printraw(msg);
    printraw("\r\n");
}

extern void printhex(const char *msg, uint32_t value)
{
    int n;

    printraw("bootrom: ");
    printraw(msg);
    printraw(": 0x");

    for (n = 0; n < 8; n++) {
        uint8_t v = (value >> ((7 - n) * 4)) & 0xF;

        if (v < 10) {
            printc('0' + v);
        } else {
            printc('A' + v - 10);
        }
    }

    printraw("\r\n");
}

