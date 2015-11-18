#ifndef BUZZ_DATA_INTERFACE_H
#define BUZZ_DATA_INTERFACE_H

#include "buzzValue.h"

struct _buzzDataPrivate;
struct _buzzData;

typedef struct _buzzData   buzzData;

struct _buzzData {
    const char  *name;
    char          (*connect)(buzzData *self);
    void          (*disconnect)(buzzData *self);
    buzzValue_t*  (*read_config)(buzzData *self, const char *key);
    void          (*write_config)(buzzData *self, buzzValue_t  *buzzValue);
    void          (*write)(buzzData *self,  buzzValueList_t  *bvList);
    struct _buzzDataPrivate*  private;
};


#define BUZZ_DATA(obj)  ((buzzData*)obj)

buzzData*
buzz_data_create_default();

char
buzz_data_is_connected(buzzData*);

void
buzz_data_set_keep_connected(buzzData*, char);

char
buzz_data_get_keep_connected(buzzData*);

char
buzz_data_connect(buzzData *self);

void
buzz_data_disconnect(buzzData *self);

void
buzz_data_write_config(buzzData*, buzzValue_t*);

buzzValue_t*
buzz_data_read_config(buzzData* self, const char* key);

void
buzz_data_write(buzzData *self,  buzzValueList_t  *bvList);

#endif
