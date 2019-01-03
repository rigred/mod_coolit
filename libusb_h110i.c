#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <libusb.h>
//and compile with:
//gcc -Wall -o libusb_h110i libusb_h110i.c `pkg-config --cflags --libs libusb-1.0`


#define USB_VENDOR_ID	    0x1b1c      /* USB vendor ID used by the device
                                        * 0x1b1 is Corsair / CoolIT
                                        */
#define USB_PRODUCT_ID	    0x0c04     /* USB product ID used by the device 
                                        * 0x0c04 is H110i
                                        */ 

#define HID_SET_REPORT      0x09
#define HID_GET_REPORT      0x01
#define HID_REPORT_TYPE_INPUT 0x01
#define HID_REPORT_TYPE_OUTPUT 0x02
#define HID_REPORT_TYPE_FEATURE 0x03
#define INTERFACE_NUMBER 0
#define INTERRUPT_IN_ENDPOINT 0x81

#define USB_ENDPOINT_IN	    (LIBUSB_ENDPOINT_IN  | 1)   /* endpoint address */
#define USB_ENDPOINT_OUT	(LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT | LIBUSB_RECIPIENT_INTERFACE )   /* endpoint address */
#define INTERFACE_NUMBER    0
#define USB_TIMEOUT	        1000        /* Connection timeout (in ms) */

static libusb_context *ctx = NULL;
static libusb_device_handle *handle;
struct libusb_device_descriptor descriptor;
struct libusb_config_descriptor *conf;

static uint8_t receiveBuf[64];
uint8_t transferBuf[64];

uint16_t counter=0;


void print_hex(unsigned char *buffer, size_t len) {
    printf("Size of buffer: %ld\nData: ", len);
    for (int i =0; i < len; i++)
    {
        printf("%02x", buffer[i] & 0xff);
    }
    printf("\n");
}


/*
 * Read a packet
 */
static int usb_read(void)
{
	int nread, ret;
    ret = libusb_interrupt_transfer(handle, USB_ENDPOINT_IN, receiveBuf, sizeof(receiveBuf), &nread, USB_TIMEOUT );
	//ret = libusb_bulk_transfer(handle, USB_ENDPOINT_IN, receiveBuf, sizeof(receiveBuf), &nread, USB_TIMEOUT);
	if (ret){
		printf("ERROR in bulk read: %d\n", ret);
		return -1;
    } else {
        print_hex(receiveBuf, sizeof(receiveBuf));
		//printf("%s", receiveBuf);  //Use this for benchmarking purposes
		return 0;
    }
}


/*
 * write a few bytes to the device
 *
 */
static int usb_write(void)
{
	int ret;
    //write transfer
    ret = libusb_control_transfer( handle, USB_ENDPOINT_OUT, HID_SET_REPORT, ( HID_REPORT_TYPE_OUTPUT << 8 ) | 0x00, INTERFACE_NUMBER, transferBuf, sizeof(transferBuf), USB_TIMEOUT );
    //Error handling
    switch(ret){
        case LIBUSB_ERROR_TIMEOUT:
            printf("ERROR in bulk write: %d Timeout\n", ret);
            break;
        case LIBUSB_ERROR_PIPE:
            printf("ERROR in bulk write: %d Pipe\n", ret);
            break;
        case LIBUSB_ERROR_NO_DEVICE:
            printf("ERROR in bulk write: %d No Device\n", ret);
            break;
        default:
            printf("send %ld bytes to device\n", sizeof(transferBuf));
            return 0;
    }
    return -1;

}

/*
 * on SIGINT: close USB interface
 * This still leads to a segfault on my system...
 */
static void sighandler(int signum)
{
    printf( "\nInterrupt signal received\n" );
	if (handle){
        libusb_release_interface (handle, 0);
        printf( "\nInterrupt signal received1\n" );
        libusb_close(handle);
        printf( "\nInterrupt signal received2\n" );
	}
	printf( "\nInterrupt signal received3\n" );
	libusb_exit(NULL);
	printf( "\nInterrupt signal received4\n" );

	exit(0);
}

int main(int argc, char **argv)
{
    //Pass Interrupt Signal to our handler
	signal(SIGINT, sighandler);

	libusb_init(&ctx);
	libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, 3);

    //Open Device with VendorID and ProductID
	handle = libusb_open_device_with_vid_pid(ctx,
				USB_VENDOR_ID, USB_PRODUCT_ID);
	if (!handle) {
		perror("device not found");
		return 1;
	}
	
	if ( libusb_detach_kernel_driver(handle, 0) ) {
			libusb_close(handle);
			libusb_free_config_descriptor(NULL);
			return 0;
    }

	int r = 1;
	//Claim Interface 0 from the device
    r = libusb_claim_interface(handle, 0);
	if (r < 0) {
		fprintf(stderr, "usb_claim_interface error %d\n", r);
		return 2;
	}
	printf("Interface claimed\n");

    memset(&transferBuf, 0x00, sizeof(transferBuf));

    transferBuf[0] = 0x03; // Op Length
	transferBuf[1] = 0x81; // command id 81
	transferBuf[2] = 0x09; // read 1 byte
	transferBuf[3] = 0x01; // read deviceID

    print_hex(transferBuf, sizeof(transferBuf));

    usb_write();
    while(1) {
        usb_read();
    }

    //never reached
    r = libusb_release_interface( handle, 0 );
    r = libusb_attach_kernel_driver( handle, 0 );
    libusb_close( handle );
	libusb_exit(NULL);

	return 0;
}
