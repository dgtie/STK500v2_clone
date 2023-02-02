void init(void), toggle(void), USBDeviceInit(void), button(void);
bool wait(unsigned);
bool uart_rx(char *buf, int len);
void stk500(void), parser_reset(void);

int main(void) {
    init();
    USBDeviceInit();
    parser_reset();
    while (wait(0)) stk500();
}

void poll(unsigned t) {
    if (t & 63) return;
    button();
}

//avrdude -P /dev/ttyACM0 -c stk500v2 -p m324p
//avrdude -P /dev/ttyACM0 -c stk500v2 -p m328p -U flash:r:flash.bin:r