/**
 *	libusbCon.c
 *	this file uses the libusb library to communicate directly with an idTech minimag card reader.
 *	the file has methods that can identify the card reader on the system usb bus and claim it.  there are additional methods that support asycronous
 *	blocking for a swiping of a card and transfer of data from the card reader directly to this program.
 *	this library has additional files that support converting the data from the card reader into meaning full values and providing it for use by the
 *	application.
 */

#include "mxnLogger.h"	
#include "includes/libusbCon.h"

/**
 *	initializeConnection()
 *	create a connection to the card reader
 *	return 0 on failure 1 on success
 */
int initializeConnection(usb_dev_handle **crdRdr_handle) {
  mxnLog(MXN_INFO,"--> Entering initializeConnection();");

  initLibusb();								// initialize libusb

  /// get a handle to the card reader
  if(!getCardReader(crdRdr_handle)) {
    mxnLog(MXN_ERR, "ERROR getting the card reader\n");
    return 0;
  }

  return 1;
}

/**
 * closeConnection()
 *	clean up the resources consumed by initializeConnection()
 */
void closeConnection(usb_dev_handle *crdRdr_handle) {

  /// methods provided by the libusb library
  usb_release_interface(crdRdr_handle, 0);	
  usb_close(crdRdr_handle);

  /// return the data consumed by the handle to the card reader
  free(crdRdr_handle);
}

/**
 * claimCrdRdrInrptInterface()
 *	attempt to claim the card reader interface that can be used to transfer data from a card swipe
 *	return 1 on success; 0 on failure
 *	the useKernel parameter determines whether or not to detach from the kernel driver
 *	1 will continue to use the kernel; 0 will detach from the kernel
 */	
int claimCrdRdrInrptInterface(usb_dev_handle *devHandle, int useKernel) {
  int claimSuccess = 0;

  mxnLog(MXN_INFO, "--> Entering claimCrdRdrInrptInterface(); useKernel %i", useKernel);

  /// detach the kernel driver if neccessary
  if(useKernel == 0) {
    if(LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP && (usb_detach_kernel_driver_np(devHandle, (int) CRD_INRPT_INTERFACE) < 0)) {

      //will get this error if there was no kernel driver attached in the first place.
      mxnLog(MXN_DEBUG, "Could not detach kernel driver.  Make sure you have APPROPORATE PERMISIONS and the kernel driver is running when you attempt to detach it!");
      mxnLog(MXN_DEBUG, usb_strerror());
    } else {
      mxnLog(MXN_DEBUG, "Sucessfully detached from the kernel.");
    }		
  } else {
    mxnLog(MXN_DEBUG, "there was no attempt to detach from the kernel");
  }

  /// attempt to claim the interface
  mxnLog(MXN_DEBUG, "Attempting to claim the mini mag interrput interface");
  claimSuccess = usb_claim_interface(devHandle, CRD_INRPT_INTERFACE);
  mxnLog(MXN_DEBUG, "After claim attempt");

  /// analyze the results of the claim attempt
  if(claimSuccess == 0) {
    claimSuccess = 1;
    mxnLog(MXN_DEBUG, "The card reader interface was claimed!");
  } else {
    mxnLog(MXN_DEBUG, "Claim Failure!  The minimag interface with the interrupt endpoint could not be claimed.");
    claimSuccess = 0;
  }

  mxnLog(MXN_INFO, "<-- Exiting claimCrdRdrInrptInterface(); claimSuccess: %i\n", claimSuccess);

  return claimSuccess;
}

/**
 * initLibusb()
 *	run methods required to initialize the libusb library
 */
void initLibusb() {
  mxnLog(MXN_INFO, "--> Entering initLibusb(), no inputs\n");

  /// the following calls are all provided by the libusb library
  usb_init();
  usb_find_busses();
  usb_find_devices();

  mxnLog(MXN_INFO, "<-- Exiting initLibusb(), library initialized\n");
}

/**
 * isCardReader()
 *	scan the device descriptor vendor and product ids against the values know to match those of the usb
 *	card reader.  if there is a match for both values return 1, otherwise return 0.
 */
int isCardReader(u_int16_t idVendor, u_int16_t idProduct) {
  int isCardReader = 0;

  mxnLog(MXN_INFO, "--> Entering isCardReader(); idVendor: %i, idProduct: %i\n", idVendor, idProduct);

  if((idVendor == CRD_RDR__VENDOR_ID) && (idProduct == CRD_RDR__PRODUCT_ID)) {
    isCardReader = 1;
  } else {
    isCardReader = 0;
  }

  mxnLog(MXN_INFO, "<-- Exiting isCardReader(); isCardReader: %i", isCardReader);

  return isCardReader;
}

