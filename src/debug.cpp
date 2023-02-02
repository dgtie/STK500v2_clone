void toggle(void);
bool uart_rx(char *buf, int len);

#define SIZE 2048

char history[SIZE];
int w_index = 0;
int r_index = 0;

void debug_w(char c) { if (w_index < SIZE) history[w_index++] = c; }

char number[] = "00 00 00 00\r\n";

void print(char c, char *p) {
    const char n[] = "0123456789abcdef";
    int i = (c >> 4) & 15;
    p[0] = n[i];
    p[1] = n[c & 15];
}

void button_changed(int b) {
    if (!b) toggle();
    else {
        if (w_index == r_index) uart_rx("none\r\n", 6);
        else {
            print(history[r_index++], number);
            print(history[r_index++], &number[3]);
            print(history[r_index++], &number[6]);
            print(history[r_index++], &number[9]);
            uart_rx(number, 13);
        }
    }
}

