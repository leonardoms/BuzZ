#ifndef BUZZ_FACEBOOK_H
#define BUZZ_FACEBOOK_H

#include "buzzInterface.h"
#include "buzzValue.h"

#define  FACEBOOK_PUBLIC  0
#define  FACEBOOK_USER    1

typedef struct _buzzFacebook   buzzFacebook;

#define BUZZ_FACEBOOK(obj)  ( (buzzFacebook*)obj )

buzzFacebook*
buzz_create_facebook();

#endif
