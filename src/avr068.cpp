#include "command.h"

#define CONFIG_PARAM_BUILD_NUMBER_LOW   0
#define CONFIG_PARAM_BUILD_NUMBER_HIGH  1
// be careful with changing HW versions. avrstudio does not like all numbers:
#define CONFIG_PARAM_HW_VER             2
// update here our own default sw version:
#define D_CONFIG_PARAM_SW_MAJOR           2
#define D_CONFIG_PARAM_SW_MINOR           0x0a
#define CONFIG_PARAM_VADJUST 25
//#define CONFIG_PARAM_OSC_PSCALE 1
// PSCALE=0 means oscillator on board is off (not needed when board used as programmer):
#define CONFIG_PARAM_OSC_PSCALE 0
#define CONFIG_PARAM_OSC_CMATCH 1

bool wait(unsigned);
bool get_tx_char(char&), rx_done(void);
bool uart_rx(char *buf, int len);
void parser_reset(void);
void led_on(void), toggle(void), led_off(void);
void debug_w(char);
void spi_set_sck_duration(int), spi_init(void);
void spi_reset_pulse(void), spi_disable(void);
char spi_send(char), spi_send(char*), spi_send(char, short, char);
unsigned char spi_get_sck_duration(void);

struct __attribute__((__packed__)) cmd_program {
    unsigned char NumBytes_MSB;
    unsigned char NumBytes_LSB;
    char mode;
    unsigned char delay;
    char load_write;
    char write_page;
    char read;
    char poll1;
    char poll2;
    char data[];
};

namespace
{

const unsigned char para_table[] = {
    PARAM_BUILD_NUMBER_LOW, CONFIG_PARAM_BUILD_NUMBER_LOW,
    PARAM_BUILD_NUMBER_HIGH, CONFIG_PARAM_BUILD_NUMBER_HIGH,
    PARAM_HW_VER, CONFIG_PARAM_HW_VER,
    PARAM_SW_MAJOR, D_CONFIG_PARAM_SW_MAJOR,
    PARAM_SW_MINOR, D_CONFIG_PARAM_SW_MINOR,
    PARAM_VTARGET, 33,
    PARAM_VADJUST, CONFIG_PARAM_VADJUST,
    PARAM_OSC_PSCALE, CONFIG_PARAM_OSC_PSCALE,
    PARAM_OSC_CMATCH, CONFIG_PARAM_OSC_CMATCH,
    PARAM_TOPCARD_DETECT, 0xFF,
    PARAM_DATA, 0,
    PARAM_RESET_POLARITY, 1
};

struct __attribute__((__packed__)) {
    char start;
    char sn;
    unsigned char sizeM;
    unsigned char sizeL;
    char token;
    char message[283];
} packet;
#define RETURN_STATUS packet.message[1]

int cs, command;
char *ptr;
char param_controller_init; // dummy
bool larger_than_64k, addressing_is_word;
unsigned short address, extended;

void parser(char c) {
    command = 0;
    cs ^= *ptr = c;
    if (packet.start != MESSAGE_START) return parser_reset();
    if (packet.token != TOKEN) return parser_reset();
    if (ptr++ == &packet.message[(packet.sizeM << 8) + packet.sizeL])
        if (!cs) command = packet.message[0];
}

void set_extended_address(int e) {
    if (larger_than_64k) spi_send(0x4d, extended = e, 0);
}

char addressing(char cmd, int odd, char c) {
    if ((larger_than_64k) && (extended & 0x8000))
        spi_send(0x4d, extended = ~extended, 0);
    if (addressing_is_word) cmd |= (odd << 3);
    return spi_send(cmd, address, c);
}

void address_next(int odd) {
    if (!addressing_is_word || odd) address++;
    if (!address) {
        if (!(extended & 0x8000)) extended = ~extended;
        extended--;
    }
}

char polling(int index, char mode, cmd_program *command) {
    int timeout = 10000;
    if (!addressing_is_word) wait(1);
    if (mode & 8) {     // RDY/BSY polling
        while (--timeout && wait(0));
            if (!(spi_send(0xf0, 0, 0) & 1)) return STATUS_CMD_OK;
    } else {
        if ((command->data[index] == command->poll1) || (mode & 2))
            wait(command->delay);
        else {
            while (--timeout && wait(0));
                if (addressing(command->read, index & 1, 0) != command->poll1) return STATUS_CMD_OK;
        }
    }
    return timeout ? STATUS_CMD_OK : STATUS_CMD_TOUT;
}

} // anonymous

void parser_reset(void) {
    cs = 0;
    packet.token = TOKEN;
    ptr = &packet.start;
}


