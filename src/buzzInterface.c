#include <buzzInterface.h>

#include <stdlib.h>
#include <string.h>


void
buzz_collect_default(buzz* self) {

}

/* public functions */

buzz*
buzz_create_default(buzzData *bzdata) {
    buzz  *bz = NULL;

    if( bzdata == NULL )
        return NULL;

    bz = BUZZ( malloc(sizeof(buzz)) );
    if( bz == NULL )
        return NULL;

  /*  bz->private = (struct  _buzzPrivate*) malloc(sizeof(struct  _buzzPrivate));
    if( bz->private == NULL ) {
        free(bz);
        return NULL;
    }
    memset((void*)bz->private, 0, sizeof(struct _buzzPrivate));
*/
    bz->data = bzdata;

    bz->name = "DefaultBuzz";
    bz->collect = buzz_collect_default;

    return bz;
}

void
buzz_collect(buzz *self) {
    self->collect(self);
}

const char*
buzz_name(buzz *self) {
    return  self->name;
}

const char*
buzz_data_name(buzz *self) {
    return  self->data->name;
}

buzzValue_t*
buzz_read_config(buzz *self, const char* key) {
    return buzz_data_read_config(   BUZZ_DATA(self->data),
                                    key );
}

void
buzz_write_config(buzz *self, buzzValue_t* bzvalue) {
    buzz_data_write_config( BUZZ_DATA(self->data),
                            bzvalue );
}
