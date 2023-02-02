#define USB_EP0_BUFF_SIZE 8
#define USB_EP1_BUFF_SIZE 10
#define USB_EP2_BUFF_SIZE 64

#define USB_DESCRIPTOR_DEVICE 1
#define USB_DESCRIPTOR_CONFIGURATION 2
#define USB_DESCRIPTOR_STRING 3
#define USB_DESCRIPTOR_INTERFACE 4
#define USB_DESCRIPTOR_ENDPOINT 5

#define CDC_DEVICE 2
#define COMM_INTF 2
#define DATA_INTF 0x0A
#define NO_PROTOCOL 0x00    // No class specific protocol required
#define ABSTRACT_CONTROL_MODEL 2
#define V25TER 1

#define _DEFAULT 0x80
#define _SELF 0x40

#define _BULK 2
#define _INTERRUPT 3
#define _OUT 0
#define _IN 0x80

#define DEVICE 0x0100
#define CONFIG 0x0200
#define STRING 0x0300
#define STRING1 0x0301
#define STRING2 0x0302

//https://www.qnx.com/developers/docs/6.6.0.update/index.html#com.qnx.doc.dev_pub.ref_guide/topic/usblauncher_config_usb_descriptors.html

#define CS_INTERFACE 0x24
#define DSC_FN_HEADER 0
#define DSC_FN_CALL_MGT 1
#define DSC_FN_ACM 2
#define DSC_FN_UNION 6

typedef struct {
    struct __attribute__((packed)) {
        char bFunctionLength;
        char bDescriptorType;
        char bDescriptorSubType;
        short bcdCDC;
    } USB_CDC_HEADER_FN_DSC;
    struct {
        char bFunctionLength;
        char bDescriptorType;
        char bDescriptorSubType;
        char bmCapabilities;
    } USB_CDC_ACM_FN_DSC;
    struct {
        char bFunctionLength;
        char bDescriptorType;
        char bDescriptorSubType;
        char bControlInterface;
        char bSubordinateInterface;
    } USB_CDC_UNION_FN_DSC;
//    struct {
//        char bFunctionLength;
//        char bDescriptorType;
//        char bDescriptorSubType;
//        char bmCapabilies;
//        char bDataInterface;
//    } USB_CDC_CALL_MGT_FN_DSC;
} CDC_CLASS_SPECIFIC_DESCRIPTORS;

typedef struct __attribute__((packed)) {
    char bLength;
    char bDescriptorType;
    short wTotalLength;
    char bNumInterface;
    char bConfigurationValue;
    char iConfiguration;
    unsigned char bmAttributes;
    char bMaxPower;
} CONFIGURATION_DESCRIPTOR;

typedef struct {
    char bLength;
    char bDescriptorType;
    char bInterfaceNumber;
    char bAlternateSetting;
    char bNumEndpoints;
    char bInterfaceClass;
    char bInterfaceSubClass;
    char bInterfaceProtocol;
    char iInterface;
} INTERFACE_DESCRIPTOR;

typedef struct __attribute__((packed)) {
    char bLength;
    char bDescriptorType;
    unsigned char bEndpointAddress;
    char bmAttributes;
    short wMaxPacketSize;
    unsigned char bInterval;
} ENDPOINT_DESCRIPTOR;

const struct {
    char bLength;
    char bDescriptorType;
    short bcdUSB;
    char bDeviceClass;
    char bDeviceSubClass;
    char bDeviceProtocol;
    char bMaxPacketSize0;
    short idVendor;
    short idProduct;
    short bcdDevice;
    char iManufacture;
    char iProduct;
    char iSerialNumber;
    char bNumConfigurations;
} device_descriptor = {
    0x12,                   // Size of this descriptor in bytes
    USB_DESCRIPTOR_DEVICE,  // DEVICE descriptor type
    0x0200,                 // USB Spec Release Number in BCD format
    CDC_DEVICE,             // Class Code
    0x00,                   // Subclass code
    0x00,                   // Protocol code
    USB_EP0_BUFF_SIZE,      // Max packet size for EP0, see usb_config.h
    0x04d8,                 // Vendor ID
    0x000a,                 // Product ID: CDC RS-232 Emulation Demo
    0x0100,                 // Device release number in BCD format
    0x01,                   // Manufacturer string index
    0x02,                   // Product string index
    0x00,                   // Device serial number string index
    0x01                    // Number of possible configurations
};

