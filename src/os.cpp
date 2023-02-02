#include <xc.h>

void poll(unsigned), button_changed(int);

/*** DEVCFG0 ***/
//#pragma config DEBUG =      OFF
//chipkit compiler pic32-g++ has a bug here: DEBUG=OFF is not OFF
//Anyway, default is DEBUG OFF
#pragma config JTAGEN =     OFF
#pragma config ICESEL = ICS_PGx1   // ICE/ICD Comm Channel Select
#pragma config PWP =        OFF
#pragma config BWP =        OFF
#pragma config CP =         OFF

/*** DEVCFG1 ***/
#pragma config FNOSC =      PRIPLL
#pragma config FPBDIV =     DIV_1
#pragma config FSOSCEN =    OFF
#pragma config IESO =       ON
#pragma config POSCMOD =    XT
#pragma config OSCIOFNC =   OFF
#pragma config FCKSM =      CSDCMD
#pragma config WDTPS =      PS1048576
#pragma config FWDTEN =     OFF
#pragma config WINDIS =     OFF
#pragma config FWDTWINSZ =  WINSZ_25

/*** DEVCFG2 ***/
#pragma config FPLLIDIV =   DIV_2    // 8 MHz -> 4 MHz
#pragma config FPLLMUL =    MUL_20   // 80 MHz
#pragma config FPLLODIV =   DIV_2    // 40MHz System Clock
#pragma config UPLLEN =     ON
#pragma config UPLLIDIV =   DIV_2

/*** DEVCFG3 ***/
#pragma config FVBUSONIO =  ON
#pragma config USERID =     0xFFFF
// another chipkit compiler bug here in USERID
#pragma config PMDL1WAY =   OFF
#pragma config IOL1WAY =    OFF
#pragma config FUSBIDIO =   OFF

#define M25 5000  // 0.25ms

void init(void) {
  __builtin_disable_interrupts();
  _CP0_SET_COMPARE(M25);
  SYSKEY = 0;                       // ensure OSCCON is locked
  SYSKEY = 0xAA996655;              // unlock sequence
  SYSKEY = 0x556699AA;
  CFGCONbits.IOLOCK = 0;            // allow write
  SDI2R = 0b0100;                   // PB2
  RPB1R = 0b0100;                   // SDO2
  CFGCONbits.IOLOCK = 1;            // forbidden write
  SYSKEY = 0;                       // relock
  ANSELBCLR = 0xf;
  TRISBCLR = 0x200;                 // RB9 (LED)
  CNPUC = 0x200;                    // RC9
  IPC0bits.CTIP = 1;
  IEC0bits.CTIE = 1;
  INTCONSET = _INTCON_MVEC_MASK;
  __builtin_enable_interrupts();
}

namespace {
    
volatile unsigned tick;

void loop(unsigned t) {
  static unsigned tick;
  if (tick == t) return;
  poll(tick = t);
}
    
}//anonymous

bool wait(unsigned i) {
    unsigned u = tick;
    i <<= 2;
    for (loop(u); i--; u = tick) while (u == tick) loop(u);
    return true;
}

void wait_M25(void) {
  unsigned u = tick;
  while (u == tick);
}

extern "C"
void __attribute__((interrupt(ipl1soft), vector(_CORE_TIMER_VECTOR), nomips16)) rtcISR(void) {
  _CP0_SET_COMPARE(_CP0_GET_COMPARE() + M25);
  tick++;
  IFS0bits.CTIF = 0;
}

void toggle(void) { LATBINV = 0x200; }
void led_on(void) { LATBSET = 0x200; }
void led_off(void) { LATBCLR = 0x200; }

void button(void) {
  static int b = 0x200;
  if (b == (PORTC & 0x200)) return;
  button_changed(b ^= 0x200);
}