void stk500(void) {
    char c;
    if (!get_tx_char(c)) return;
    parser(c);
    int size = 2;
//    if (command) debug_w(command);
    addressing_is_word = true;  // 16 bit is default
    switch (command) {
        case 0: return;
        case CMD_SIGN_ON: toggle();
        {
            struct __attribute__((__packed__)) _answer {
                unsigned char length;
                int signature[2];
            } *answer = (_answer*)&packet.message[2];
            answer->length = 8;
            answer->signature[0] = 0x354B5453;   // STK5
            answer->signature[1] = 0x325F3030;   // 00_2
        }
            size = 11;
            RETURN_STATUS = STATUS_CMD_OK;
            break;
        case CMD_SET_PARAMETER:
        {
            struct __attribute__((__packed__)) _command {
                unsigned char parameter;
                unsigned char value;
            } *command = (_command*)&packet.message[1];
            if (command->parameter == PARAM_SCK_DURATION)
                spi_set_sck_duration(command->value);
            else if (command->parameter == PARAM_CONTROLLER_INIT)
                    param_controller_init = command->value;
        }
            RETURN_STATUS = STATUS_CMD_OK;
            break;
        case CMD_GET_PARAMETER:
            size = 3;   // assume STATUS_CMD_OK
        {
            struct __attribute__((__packed__)) _command {
                unsigned char parameter;
                unsigned char value;
            } *command = (_command*)&packet.message[1];
            int i;
            for (i = 0; i < sizeof(para_table); i += 2)
                if (command->parameter == para_table[i]) break;
            if (i < sizeof(para_table))
                command->value = para_table[++i];
            else {
                if (command->parameter == PARAM_SCK_DURATION)
                    command->value = spi_get_sck_duration();
                else if (command->parameter == PARAM_CONTROLLER_INIT)
                    command->value = param_controller_init;
                else size = 2;
            }
        }
            RETURN_STATUS = size == 3 ? STATUS_CMD_OK : STATUS_CMD_UNKNOWN;
            break;
        case CMD_LOAD_ADDRESS:
            larger_than_64k = packet.message[1] & 0x80 ? true : false;
            address = (packet.message[3] << 8) | (packet.message[4] & 255);
            extended = ~packet.message[2];  // pending
            RETURN_STATUS = STATUS_CMD_OK;
            break;
        case CMD_FIRMWARE_UPGRADE:
            RETURN_STATUS = STATUS_CMD_FAILED;
            break;
        case CMD_ENTER_PROGMODE_ISP:
            spi_init();
        {
            struct __attribute__((__packed__)) _command {
                unsigned char timeout;
                unsigned char stabDelay;
                unsigned char cmdexeDelay;
                unsigned char synchLoops;
                unsigned char byteDelay;
                unsigned char pollValue;
                unsigned char pollIndex;
                char cmd[4];
            } *command = (_command*)&packet.message[1];
            wait(command->stabDelay);
            if (command->synchLoops > 48) command->synchLoops = 48;
            if (!command->byteDelay) command->byteDelay = 1;
            char r3, r4, r = command->pollValue;
            for (int i = 0; i < command->synchLoops; i++) {
                led_on();
                wait(command->cmdexeDelay);
                spi_send(command->cmd[0]);
                wait(command->byteDelay);
                spi_send(command->cmd[1]);
                wait(command->byteDelay);
                led_off();
                r3 = spi_send(command->cmd[2]);
                wait(command->byteDelay);
                r4 = spi_send(command->cmd[3]);
                wait(command->byteDelay);
                if (command->pollIndex == 3) r = r3;
                if (command->pollIndex == 4) r = r4;
                if (r == command->pollValue) {
                    led_on();
                    break;
                }
                spi_reset_pulse();
                wait(20);
            }
            RETURN_STATUS = r == command->pollValue ? STATUS_CMD_OK : STATUS_CMD_FAILED;
        }
            break;
        case CMD_LEAVE_PROGMODE_ISP:
            spi_disable();
            led_off();
            RETURN_STATUS = STATUS_CMD_OK;
            break;
        case CMD_CHIP_ERASE_ISP:
        {
            struct __attribute__((__packed__)) _command {
                unsigned char eraseDelay;
                unsigned char pollMethod;
                char cmd[4];
            } *command = (_command*)&packet.message[1];
            spi_send(command->cmd);
            if (command->pollMethod) {
                for (int i = 0; i < 10000; i++)
                    if (!(spi_send(0xf0, 0, 0) & 1) && wait(0)) break;
            } else wait(command->eraseDelay);
        }
            RETURN_STATUS = STATUS_CMD_OK;
            break;
        case CMD_PROGRAM_EEPROM_ISP:
            addressing_is_word = false;
        case CMD_PROGRAM_FLASH_ISP:
        {
            cmd_program *command = (cmd_program*)&packet.message[1];
            if (command->delay < 4) command->delay = 4;
            if (command->delay > 32) command->delay = 32;
            unsigned short saddress = address;
            size = (command->NumBytes_MSB << 8) | command->NumBytes_LSB;
            if (size > 280) {
                size = 2;
                RETURN_STATUS = STATUS_CMD_FAILED;
                break;
            }
            if (command->mode & 1) {    // page mode
                for (int i = 0; i < size; i++) {
                    addressing(command->load_write, i & 1, command->data[i]);
                    address_next(i & 1);
                    RETURN_STATUS = STATUS_CMD_OK;
                }
                if (command->mode & 0x80) { // write page
                    extended = 0;   // suppress update extended address
                    spi_send(command->write_page, saddress, 0);
                    RETURN_STATUS = polling(0, command->mode >> 3, command);
                }
            } else {                    // word mode
                for (int i = 0; i < size; i++) {
                    addressing(command->load_write, i & 1, command->data[i]);
                    if (!addressing_is_word) wait(2);   // EEPROM
                    if ((RETURN_STATUS = polling(i, command->mode >> 3, command)) == STATUS_CMD_TOUT) break;
                    address_next(i & 1);
                }
            }
        }
            size = 2;
            break;
        case CMD_READ_EEPROM_ISP:
            addressing_is_word = false;
        case CMD_READ_FLASH_ISP:
        {
            struct __attribute__((__packed__)) _command {
                unsigned char NumBytes_MSB;
                unsigned char NumBytes_LSB;
                char cmd;
            } *command = (_command*)&packet.message[1];
            size = (command->NumBytes_MSB << 8) | command->NumBytes_LSB;
            if (size > 280) size = 280;
            char cmd = command->cmd;
            for (int i = 0; i < size; i++) {
                packet.message[i + 2] = addressing(cmd, i & 1, 0);
                address_next(i & 1);
            }
        }
            RETURN_STATUS = packet.message[size + 2] = STATUS_CMD_OK;
            size += 3;
            break;
        case CMD_PROGRAM_LOCK_ISP:
        case CMD_PROGRAM_FUSE_ISP:
            spi_send((char*)&packet.message[1]);
            size = 3;
            RETURN_STATUS = packet.message[2] = STATUS_CMD_OK;
            break;
        case CMD_READ_OSCCAL_ISP:
        case CMD_READ_SIGNATURE_ISP:
        case CMD_READ_LOCK_ISP:
        case CMD_READ_FUSE_ISP:
        {
            struct __attribute__((__packed__)) _command {
                unsigned char pollIndex;
                char cmd[4];
            } *command = (_command*)&packet.message[1];
            char r[5];
            r[1] = spi_send(command->cmd[0]); wait(5);
            r[2] = spi_send(command->cmd[1]); wait(5);
            r[3] = spi_send(command->cmd[2]); wait(5);
            r[4] = spi_send(command->cmd[3]);
            packet.message[2] = r[command->pollIndex];
        }
            size = 4;
            RETURN_STATUS = packet.message[3] = STATUS_CMD_OK;
            break;
        case CMD_SPI_MULTI:
        {
            struct __attribute__((__packed__)) _command {
                unsigned char NumTx, NumRx, RxStartAddr;
                char TxData[];
            } *command = (_command*)&packet.message[1];
            int NumRx = command->NumRx;
            int RxStartAddr = command->RxStartAddr;
            char *data = (char*)&packet.message[2];
            int tx_index = 0;
            int rx_index = 0;
            while (command->NumTx || NumRx) {
                data[rx_index] = spi_send(command->NumTx ? command->TxData[tx_index] : 0);
                if ((tx_index++ >= RxStartAddr) && NumRx) { rx_index++; NumRx--; }
                if (command->NumTx) command->NumTx--;
            }
            size = rx_index + 3;
            RETURN_STATUS = data[rx_index] = STATUS_CMD_OK;
        }
            break;
        default: RETURN_STATUS = STATUS_CMD_UNKNOWN;
    }
    cs = command = 0;
    ptr = &packet.start;
    packet.sizeL = size & 255;
    packet.sizeM = (size >> 8) & 255;
    size += 5;
    for (int i = 0; i < size; i++) cs ^= ptr[i];
    ptr[size++] = cs;
    ptr[size++] = '\n';     // flush tty buffer
    uart_rx(ptr, size);
    while (!rx_done());     // block tx from using packet
    parser_reset();
}

/*
stty -F /dev/ttyACM0 115200 cs8 -cstopb -parenb
cat abc.dat > /dev/ttyACM0
od -x < /dev/ttyACM0
*/