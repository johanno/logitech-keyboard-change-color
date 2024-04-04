// Userspace USB driver for changing the Logitech G110 keyboard backlight color
// Copyleft 2011, Tom Van Braeckel <tomvanbraeckel@gmail.com>
// Credits: Tom Van Braeckel, Rich Budman
// Licensed under GPLv2

#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <stdlib.h>

// The USB sniffer logs show a value of 0x00000022 for the RequestTypeReservedBits,
// but this driver only works when using 0x00000021 (which is also what the specs dictate).
#define CONTROLFLAGS 0x21

// This drivers is for device 046d:c22b
#define VENDORID 0x046d		// Logitech
#define PRODUCTID 0xc22b	// G110 G-keys

// microseconds to wait between changing the color a little bit
#define FADESPEED 100000 	// µs

// this is the most important function, called by various functions below
static int change_color(libusb_device_handle *handle, char* buffer) {
	//int LIBUSB_CALL libusb_control_transfer(libusb_device_handle *dev_handle,
	int written = libusb_control_transfer(handle,
		// uint8_t request_type, uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
		CONTROLFLAGS, 0x00000009, 0x00000307, 0x00000000,
		// unsigned char *data, uint16_t wLength, unsigned int timeout);
		buffer, 0x00000005, 5000);

	if (written != 5) {
		fprintf(stderr, "Setting color failed, error code %d\nThis happens sporadically on some machines.\n", written);
	}
	return written;
}

// go from red to blue to red to blue to...
// brightness is fixed and determinted by prefix[4]
static void loop_through_colors(libusb_device_handle *handle, char*prefix) {
	int i;
	while(1) {
		// change the color gradually
		for (i=1;i<255;i++) {
			prefix[1]=i;
			change_color(handle,prefix);
			usleep(FADESPEED);
		}
		// and gradually back
		for (i=255;i>0;i--) {
			prefix[1]=i;
			change_color(handle,prefix);
			usleep(FADESPEED);
		}
	}
}

// Credit for this function: http://www.jespersaur.com/drupal/node/25
static struct libusb_device *findKeyboard (int vendor, int product) {
	libusb_device **devs;
	int r, cnt;

	r = libusb_init(NULL);
	if (r < 0){
		fprintf(stderr, "failed to init.");
		return NULL;
	}
	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0){
		libusb_exit(NULL);
		fprintf(stderr, "failed to get device list.");
		return NULL;
	}

  //usb_set_debug(3);
	libusb_device *dev;
	int i = 0;
	uint8_t path[8];

	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0){
			fprintf(stderr, "failed to get device descriptor");
		}
		if (desc.idVendor == vendor && desc.idProduct == product) {
			return dev;
		}
	}
	return NULL;
}

int main (int argc,char **argv) {
	fprintf(stdout, "usage: %s [color 0-255]\nSets the color. If no color is given it loops them through.\n", argv[0]);
	struct libusb_device * dev;
	dev = findKeyboard(VENDORID,PRODUCTID);
	if (dev == NULL) {
		fprintf(stderr, "Error: keyboard not found!\n");
		return 1;
	} else {
	struct libusb_device_handle *handle;
	int rc;
	rc = libusb_open(dev, &handle);
	if (LIBUSB_SUCCESS != rc) {
		fprintf (stderr, "No access to device: %s\n",
			libusb_strerror((enum libusb_error)rc));
	}
	if (dev == NULL) {
		fprintf(stderr, "Error: could not open usb device!\n");
		return 1;
	} else {

		// detach kernel driver (if there is any attached)
		int detachResult = libusb_detach_kernel_driver(handle, 0);
		if (detachResult < 0) {
			// -61 = -ENODATA = No data available ?

			// let's not fuss about this, since it probably means the device is not attached with a kernel driver
			//fprintf(stderr, "Warning: Could not detach interface 0 (errnr %d)!\n", detachResult);
		}

		// set configuration
		int setConfigResult = libusb_set_configuration(handle, 1);	// we choose 1, like the bConfigurationValue in the dump
		if (setConfigResult < 0) {
			// -16 = EBUSY = device or resource is busy
			fprintf(stderr, "Could not set configuration (errnr %d)!\n", setConfigResult);
		}

		// set single color if an argument was given, otherwise loop through colors
		unsigned char prefix [] = {0x07, 0x00, 0x00, 0x00, 0xff};
		if ( argc > 1 ) {
                     int i = atoi( argv[1] );
                     if ( i < 0 || i > 255 ) {
                         printf("Error: invalid color range (0-255)\n");
                     } else {  
                         prefix[1] = atoi( argv[1] );
                         change_color(handle, (char*)prefix);
                     }
                 } else {
                     loop_through_colors(handle, (char*)prefix);
                 }
		libusb_close(handle);
  }

  return 0;
  }
}
