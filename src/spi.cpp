#include <xc.h>

bool wait(unsigned);
void wait_M25(void);

namespace
{
    
int sck_dur;

} // anonymous

void spi_reset_pulse(void) {
    wait_M25();
    LATBSET = _LATB_LATB3_MASK;
    wait_M25();
    LATBCLR = _LATB_LATB3_MASK;
//    wait(20);
}

void spi_set_sck_duration(int i) {
    const int table[7] = { 21, 86, 349, 714, 740, 768, 8191 };
    int m = SPI2CON & SPICON_ON;
    SPI2CONCLR = SPICON_ON;
    if (i > 6) i = 6;
    SPI2BRG = table[sck_dur = i];
    SPI2CONSET = m;
}

void spi_init(void) {
    TRISBCLR = LATBCLR = _LATB_LATB3_MASK;
    SPI2CON = 0;
    int c = SPI2BUF;    // clears the receive buffer
//    TRISBCLR = LATBCLR = _LATB_LATB15_MASK | _LATB_LATB1_MASK;
    spi_set_sck_duration(sck_dur = 0);
//    SPI2STATCLR = SPISTAT_SPIROV;
    SPI2CON = SPICON_MSTEN | SPICON_CKE;
    SPI2CONSET = SPICON_ON;
    spi_reset_pulse();
    wait(20);
}

char spi_send(char c) {
    IFS1CLR = _IFS1_SPI2RXIF_MASK;
    SPI2BUF = c;
    while (!IFS1bits.SPI2RXIF);
    c = SPI2BUF;
    IFS1CLR = _IFS1_SPI2RXIF_MASK;
    return c;
}

char spi_send(char *p) {
    spi_send(p[0]);
    spi_send(p[1]);
    spi_send(p[2]);
    return spi_send(p[3]);
}

char spi_send(char c, short a, char d) {
    spi_send(c);
    spi_send(a >> 8);
    spi_send(a & 255);
    return spi_send(d);
}

void spi_disable(void) {
//    TRISBSET = _TRISB_TRISB15_MASK | _TRISB_TRISB1_MASK;
    SPI2CON = 0;                                // SPI off
    TRISBSET = LATBSET = _LATB_LATB3_MASK;;     // TRG-RST
}

unsigned char spi_get_sck_duration(void) { return sck_dur & 255; }
