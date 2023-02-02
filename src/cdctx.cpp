void CDCtx(void), USBSEI(void), USBCLI(void);

namespace
{

enum { IDLE, HANDLER, PENDING } state;
char *pending_buffer, *tx_buffer;
int pending_length, tx_length;

void done(void) {
    switch (state) {
        case HANDLER:
            state = IDLE;
            break;
        case PENDING:
            state = HANDLER;
            CDCtx();
            tx_buffer = pending_buffer;
            tx_length = pending_length;
            break;
        default:;
    }
}

} // anonymous

void out_buffer_handler(char *buffer, int length) {
    pending_buffer = buffer; pending_length = length;
    switch (state) {
        case IDLE:
            state = HANDLER;
            CDCtx();
            if (length) {
                tx_buffer = buffer;
                tx_length = length;
            } else done();
            break;
        case HANDLER:
            state = PENDING;
            break;
        default:;
    }
}

bool get_tx_char(char &c) {
    if (!tx_length) return false;
    c = *tx_buffer++;
    if (!--tx_length) { USBCLI(); done(); USBSEI(); }
}

