#ifndef CRDRDRD_DATACNVRTR
#define CRDRDRD_DATACNVRTR
 
 /// TODO: COMMENT
 /// FUNCTION PROTOTYPES ........../

#include <glib.h>

GString *decodeSwipe(GByteArray *swipe_data);
GString *extractCardNumber(GString *data);
void eraseCardData(GString *data);

#endif
