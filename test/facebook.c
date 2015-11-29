#include <buzzData.h>
#include <buzz.h>

#define FB_POST_FIELDS  "likes_count,likes.summary(true),likes.summary.total_count;" \
                        "message,message,message;" \
                        "created_time,created_time,created_time;" \
                        "post_id,id,id;"

int main(int argc, char const *argv[]) {

    buzzData  *bzdata = BUZZ_DATA(buzz_data_create_stdio());
    if( bzdata == NULL )
        return 0;


    buzz      *bzFacebook = BUZZ(buzz_create_facebook( bzdata ));
    if( bzFacebook == NULL )
        return 0;

    buzz_write_config(BUZZ(bzFacebook), buzz_value_string_new( "facebook_app_id", "your_app_id") );
    buzz_write_config(BUZZ(bzFacebook), buzz_value_string_new( "facebook_app_secret", "your_app_secret") );
    buzz_write_config(BUZZ(bzFacebook), buzz_value_string_new( "facebook_post_id", "a_post_id") );
    buzz_write_config(BUZZ(bzFacebook), buzz_value_string_new( "facebook_post_fields", FB_POST_FIELDS) );

    buzz_collect(bzFacebook);

    return 0;
}
