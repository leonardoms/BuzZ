
#include  <buzz_facebook.h>

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include <oauth.h>

struct _facebook_private {
    char  *app_key;
    char  *app_secret;
    char  *token;
    CURL      *curl;
    char	*reserved;
};

struct _buzzFacebook {
    struct  _buzz  bz;
    struct  _facebook_private  *private;
};

void
buzz_connect_facebook(buzzFacebook *bzFacebook);

void
buzz_collect_facebook(buzz *self) {

}

buzzFacebook*
buzz_create_facebook(buzzData  *bzdata) {
    buzzFacebook  *bzFacebook = NULL;
    buzz         *bz = NULL;

    bzFacebook = BUZZ_FACEBOOK( malloc(sizeof(buzzFacebook)) );

    if( bzFacebook == NULL )
        return NULL;

    // populate interface with default callbacks and properties
    bz = buzz_create_default(bzdata);
    memcpy( (void*)bzFacebook, (void*)bz, sizeof(buzz) );

    bzFacebook->private = (struct  _facebook_private*)malloc(sizeof(struct  _facebook_private));
    if( bzFacebook->private == NULL ) {
          free(bzFacebook);
          return NULL;
    }

    memset((void*)bzFacebook->private, 0, sizeof(struct  _facebook_private));

    BUZZ(bzFacebook)->name = "Facebook";
    BUZZ(bzFacebook)->data = bzdata;
    BUZZ(bzFacebook)->collect = buzz_collect_facebook;

    curl_global_init(CURL_GLOBAL_ALL);

    return bzFacebook;
}
