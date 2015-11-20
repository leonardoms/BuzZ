#ifndef BUZZ_VALUE_H
#define BUZZ_VALUE_H

#ifndef NULL
#define NULL  ( (void*)0 )
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define buzz_value_integer(obj) ( (obj != NULL) ? (int)(*((int*)obj->value)) : 0 )
#define buzz_value_number(obj)  ( (obj != NULL) ? (double)(*((double*)obj->value)) : 0.0 )
#define buzz_value_string(obj)  ( (obj != NULL) ? (char*)obj->value : (char*)NULL )

typedef enum { BUZZ_TYPE_VOID = 0, BUZZ_TYPE_INTEGER, BUZZ_TYPE_NUMBER, BUZZ_TYPE_STRING } buzzValueType;

struct _buzzValue {
    buzzValueType type;
    const char  *key;
    void        *value;
};

typedef struct _buzzValue  buzzValue_t;

typedef struct _buzzValueList   buzzValueList_t;

typedef void (*buzzValueListCallback)(buzzValue_t*, void *user_data);// buzzValueListCallback_t;

#define BUZZ_VALUE(obj)   ( (buzzValue_t*)obj )

buzzValue_t*
buzz_value_string_new(const char* key, const char* value);

buzzValue_t*
buzz_value_integer_new(const char* key, int value);

buzzValue_t*
buzz_value_number_new(const char* key, double value);

void
buzz_value_free(buzzValue_t*);

buzzValueList_t*
buzz_value_list_add(buzzValueList_t *bvlist, buzzValue_t  *bzvalue);

buzzValue_t*
buzz_value_list_get(buzzValueList_t *bvlist, const char* key);

void
buzz_value_list_foreach(buzzValueList_t *bvlist, buzzValueListCallback callback, void  *user_data );

void
buzz_value_list_free(buzzValueList_t *bvlist);

#endif
