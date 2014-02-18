#ifndef LIBUSBCONN
#define LIBUSBCONN

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <usb.h>
#include <glib.h>

#define CRDRDRD_SYSLOG_FACILITY	200				// this value will identify messages coming from the crdrdrd
#define CRDRDRD_LOG_LEVEL			LOG_INFO		// specify the current log level for the crdrdrd
#define CRDRDRD_APP_NAME			"crdrdrd"		// perhaps this should be specified elsewhere



/// CONSTANTS
#define CRD_RDR__VENDOR_ID	2765	// idTech minimag vendor id (unique to the product)
#define CRD_RDR__PRODUCT_ID 	512		// idTech minimag product id (unique to the product)
#define CRD_INRPT_INTERFACE 	0		// the interface the minimag exposes
#define CRD_INRPT_ENDPOINT 	0x81	// the endpoint for interrupt reading from the minimag
//#define READ_BUFFER_SIZE_B	832		// the amount of data to read during an interrupt read

/// FUNCTION PROTOTYPES ......../
int 	initializeConnection(usb_dev_handle **crdRdr_handle);                               
int 	claimCrdRdrInrptInterface(usb_dev_handle *devHandle, int useKernel);
int 	isCardReader(u_int16_t idVendor, u_int16_t idProduct);
GByteArray *blockForCardSwipe(usb_dev_handle *devHandle);
int 	getCardReader(usb_dev_handle **theCrdrdrHandle);
void 	closeConnection(usb_dev_handle *crdRdr_handle);
void 	initLibusb();

#endif
