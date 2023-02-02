/* 
 * File:   usb.h
 * Author: sylam
 *
 * Created on August 24, 2022, 10:12 PM
 */

#ifndef USB_H
#define	USB_H

#define PID_SETUP 13                // PID type
#define PID_IN 9
#define PID_OUT 1

#define TYPE_STANDARD 0              // type
#define TYPE_CLASS 1
#define TYPE_VENDOR 2

#define CLEAR_FEATURE 1         // std Request
#define GET_CONFIGURATION 8
#define GET_DESCRIPTOR 6
#define GET_INTERFACE 10
#define GET_STATUS 0
#define SET_ADDRESS 5
#define SET_CONFIGURATION 9
#define SET_DESCRIPTOR 7
#define SET_FEATURE 3
#define SET_INTERFACE 11
#define SYNCH_FRAME 12

/* CDC Class-Specific Requests */
#define SEND_ENCAPSULATED_COMMAND   0x00
#define GET_ENCAPSULATED_RESPONSE   0x01
#define SET_COMM_FEATURE            0x02
#define GET_COMM_FEATURE            0x03
#define CLEAR_COMM_FEATURE          0x04
#define SET_LINE_CODING             0x20
#define GET_LINE_CODING             0x21
#define SET_CONTROL_LINE_STATE      0x22
#define SEND_BREAK                  0x23

typedef struct {
        unsigned int recipient:5;
        unsigned int type:2;
        unsigned int direction:1;
        char request;
        unsigned short value;
        unsigned short index;
        unsigned short length;
} setup_packet;


#endif	/* USB_H */

