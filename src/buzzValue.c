
#include <buzzValue.h>

#include <stdlib.h>
#include <string.h>

struct  _buzzValueList {
  buzzValue_t *bzvalue;
  struct  _buzzValueList  *next;
};

// http://www.cprogramming.com/snippets/source-code/singly-linked-list-insert-remove-add-count
buzzValueList_t*
buzz_value_list_add(buzzValueList_t *bvlist, buzzValue_t  *bzvalue) {
    if( bzvalue == NULL )
        return bvlist;

    if( bzvalue->key == NULL && bzvalue->value == NULL )
        return bvlist;

    buzzValueList_t  *temp = (buzzValueList_t*)malloc(sizeof(buzzValueList_t));
    temp->bzvalue = bzvalue;

    if( bvlist == NULL ) {
        bvlist = temp;
        bvlist->next=NULL;
    } else {
        temp->next = bvlist;
        bvlist = temp;
    }

    return bvlist;
}

buzzValue_t*
buzz_value_list_get(buzzValueList_t *bvlist, const char* key) {
    buzzValueList_t*  lst = bvlist;
    if( lst == NULL )
        return NULL;

    if( key == NULL )
        return NULL;

    while( lst != NULL ) {
      if( strcmp(lst->bzvalue->key, key) == 0 )
        return lst->bzvalue;
      lst = lst->next;
    };
    return NULL;
}

void
buzz_value_list_foreach(buzzValueList_t *bvlist, buzzValueListCallback callback, void  *user_data ) {

    buzzValueList_t*  lst = bvlist;
    if( lst == NULL )
        return;

    if( callback == NULL )
        return;

    while( lst != NULL ) {
      callback(lst->bzvalue, user_data);

      lst = lst->next;
    };
}

void
buzz_value_list_free(buzzValueList_t *bvlist) {

}

buzzValue_t*
buzz_value_string_new(const char* key, const char* value) {
    buzzValue_t *ret = NULL;

    if( (key == NULL) && (value == NULL) )
        return NULL;

    ret = BUZZ_VALUE( malloc( sizeof(buzzValue_t) ) );

    if( ret == NULL )
        return NULL;

    if( key != NULL ) {
        ret->key = (char*)malloc( strlen(key) + 1);
        strcpy((char*)ret->key, key);
    } else
        ret->key = NULL;

    if( value != NULL ) {
        ret->value = (char*)malloc( strlen(value) + 1);
        strcpy((char*)ret->value, value);
    } else
        ret->value = NULL;

    ret->type = BUZZ_TYPE_STRING;

    return ret;
}

buzzValue_t*
buzz_value_integer_new(const char* key, int value) {
    buzzValue_t *ret = NULL;

    //if( (key == NULL) )
    //    return NULL;

    ret = BUZZ_VALUE( malloc( sizeof(buzzValue_t) ) );

    if( ret == NULL )
        return NULL;

    if( key != NULL ) {
        ret->key = (char*)malloc( strlen(key) + 1 );
        strcpy((char*)ret->key, key);
    } else
        ret->key = NULL;

    ret->value = (void*)malloc( sizeof(int) );

    *((int*)ret->value) = value;

    ret->type = BUZZ_TYPE_INTEGER;

    return ret;
}


buzzValue_t*
buzz_value_number_new(const char* key, double value) {
    buzzValue_t *ret = NULL;

    //if( (key == NULL) )
    //    return NULL;

    ret = BUZZ_VALUE( malloc( sizeof(buzzValue_t) ) );

    if( ret == NULL )
        return NULL;

    if( key != NULL )
      ret->key = (char*)malloc( strlen(key) + 1 );
    else
      ret->key = NULL;
    ret->value = (void*)malloc( sizeof(double) );

    *((double*)ret->value) = value;

    ret->type = BUZZ_TYPE_NUMBER;

    return ret;
}

void
buzz_value_free(buzzValue_t* bzvalue) {
  if(bzvalue == NULL ) return;
  if(bzvalue->key) free((void*)bzvalue->key);
  if(bzvalue->value) free(bzvalue->value);
  free((void*)bzvalue);
}
