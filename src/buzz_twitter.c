
#include  <buzz_twitter.h>

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include <oauth.h>
#include <regex.h>

struct _twitter_private {
    char  *consumer_key;
    char  *consumer_secret;
    char  *token;
    char  *token_secret;
    char  *keywords;
    char  *language;
    char  *filter;
    int    method;
    char  *columns;
    char  reconnect;
    char  *url;
    CURL      *curl;
    char      *str;
    unsigned  str_offset;
    unsigned  str_blocks;
    regex_t   reg;
};

struct _buzzTwitter {
    struct  _buzz  bz;
    struct  _twitter_private  *private;
};

void
buzz_connect_twitter(buzzTwitter *bzTwitter);

void
buzz_collect_twitter(buzz *self) {

    BUZZ_TWITTER(self)->private->consumer_key =         buzz_value_string( buzz_read_config(self, "twitter_consumer_key") );
    BUZZ_TWITTER(self)->private->consumer_secret =      buzz_value_string( buzz_read_config(self, "twitter_consumer_secret") );
    BUZZ_TWITTER(self)->private->token =           buzz_value_string( buzz_read_config(self, "twitter_token") );
    BUZZ_TWITTER(self)->private->token_secret =    buzz_value_string( buzz_read_config(self, "twitter_token_secret") );
    BUZZ_TWITTER(self)->private->keywords =        buzz_value_string( buzz_read_config(self, "twitter_keywords") );
    BUZZ_TWITTER(self)->private->filter =          buzz_value_string( buzz_read_config(self, "twitter_filter") );
    BUZZ_TWITTER(self)->private->method =          buzz_value_integer( buzz_read_config(self, "twitter_method") );
    BUZZ_TWITTER(self)->private->columns =         buzz_value_string( buzz_read_config(self, "twitter_collect") );
    BUZZ_TWITTER(self)->private->language =        buzz_value_string( buzz_read_config(self, "twitter_language") );
    BUZZ_TWITTER(self)->private->reconnect = TRUE;

    if( BUZZ_TWITTER(self)->private->consumer_key == NULL ||
        BUZZ_TWITTER(self)->private->consumer_secret == NULL ||
        BUZZ_TWITTER(self)->private->token == NULL ||
        BUZZ_TWITTER(self)->private->token_secret == NULL ||
        BUZZ_TWITTER(self)->private->keywords == NULL ) {
            printf("error [%s]: unconfigured buzzManager.\n", BUZZ(self)->name );
            return;
        }

    if( BUZZ_TWITTER(self)->private->method < 0 || BUZZ_TWITTER(self)->private->method > 1 ) {
          BUZZ_TWITTER(self)->private->method = TWITTER_STREAM;
    }

    if( BUZZ_TWITTER(self)->private->method == TWITTER_STREAM )
        BUZZ_TWITTER(self)->private->url = buzz_value_string( buzz_read_config(self, "twitter_stream_url") );
    else
        BUZZ_TWITTER(self)->private->url = buzz_value_string( buzz_read_config(self, "twitter_search_url") );

    if( BUZZ_TWITTER(self)->private->url == NULL )
        BUZZ_TWITTER(self)->private->url = BUZZ_TWITTER(self)->private->method ?
                  TWITTER_SEARCH_URL : TWITTER_STREAM_URL;

    buzz_connect_twitter( BUZZ_TWITTER(self) );

}

buzzTwitter*
buzz_create_twitter(buzzData  *bzdata) {
    buzzTwitter  *bzTwitter = NULL;
    buzz         *bz = NULL;

    bzTwitter = BUZZ_TWITTER( malloc(sizeof(buzzTwitter)) );

    if( bzTwitter == NULL )
        return NULL;

    // populate interface with default callbacks and properties
    bz = buzz_create_default(bzdata);
    memcpy( (void*)bzTwitter, (void*)bz, sizeof(buzz) );

    bzTwitter->private = (struct  _twitter_private*)malloc(sizeof(struct  _twitter_private));
    if( bzTwitter->private == NULL ) {
          free(bzTwitter);
          return NULL;
    }

    memset((void*)bzTwitter->private, 0, sizeof(struct  _twitter_private));

    BUZZ(bzTwitter)->name = "Twitter";
    BUZZ(bzTwitter)->data = bzdata;
    BUZZ(bzTwitter)->collect = buzz_collect_twitter;

    curl_global_init(CURL_GLOBAL_ALL);

    return bzTwitter;
}

