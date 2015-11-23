#ifndef BUZZ_INTERFACE_H
#define BUZZ_INTERFACE_H

#include "buzzDataInterface.h"
#include "buzzValue.h"

#define BUZZ_WEB_AGENT  "buzzMiner/0.1"

typedef struct _buzz buzz;

struct _buzz {
    const char  *name;
    void        (*collect)(buzz*  self);
    buzzData    *data;
    unsigned long long  collected;
};

#define BUZZ(obj)  ((buzz*)obj)

buzz*
buzz_create_default(buzzData *bzdata);

void
buzz_collect(buzz *self);

const char*
buzz_name(buzz *self);

const char*
buzz_data_name(buzz *self);

buzzValue_t*
buzz_read_config(buzz *self, const char* key);

void
buzz_write_config(buzz *self, buzzValue_t* bzvalue);

#endif
