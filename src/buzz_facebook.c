
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
    char      *str;
    unsigned  str_offset;
    unsigned  str_blocks;
    unsigned  long  collected;
};

struct _buzzFacebook {
    struct  _buzz  bz;
    struct  _facebook_private  *private;
};

void
buzz_facebook_connect(buzzFacebook *bzFacebook);

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
buzz_facebook_auth(buzzFacebook *bzFacebook) {
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


unsigned
buzz_facebook_write_stream(char *ptr, unsigned size, unsigned nmemb, void *udata) {
    unsigned  sz = size * nmemb;
    char*     str_tmp;
    buzzFacebook *bzFacebook = BUZZ_FACEBOOK(udata);

    if( bzFacebook == NULL )
        return 0;

    if( bzFacebook->private->str_blocks > 0 ) {
        str_tmp = (char*)malloc(bzFacebook->private->str_offset + sz + 1);
        strncpy(str_tmp, bzFacebook->private->str, bzFacebook->private->str_offset);
        strncpy(str_tmp+bzFacebook->private->str_offset, ptr, sz);
        if( bzFacebook->private->str != NULL )
            free(bzFacebook->private->str);
        bzFacebook->private->str = str_tmp;
    } else {
        bzFacebook->private->str = (char*)malloc(sz + 1);
        strncpy(bzFacebook->private->str, ptr, sz);
    }

    if( bzFacebook->private->str == NULL ) {
        printf("error[%s]: `buzz_facebook_write_stream` memory not allocated.\n", BUZZ(bzFacebook)->name);
        return 0;
    }

    bzFacebook->private->str_blocks++;

    bzFacebook->private->str[bzFacebook->private->str_offset+sz] = '\0';

    bzFacebook->private->str_offset += sz;

    return sz;
}

buzzValue_t*
buzz_facebook_get_node(buzzFacebook *bzFacebook, json_t *root, char* node_name, char* node_path) {
    char  *node = NULL, *node_path_tmp = NULL;
    char  separator[] = ".";
    json_t  *n1, *n2;

    if( root == NULL || node_path == NULL )
    {
        fprintf(stderr, "error [%s]: invalid json node.\n", BUZZ(bzFacebook)->name);
        return 0;
    }

    node_path_tmp = strdup(node_path);
    node = strtok(node_path_tmp, separator);

    n1 = root;
    while( node != NULL )
    {
        n2 = n1;
        //printf("get node '%s'\n", node);
        if( json_is_object(n2) == FALSE )
            return NULL;
        n1 = json_object_get(n2, node);
      //  if( n2 != root )  //FIXME
      //    json_decref(n2);
        node = strtok(NULL, separator);
    }
    free( node_path_tmp );
    if( json_is_object(n1) || json_is_array(n1) ) {
        fprintf(stderr, "error [%s]: trying to get value from object node '%s'.\n", BUZZ(bzFacebook)->name, node_path);
        return NULL;
    }

    if( json_is_string(n1) ) {
        //printf("json_is_string(%s): %s\n", node_name, json_string_value(n1));
        return buzz_value_string_new( node_name, json_string_value(n1) );
    }

    if( json_is_integer(n1) ) {
        //printf("json_is_integer(%s): %d\n", node_name, json_integer_value(n1));
        return buzz_value_integer_new( node_name, json_integer_value(n1) );
    }

    if( json_is_real(n1) ) {
        //printf("json_is_real(%s): %g\n", node_name, json_real_value(n1));
        return buzz_value_number_new( node_name, json_real_value(n1) );
    }

    if( json_is_boolean(n1) ) {
        //printf("json_is_boolean(%s): %d\n", node_name, json_integer_value(n1));
        return buzz_value_integer_new( node_name, json_integer_value(n1) );
    }

    if( json_is_null(n1) ) {
        //printf("json_is_null(%s)\n", node_name);
        return NULL;
    }

    return NULL;
}


json_t*
buzz_facebook_get_data(buzzFacebook *bzFacebook, char *url) {
    json_t  *root;
    json_error_t  error;
    int     status;
    buzzValue_t *bzError = NULL;

    if( url == NULL ) {
        printf("error[%s]: `buzz_facebook_get_data` invalid url.\n", BUZZ(bzFacebook)->name);
        return NULL;
    }

    bzFacebook->private->curl = curl_easy_init();

    curl_easy_setopt(bzFacebook->private->curl, CURLOPT_USERAGENT, BUZZ_WEB_AGENT);
    curl_easy_setopt(bzFacebook->private->curl, CURLOPT_URL, url);
    curl_easy_setopt(bzFacebook->private->curl, CURLOPT_FAILONERROR, FALSE); // ignore HTTP errors > 400 to show Facebook API error.
    curl_easy_setopt(bzFacebook->private->curl, CURLOPT_WRITEDATA, (void*)bzFacebook);
    curl_easy_setopt(bzFacebook->private->curl, CURLOPT_WRITEFUNCTION, buzz_facebook_write_stream);

    bzFacebook->private->str_blocks = bzFacebook->private->str_offset = 0;
    if( bzFacebook->private->str != NULL ) {
        free( bzFacebook->private->str );
        bzFacebook->private->str = NULL;
    }

    status = curl_easy_perform(bzFacebook->private->curl);
    if( status ) {
        printf("error [%s]: `buzz_facebook_get_data` curl status #%d.\n", BUZZ(bzFacebook)->name, status );
        return NULL;
    } else {
        if( bzFacebook->private->str == NULL ) {
          printf("error [%s]: `buzz_facebook_get_data` get NULL HTTP response.\n", BUZZ(bzFacebook)->name );
          return NULL;
        }

//        printf("%s\n", bzFacebook->private->str);
//        printf("%s\n", url);
        root = json_loads(bzFacebook->private->str, 0, &error);
        if(!root)
        {
            fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
            return NULL;
        }

        bzError = buzz_facebook_get_node( bzFacebook, root, "error", "error.message" );
        if( bzError != NULL ) {
            printf("error [%s]: graph API error (%s)\n", BUZZ(bzFacebook)->name, buzz_value_string(bzError) );
            return NULL;
        }
    }

    char  *node_name[256], *node_path[256], *columns = NULL, *dummy = NULL;
    int i = 0, j;
    buzzValueList_t *bvlist = NULL;

    columns = strdup( buzz_value_string( buzz_read_config(BUZZ(bzFacebook), "facebook_post_fields")) );
    node_name[i] =  strtok( columns, ",;"  );
    dummy =         strtok( NULL, ",;"  );  // ignores FB API fields
    node_path[i] =  strtok( NULL, ",;"  );
    while( node_name[i] != NULL && node_path[i] != NULL ) {
        i++;
        node_name[i] =  strtok( NULL, ",;"  );
        dummy =           strtok( NULL, ",;"  );  // ignores FB API fields
        node_path[i] =    strtok( NULL, ",;"  );
    }

    for(j = 0; j < i; j++) {
        //printf("get '%s' as '%s'\n", node_name[j], node_path[j]);
        bvlist = buzz_value_list_add( bvlist, buzz_facebook_get_node(bzFacebook, root, node_name[j], node_path[j]) );
    }
    free( columns );

    BUZZ(bzFacebook)->collected++;
    buzz_data_write(BUZZ_DATA(BUZZ(bzFacebook)->data), bvlist );

    buzz_value_list_free(bvlist);
    json_decref(root);


    curl_easy_cleanup(bzFacebook->private->curl);
    curl_global_cleanup();
}

void
buzz_facebook_get_post(buzzFacebook *bzFacebook) {
    char  *url = NULL, *post_id = NULL, *post_fields = NULL;
    char  *post_fields_tmp = NULL, *fields[256], *fields_str = NULL;
    int   i = 0, j, sz;

    if( bzFacebook->private->token == NULL ) {
        printf("error [%s]: need access token.\n", BUZZ(bzFacebook)->name );
        return;
    }

    post_id =     buzz_value_string( buzz_read_config(BUZZ(bzFacebook), "facebook_post_id") );
    post_fields = buzz_value_string( buzz_read_config(BUZZ(bzFacebook), "facebook_post_fields") );

    if( post_id == NULL || post_fields == NULL ) {
        printf("error [%s]: `facebook_post_id` and `facebook_post_fields` required.\n", BUZZ(bzFacebook)->name );
        return;
    }

    post_fields_tmp = strdup( post_fields );
    strtok( post_fields_tmp, ",;" ); // ignore column name
    fields[i] = strtok( NULL, ",;" );
    strtok( NULL, ",;" ); // ignore JSON path
    while( fields[i] != NULL && i < 256 ) {
        strtok( NULL, ",;" ); // ignore column name
        fields[++i] = strtok( NULL, ",;"  );
        strtok( NULL, ",;" ); // ignore JSON path
    }

    sz = 0;
    for( j = 0; j < i; j++ )
        sz += strlen( fields[j] );


    fields_str = (char*)malloc( sz + i + 1 );
    sprintf(fields_str, "%s", fields[0]);
    for( j = 1; j < i; j++ ) {
        strcat(fields_str, ",");
        strcat(fields_str, fields[j]);
    }

    url = (char*)malloc(  strlen(fields_str) + strlen(FACEBOOK_POST_URL) +
                          strlen(bzFacebook->private->token) + strlen(post_id) + 1 );

    sprintf(url, FACEBOOK_POST_URL, post_id, fields_str, bzFacebook->private->token );
    free( post_fields_tmp );

    buzz_facebook_get_data( bzFacebook, url );

}

void
buzz_collect_facebook(buzz *self) {
    buzz_facebook_auth( BUZZ_FACEBOOK(self) );
    buzz_facebook_get_post( BUZZ_FACEBOOK(self) );
}
