
#include <buzzDataInterface.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct _buzzDataPrivate {
    char  connected, keep_connected;
    void  (*error_callback)(struct _buzzData *self, const char* error, void* user_data);
    void  *error_user_data;
};

char
buzz_data_connect_default(buzzData *self) {
    self->private->connected = TRUE;

    return self->private->connected;
}

void
buzz_data_disconnect_default(buzzData *self) {
    self->private->connected = FALSE;
}

buzzValue_t*
buzz_data_read_config_default(buzzData *self,const char *key) {
    buzzValue_t* ret = NULL;

    if( self->private->connected == FALSE )
        self->connect(self);

    ret = buzz_value_string_new(key, NULL);

    if( self->private->keep_connected == FALSE )
        self->disconnect(self);

    return ret;
}

void
buzz_data_write_config_default(buzzData *self, buzzValue_t  *buzzValue) {
    if( self->private->connected == FALSE )
        self->connect(self);

    if( self->private->keep_connected == FALSE )
        self->disconnect(self);
}

void
buzz_data_write_default(buzzData *self,  buzzValueList_t  *bvList) {
/*
    if( self->private->connected == FALSE )
        self->connect(self);

    if( self->private->keep_connected == FALSE )
        self->disconnect(self);
*/
}

/* Public Functions */

buzzData*
buzz_data_create_default() {
    buzzData  *bzdata = NULL;

    bzdata = BUZZ_DATA( malloc( sizeof(buzzData) ) );
    memset((void*)bzdata, 0, sizeof(buzzData));

    if( bzdata == NULL )
        return NULL;

    bzdata->private = (struct _buzzDataPrivate*)malloc( sizeof(struct _buzzDataPrivate) );

    if( bzdata->private == NULL ) {
      free(bzdata);
      return NULL;
    }

    //memset((void*)bzdata->private, 0, sizeof(struct _buzzDataPrivate));

    bzdata->name = "DefaultDataManager";
    bzdata->connect = buzz_data_connect_default;
    bzdata->read_config = buzz_data_read_config_default;
    bzdata->write_config = buzz_data_write_config_default;
    bzdata->write = buzz_data_write_default;
    bzdata->private->connected = 0;

    return bzdata;
}

void
buzz_data_destroy_default(buzzData *self) {

}

char
buzz_data_is_connected(buzzData *self) {
    return self->private->connected;
}

void
buzz_data_set_keep_connected(buzzData *self, char stats) {
    self->private->keep_connected = ( stats != 0 );
}

char
buzz_data_get_keep_connected(buzzData *self) {
    return self->private->keep_connected;
}

void
buzz_data_write_config(buzzData* self, buzzValue_t* bzvalue) {

    if( buzz_data_is_connected(self) == FALSE )
        if( self->connect(self) == FALSE ) {
            fprintf(stderr, "error [%s]: fails to connect.", BUZZ_DATA(self)->name );
            return;
    }

    self->write_config(self, bzvalue);
    if( buzz_data_get_keep_connected(self) == FALSE )
        self->disconnect(self);
}


buzzValue_t*
buzz_data_read_config(buzzData* self, const char* key) {
    buzzValue_t* ret = NULL;

    if( buzz_data_is_connected(self) == FALSE )
        if( self->connect(self) == FALSE ) {
            fprintf(stderr, "error [%s]: read config fails to connect.\n", BUZZ_DATA(self)->name );
            return NULL;
        }

    ret = self->read_config(self, key);

    if( buzz_data_get_keep_connected(self) == FALSE )
        self->disconnect(self);

    return ret;
}

char
buzz_data_connect(buzzData *self) {
    char ret;

    ret = self->connect(self);
    if( ret != FALSE )
      self->private->connected = 1;

    return ret;
}

void
buzz_data_disconnect(buzzData *self) {
    if( buzz_data_is_connected(self) ) {
        self->private->connected = 0;
        self->disconnect(self);
    }
}

void
buzz_data_write(buzzData* self, buzzValueList_t* bvlist) {
    if( buzz_data_is_connected(self) == FALSE )
        if( self->connect(self) == FALSE ) {
            fprintf(stderr, "error [%s]: data write fails to connect.\n", BUZZ_DATA(self)->name );
            return;
    }

    self->write(self, bvlist);
    if( buzz_data_get_keep_connected(self) == FALSE )
        self->disconnect(self);
}