/**
 * getCardReader()
 *	loop through all available busses searching for the usb card reader
 *	if the card reader is found return a pointer to the handle
 *	other wise return NULL
 */
int getCardReader(usb_dev_handle **theCrdrdrHandle) {
  struct usb_bus 		*curBus;					// pointer to the current usb bus
  struct usb_device 		*curDev;					// pointer to the current device on the current usb bus
  int						foundCrdRdr = 0;			// status indicating that the idTechMiniMag has been discovered on the usb bus of the localhost machine
  int						handleObtained = 0;		// stauts of attempt to claim the card reader handle

  mxnLog(MXN_INFO, "--> Entering getCardReader();");



  mxnLog(MXN_INFO, "cycling through usb busses on the localhost");

  for (curBus = usb_busses; curBus; curBus = curBus->next) {// cycle through the USB busses on localhost
    mxnLog(MXN_INFO, "cycling through devices on cur bus");

    for (curDev = curBus->devices; curDev; curDev = curDev->next) {// cycle through the devices on curBus

      mxnLog(MXN_INFO, "idVendor: %i, idProduct: %i", curDev->descriptor.idVendor, curDev->descriptor.idProduct);

      if(isCardReader(curDev->descriptor.idVendor, curDev->descriptor.idProduct) == 1) {// check if curDev is the card reader
        mxnLog(MXN_DEBUG, "The card reader has been found.");

        if (!(*theCrdrdrHandle = usb_open(curDev))) {// attempt to open the card reader
          mxnLog(MXN_ERR, "Could not open the card reader!");
        } else {
          mxnLog(MXN_DEBUG, "a handle to the card reader was obtained");
          handleObtained = 1;
        }

        foundCrdRdr = 1;
        break;
      }
    }
    if(foundCrdRdr == 1) {	// no need to process additional busses after finding the card reader
      break;
    }
  }

  /*	NOTE: IT MAY BE USEFUL TO REPORT BOTH THE LOCATION STATUS AND THE HANDLE STATUS HERE AS THIS METHOD IS RESPONSIBLE FOR BOTH OF THOSE THINGS	*/ 
  /// log a message about the methods success in locating and obtaining a handle to the card reader

  if (handleObtained)
    mxnLog(MXN_INFO, "<-- Exiting getCardReader(); the card reader handle was obtained");
  else
    mxnLog(MXN_INFO, "<-- Exiting getCardReader(); the card reader handle was NOT obtained");

  /// look at some status flags to determine the appropriate return value
  if(foundCrdRdr == 0 || handleObtained == 0)
    return 0;
  else if(foundCrdRdr == 1 && handleObtained == 1)
    return 1;

  return 0;
}

//Hacky idea, since it seems to freeze on read if you read too much, and we don't
//know how much to read, we are going to read 8 bytes at a time until it stops
//sending and by doing that we are guaranteed to get everything.
GByteArray *blockForCardSwipe(usb_dev_handle *devHandle) {

  
  GByteArray *swipe_data = g_byte_array_new();
  char buf[8];
  int bufsize=8;
  int len;


//  mxnLog(MXN_DEBUG,"attempting to interrupt_read %i from card reader\n", bufsize);

  len = usb_interrupt_read(devHandle, CRD_INRPT_ENDPOINT, buf, bufsize, 0);
//  mxnLog(MXN_DEBUG, "got %i back from first interrupt_read\n",len);
  
  if (len>0) 
    g_byte_array_append(swipe_data, (unsigned char*)buf, len);
/*  else if (len==0) {
    mxnLog(MXN_CRIT, "Got back len of 0");
  }
  else if (len<0) {
  	mxnLog(MXN_CRIT, "Fatal error in read! Exiting!");
	exit(0);
  }*/

  while ((len = usb_interrupt_read(devHandle, CRD_INRPT_ENDPOINT, buf, bufsize, 1000))>0) {
    if (len>0) {
      g_byte_array_append(swipe_data, (unsigned char*)buf, len);
    }
/*  else if (len==0) {
    mxnLog(MXN_CRIT, "Got back len of 0");
  }

    else if (len<0) {
      mxnLog(MXN_CRIT, "Fatal error in read! Exiting!");
      exit(0);
    }*/
  }
//  mxnLog(MXN_DEBUG, "post block loop total:%i\n",swipe_data->len);

  return swipe_data;
}