const struct __attribute__((packed)) {
    CONFIGURATION_DESCRIPTOR config;
    INTERFACE_DESCRIPTOR interface0;
    CDC_CLASS_SPECIFIC_DESCRIPTORS cdc_specific;
    ENDPOINT_DESCRIPTOR ep1_in;
    INTERFACE_DESCRIPTOR interface1;
    ENDPOINT_DESCRIPTOR ep2_out;
    ENDPOINT_DESCRIPTOR ep2_in;
} configuration_descriptor1 = {
    {
        9,//sizeof(USB_CFG_DSC),    // Size of this descriptor in bytes
        USB_DESCRIPTOR_CONFIGURATION,                // CONFIGURATION descriptor type
        sizeof(configuration_descriptor1),  // Total length of data for this cfg   67, 62
        2,                      // Number of interfaces in this cfg
        1,                      // Index value of this configuration
        0,                      // Configuration string index
        _DEFAULT | _SELF,               // Attributes, see usb_device.h
        50                     // Max power consumption (2X mA)
    },
    {
        9,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
        USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
        0,                      // Interface Number
        0,                      // Alternate Setting Number
        1,                      // Number of endpoints in this intf
        COMM_INTF,              // Class code
        ABSTRACT_CONTROL_MODEL, // Subclass code
        V25TER,                 // Protocol code
        0                      // Interface string index
    },
    {
        {
            5,
            CS_INTERFACE,
            DSC_FN_HEADER,
            0x0110         //0x0120 (version number)
        },
        {
            4,
            CS_INTERFACE,
            DSC_FN_ACM,
            2              //USB_CDC_ACM_SUPPORT_LINE_CODING_LINE_STATE_AND_NOTIFICATION
        },
        {
            5,
            CS_INTERFACE,
            DSC_FN_UNION,
            0,                      // CDC_COMM_INTF_ID
            1                       // CDC_DATA_INTF_ID
//        },
//        {
//            5,
//            CS_INTERFACE,
//            DSC_FN_CALL_MGT,
//            0,
//            1                       // CDC_DATA_INTF_ID
        }
    },
    {
        7, /*sizeof(USB_EP_DSC)*/
        USB_DESCRIPTOR_ENDPOINT,   //Endpoint Descriptor
        1 | _IN,                   //EndpointAddress
        _INTERRUPT,                //Attributes
        USB_EP1_BUFF_SIZE,              //size
        255                        //Interval
    },
    {
        9,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
        USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
        1,                      // Interface Number
        0,                      // Alternate Setting Number
        2,                      // Number of endpoints in this intf
        DATA_INTF,              // Class code
        0,                      // Subclass code
        NO_PROTOCOL,            // Protocol code
        0                       // Interface string index
    },
    {
        7, /*sizeof(USB_EP_DSC)*/
        USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
        2 | _OUT,                   //EndpointAddress
        _BULK,                       //Attributes
        USB_EP2_BUFF_SIZE,                //size
        0                          //Interval
    },
    {
        7, /*sizeof(USB_EP_DSC)*/
        USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
        2 | _IN,                   //EndpointAddress
        _BULK,                       //Attributes
        USB_EP2_BUFF_SIZE,                //size
        0                          //Interval
    }
};

const struct {
    char bLength;
    char bDescriptorType;
    short string[1];
} sd000 = {
    sizeof(sd000),
    USB_DESCRIPTOR_STRING,
    { 0x0409 }
};

const struct {
    char bLength;
    char bDescriptorType;
    short string[25];
} sd001 = {
    sizeof(sd001),
    USB_DESCRIPTOR_STRING,
    {
        'M','i','c','r','o','c','h','i','p',' ',
        'T','e','c','h','n','o','l','o','g','y',
        ' ','I','n','c','.'
    }
};

const struct {
    char bLength;
    char bDescriptorType;
    short string[25];
} sd002 = {
    sizeof(sd002),
    USB_DESCRIPTOR_STRING,
    {
        'C','D','C',' ','R','S','-','2','3','2',
        ' ','E','m','u','l','a','t','i','o','n',
        ' ','D','e','m','o'
    }
};

char *get_std_descriptor(short type) {
    switch (type) {
        case DEVICE: return (char*)&device_descriptor;
        case CONFIG: return (char*)&configuration_descriptor1;
        case STRING: return (char*)&sd000;
        case STRING1: return (char*)&sd001;
        case STRING2: return (char*)&sd002;
        default: return 0;
    }
}