buzzValue_t*
buzz_twitter_get_node(buzzTwitter *bzTwitter, json_t *root, char* node_name, char* node_path) {
    char  *node = NULL, *node_path_tmp = NULL;
    char  separator[] = ".";
    json_t  *n1, *n2;

    if( root == NULL || node_path == NULL )
    {
        fprintf(stderr, "error [%s]: invalid json node.\n", BUZZ(bzTwitter)->name);
        return 0;
    }

    node_path_tmp = strdup(node_path);
    node = strtok(node_path_tmp, separator);

    n1 = root;
    while( node != NULL )
    {
        n2 = n1;
        if( json_is_object(n2) == FALSE )
            return NULL;
        //printf("get node '%s'\n", node);
        n1 = json_object_get(n2, node);
      //  if( n2 != root )  //FIXME
      //    json_decref(n2);
        node = strtok(NULL, separator);
    }
    free( node_path_tmp );
    if( json_is_object(n1) || json_is_array(n1) ) {
        fprintf(stderr, "error [%s]: trying to get value from object node '%s'.\n", BUZZ(bzTwitter)->name, node_path);
        return NULL;
    }

    if( json_is_null(n1) ) {
        return NULL;
    }

    if( json_is_string(n1) ) {
        return buzz_value_string_new( node_name, json_string_value(n1) );
    }

    if( json_is_integer(n1) ) {
        return buzz_value_integer_new( node_name, json_integer_value(n1) );
    }

    if( json_is_real(n1) ) {
        return buzz_value_number_new( node_name, json_real_value(n1) );
    }

    if( json_is_boolean(n1) ) {
        return buzz_value_integer_new( node_name, json_integer_value(n1) );
    }

    return NULL;
}

unsigned char buzz_twitter_stream_matchs(buzzTwitter *bzTwitter, const char* stream_str) {
    json_t  *root, *text;
    json_error_t  error;
    const char    *text_str = NULL;

    if( stream_str == NULL )
        return 0;

    if( stream_str[0] != '[' && stream_str[0] != '{')
        return 0;

    root = json_loads(stream_str, 0, &error);
    if(!root)
    {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return 0;
    }

    text = json_object_get(root, "text");
    if(!json_is_string(text))
    {
        fprintf(stderr, "error: invalid tweet text\n");
        json_decref(root);
        return 0;
    }

    text_str = json_string_value(text);

    if( bzTwitter->private->filter != NULL ) {
        if( regcomp(&bzTwitter->private->reg , bzTwitter->private->filter, REG_EXTENDED|REG_NOSUB) != 0 ) {
            fprintf(stderr,"error [%s]: filter expression is invalid!\n", BUZZ(bzTwitter)->name );
            json_decref(root);
            return 0;
        }
        if ( (regexec(&bzTwitter->private->reg, text_str, 0, (regmatch_t *)NULL, 0)) != 0 ) {
            json_decref(root);
            return 0;
        }
    }

    char  *node_name[256], *node_path[256], *columns = NULL;
    int i = 0, j;
    buzzValueList_t *bvlist = NULL;

    columns = strdup( bzTwitter->private->columns );
    node_name[i] = strtok( columns, ",;"  );
    node_path[i] = strtok( NULL, ",;"  );
    while( node_name[i] != NULL && node_path[i] != NULL ) {
      //  printf("declared '%s' as '%s'\n", node_name[i], node_path[i]);
        node_name[++i] = strtok( NULL, ",;"  );
        node_path[i] = strtok( NULL, ",;"  );
    }

    for(j = 0; j < i; j++) {
        //printf("get '%s' as '%s'\n", node_name[j], node_path[j]);
        bvlist = buzz_value_list_add( bvlist, buzz_twitter_get_node(bzTwitter, root, node_name[j], node_path[j]) );
    }
    free( columns );

    BUZZ(bzTwitter)->collected++;
    buzz_data_write(BUZZ_DATA(BUZZ(bzTwitter)->data), bvlist );

    buzz_value_list_free(bvlist);
    json_decref(root);

    return 1;

}

