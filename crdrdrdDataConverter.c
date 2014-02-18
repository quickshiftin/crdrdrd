#define _GNU_SOURCE 
#include <unistd.h>

#include <regex.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mxnLogger.h"
#include "includes/crdrdrdDataConverter.h"


typedef struct scan_code_type {
  int num;  //the scan code number

  char lowercase;
  char uppercase;

} scan_code_type;

#define SCAN_CODE_MAX 57
const scan_code_type scan_codes[SCAN_CODE_MAX] = {
  {0, ' ', ' '},
  {1, ' ', ' '},
  {2, ' ', ' '},
  {3, ' ', ' '},
  {4, 'a', 'A'},
  {5, 'b', 'B'},
  {6, 'c', 'C'},
  {7, 'd', 'D'},
  {8, 'e', 'E'},
  {9, 'f', 'F'},
  {10, 'g', 'G'},
  {11, 'h', 'H'},
  {12, 'i', 'I'},
  {13, 'j', 'J'},
  {14, 'k', 'K'},
  {15, 'l', 'L'},
  {16, 'm', 'M'},
  {17, 'n', 'N'},
  {18, 'o', 'O'},
  {19, 'p', 'P'},
  {20, 'q', 'Q'},
  {21, 'r', 'R'},
  {22, 's', 'S'},
  {23, 't', 'T'},
  {24, 'u', 'U'},
  {25, 'v', 'V'},
  {26, 'w', 'W'},
  {27, 'x', 'X'},
  {28, 'y', 'Y'},
  {29, 'z', 'Z'},
  {30, '1', '!'},
  {31, '2', '@'},
  {32, '3', '#'},
  {33, '4', '$'},
  {34, '5', '%'},
  {35, '6', '^'},
  {36, '7', '&'},
  {37, '8', '*'},
  {38, '9', '('},
  {39, '0', ')'},
  {40, '\n', '\n'},//return
  {41, ' ', ' '},  //escape
  {42, ' ', ' '},  //backspace
  {43, ' ', ' '},  //tab
  {44, ' ', ' '},  //space
  {45, '-', '_'},
  {46, '=', '+'},
  {47, '[', '{'},
  {48, ']', '}'},
  {49, '\\', '|'},
  {50, ' ', ' '},  //international
  {51, ';', ':'},
  {52, '\'', '"'},
  {53, ' ', ' '},  //accent and tilde
  {54, ',', '<'},
  {55, '.', '>'},
  {56, '/', '?'}
};

GString *decodeSwipe(GByteArray *swipe_data) {
  int i;
  const int reportsize=8;
  int shift=0;

  GString *decoded = g_string_sized_new(100);

//  mxnLog(MXN_INFO, "swipe was %i long", swipe_data->len);

//  mxnLog(MXN_DEBUG, "code[0]:%c", scan_codes[0]);

  //the first byte will specify array modifiers such as shift or alt
  //the third will specify the key being pressed.
  //the rest, we don't even care about.
  for (i=0; i<swipe_data->len; i+=reportsize) {
/*    mxnLog(MXN_DEBUG,"dec:%.3i %.3i %.3i %.3i %.3i %.3i %.3i %.3i",
        (int)(swipe_data->data[i+0] & 0x0ff),
        (int)(swipe_data->data[i+1] & 0x0ff),
        (int)(swipe_data->data[i+2] & 0x0ff),
        (int)(swipe_data->data[i+3] & 0x0ff),
        (int)(swipe_data->data[i+4] & 0x0ff),
        (int)(swipe_data->data[i+5] & 0x0ff),
        (int)(swipe_data->data[i+6] & 0x0ff),
        (int)(swipe_data->data[i+7] & 0x0ff));*/


    //to deal with array modifiers
    if ((swipe_data->data[i] & 0x0ff)==2) shift=1;  //TODO this may not be quite the "correct" way (but it should work)
    else shift=0;
//    mxnLog(MXN_DEBUG, "shift:%i\n",shift);

    //no actual key was pressed
    if ((swipe_data->data[i+2] & 0x0ff)==0) continue;

    if (shift) {
      g_string_append_c(decoded, scan_codes[((swipe_data->data[i+2])) & 0x0ff].uppercase);
//      mxnLog(MXN_DEBUG,"%c",scan_codes[((swipe_data->data[i+2])) & 0x0ff].uppercase);
    }
    else {
      g_string_append_c(decoded, scan_codes[((swipe_data->data[i+2])) & 0x0ff].lowercase);
//      mxnLog(MXN_DEBUG,"%c",scan_codes[((swipe_data->data[i+2])) & 0x0ff].lowercase);
    }
  }
  return decoded;
}


GString *extractCardNumber(GString *data) {
  GString *ret=NULL;
  char *beg=NULL, *end=NULL;

  //seems to be delimited by a semicolon in front of the card number
  //the ending is a question mark followed by a \n

  mxnLog(MXN_DEBUG,"extractCardNumber: data:'%s'\n",data->str);
  beg=strchr(data->str,';');
  mxnLog(MXN_DEBUG,"extractCardNumber: beg:'%s'\n",beg);

  if (beg) {
//    mxnLog(MXN_DEBUG, "found beg in position %i",beg-data->str);
    end=strchr(beg,'\n');
    mxnLog(MXN_DEBUG,"extractCardNumber: end:'%s'\n",end);

  //  if (end) mxnLog(MXN_DEBUG, "found end in position %i",end-data->str);
  }
  mxnLog(MXN_DEBUG,"extractCardNumber beg and end :'%s' '%s'\n",beg,end);
  if (beg && end) {
    beg++;
    char *tmp=strndup(beg,end-beg-1);
    mxnLog(MXN_DEBUG,"extractCardNumber tmp:'%s'\n",tmp);
    ret = g_string_new(tmp);
    free(tmp);
  }
  mxnLog(MXN_DEBUG,"extractCardNumber: %s\n",ret?ret->str:NULL);
  return ret;
}


