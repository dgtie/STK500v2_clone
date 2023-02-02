void CDCrx(char *buffer, int length);

namespace
{
    char *buffer;
    int length;
}

void CDCrx_reset(void) { length = 0; }

void in_buffer_handler(int len) {
    buffer = &buffer[len];
    if ((length -= len)) CDCrx(buffer, length);
}

bool uart_rx(char *buf, int len) {
    if (length) return false;
    CDCrx(buffer = buf, length = len);
    return true;
}

bool rx_done(void) { return !length; }