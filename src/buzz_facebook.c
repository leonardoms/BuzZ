
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
buzz_collect_facebook(buzz *self);

buzzFacebook*
buzz_create_facebook(buzzData  *bzdata) {
    buzzFacebook  *bzFacebook = NULL;
    buzz         *bz = NULL;

    if( bzdata == NULL )
        return NULL;

    bzFacebook = BUZZ_FACEBOOK( malloc(sizeof(buzzFacebook)) );

    if( bzFacebook == NULL )
        return NULL;

    // populate interface with default callbacks and properties
    bz = buzz_create_default(bzdata);
    if( bz == NULL )
        return NULL;

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

unsigned
buzz_facebook_write_atoken(char *ptr, unsigned size, unsigned nmemb, void *udata) {
    if( udata == NULL )
        return;

    if( BUZZ_FACEBOOK(udata)->private->token != NULL ) {
        free( BUZZ_FACEBOOK(udata)->private->token );
        BUZZ_FACEBOOK(udata)->private->token = NULL; // invalidate token address
    }

    BUZZ_FACEBOOK(udata)->private->token = (char*)malloc( size * nmemb );
    if( BUZZ_FACEBOOK(udata)->private->token == NULL ) {
        printf("error [%s]: get access token. (no memory)\n", BUZZ(udata)->name );
        return;
    }

    memcpy( BUZZ_FACEBOOK(udata)->private->token, ptr, size * nmemb );
    BUZZ_FACEBOOK(udata)->private->token[size * nmemb] = '\0';

    return size * nmemb;

}

void
buzz_auth_facebook(buzzFacebook *bzFacebook) {
    int status, sz;
    char  *auth_url = NULL, *app_id = NULL, *app_secret = NULL;

    app_id =      buzz_value_string( buzz_read_config(BUZZ(bzFacebook), "facebook_app_id") );
    app_secret =  buzz_value_string( buzz_read_config(BUZZ(bzFacebook), "facebook_app_secret") );

    if( app_id == NULL || app_secret == NULL ) {
        printf("error [%s]: `facebook_app_id` or `facebook_app_secret` is unset.\n", BUZZ(bzFacebook)->name );
        return;
    }

    sz = strlen(FACEBOOK_TOKEN_URL) + strlen(app_id) + strlen(app_secret) + 1;
    auth_url = (char*)malloc( sz );

    if( auth_url == NULL ) {
        printf("error [%s]: build access token url. (no memory)\n", BUZZ(bzFacebook)->name );
        return;
    }

    sprintf( auth_url, FACEBOOK_TOKEN_URL, app_id, app_secret );

    curl_global_init(CURL_GLOBAL_ALL);

    bzFacebook->private->curl = curl_easy_init();

    curl_easy_setopt(bzFacebook->private->curl, CURLOPT_USERAGENT, BUZZ_WEB_AGENT);
    curl_easy_setopt(bzFacebook->private->curl, CURLOPT_URL, auth_url);
    curl_easy_setopt(bzFacebook->private->curl, CURLOPT_FAILONERROR, 1);
    curl_easy_setopt(bzFacebook->private->curl, CURLOPT_WRITEDATA, (void*)bzFacebook);
    curl_easy_setopt(bzFacebook->private->curl, CURLOPT_WRITEFUNCTION, buzz_facebook_write_atoken);

    status = curl_easy_perform(bzFacebook->private->curl);
    if( status )
        printf("error [%s]: buzz_auth_facebook curl status #%d.\n", BUZZ(bzFacebook)->name, status );

    free(auth_url);

    curl_easy_cleanup(bzFacebook->private->curl);
    curl_global_cleanup();
}

void
buzz_connect_facebook(buzzFacebook *bzFacebook) {
  // get the access_token
  buzz_auth_facebook( bzFacebook );

}

void
buzz_collect_facebook(buzz *self) {
    //example:
    // https://graph.facebook.com/v2.5/564161453675848_916127781812545/?fields=likes&access_token=1528898687402298|fg-X7yBKZ76GWzzEif-I-qrZn9I
    // https://graph.facebook.com/v2.5/564161453675848_915897268502263/likes?access_token=1528898687402298|fg-X7yBKZ76GWzzEif-I-qrZn9I
    // https://graph.facebook.com/v2.5/pageid_postid/?fields=fields&atoken

    buzz_connect_facebook( BUZZ_FACEBOOK(self) );
    printf("%s\n", BUZZ_FACEBOOK(self)->private->token);
}
