#ifndef BUZZ_FACEBOOK_H
#define BUZZ_FACEBOOK_H

#include "buzzInterface.h"
#include "buzzValue.h"

#define  FACEBOOK_PUBLIC  0
#define  FACEBOOK_USER    1

#define  FACEBOOK_TOKEN_URL "https://graph.facebook.com/oauth/access_token?client_id=%s&client_secret=%s&type=client_cred"
#define  FACEBOOK_POST_URL  "https://graph.facebook.com/v2.5/%s/?fields=%s&%s"

typedef struct _buzzFacebook   buzzFacebook;

#define BUZZ_FACEBOOK(obj)  ( (buzzFacebook*)obj )

buzzFacebook*
buzz_create_facebook();

#endif