unsigned
buzz_twitter_write_stream(char *ptr, unsigned size, unsigned nmemb, void *udata) {

    unsigned  sz = size * nmemb;
    char*     str_tmp;
    buzzTwitter *bzTwitter = BUZZ_TWITTER(udata);

    if( bzTwitter == NULL )
        return 0;

    if( bzTwitter->private->str_blocks > 0 ) {
        str_tmp = (char*)malloc(bzTwitter->private->str_offset + sz + 1);
        strncpy(str_tmp, bzTwitter->private->str, bzTwitter->private->str_offset);
        strncpy(str_tmp+bzTwitter->private->str_offset, ptr, sz);
        if( bzTwitter->private->str != NULL )
            free(bzTwitter->private->str);
        bzTwitter->private->str = str_tmp;
    } else {
        bzTwitter->private->str = (char*)malloc(sz + 1);
        strncpy(bzTwitter->private->str, ptr, sz);
    }

    if( bzTwitter->private->str == NULL ) {
        printf("error[%s]: memory not allocated. [buzz_write_stream]\n", BUZZ(bzTwitter)->name);
    }

    bzTwitter->private->str_blocks++;

    if(bzTwitter->private->str[bzTwitter->private->str_offset+sz-1] == '\n') {// until next statuses entry
        bzTwitter->private->str[bzTwitter->private->str_offset+sz] = '\0';

        //printf("verificando -> [[[%s]]] \n", bzTwitter->private->str);
        buzz_twitter_stream_matchs( bzTwitter, bzTwitter->private->str );
        if( bzTwitter->private->str != NULL )
            free(bzTwitter->private->str);
        bzTwitter->private->str = NULL;
        bzTwitter->private->str_offset = 0;
        bzTwitter->private->str_blocks = 0;
    } else {
        bzTwitter->private->str_offset += sz;
    }
    return sz;
}

void
buzz_auth_twitter(buzzTwitter *bzTwitter) {
    char *url = NULL, *lang = NULL;

    if(bzTwitter->private->language == NULL)
        lang = "";
    else
        lang = bzTwitter->private->language;

    url = (char*)malloc(strlen(bzTwitter->private->url)+strlen(bzTwitter->private->keywords)+strlen(lang));
    if( url == NULL )
        return;

    sprintf( (char*)url, bzTwitter->private->url, bzTwitter->private->keywords, lang );

    printf("url: %s\n", url);

    curl_easy_setopt( bzTwitter->private->curl, CURLOPT_URL,
        oauth_sign_url2(  url, NULL, OA_HMAC, "GET",
                          bzTwitter->private->consumer_key,
                          bzTwitter->private->consumer_secret,
                          bzTwitter->private->token,
                          bzTwitter->private->token_secret)
                        );
}

void
buzz_connect_twitter(buzzTwitter *bzTwitter) {
    int status;

    curl_global_init(CURL_GLOBAL_ALL);

    bzTwitter->private->curl = curl_easy_init();

    buzz_auth_twitter( bzTwitter );

    curl_easy_setopt(bzTwitter->private->curl, CURLOPT_USERAGENT, BUZZ_WEB_AGENT);
   	curl_easy_setopt(bzTwitter->private->curl, CURLOPT_FAILONERROR, 1);
    curl_easy_setopt(bzTwitter->private->curl, CURLOPT_WRITEDATA, (void*)bzTwitter);
    curl_easy_setopt(bzTwitter->private->curl, CURLOPT_WRITEFUNCTION, buzz_twitter_write_stream);

    do {
        status = curl_easy_perform(bzTwitter->private->curl);
        if( status )
            printf("error [%s]: curl status #%d.\n", BUZZ(bzTwitter)->name, status );
    } while ( bzTwitter->private->reconnect );

    curl_easy_cleanup(bzTwitter->private->curl);
    curl_global_cleanup();
}
