#include <buzzData.h>
#include <buzz.h>

#define json_twitter_columns  "created_at,created_at;id_str,id_str;text,text;in_reply_to_status_id_str,in_reply_to_status_id_str;in_reply_to_user_id_str,in_reply_to_user_id_str;" \
                      "in_reply_to_screen_name,in_reply_to_screen_name;user__id_str,user.id_str;user__screen_name,user.screen_name;user__location,user.location;" \
                      "place__id,place.id;place__country,place.country;entities__hashtags,entities.hashtags;entities__symbols,entities.symbols;" \
                      "entities__user_mentions,entities.user_mentions;lang,lang;user__followers_count,user.followers_count;user__friends_count,user.friends_count;" \
                      "user__listed_count,user.listed_count;user__geo_enabled,user.geo_enabled;user__verified,user.verified;favorite_count,favorite_count;" \
                      "retweet_count,retweet_count;timestamp_ms,timestamp_ms"

int main(int argc, char const *argv[]) {

    buzzData  *bzdata = BUZZ_DATA(buzz_data_create_pgsql());
    buzz      *bzTwitter = BUZZ(buzz_create_twitter( bzdata ));

    buzz_data_pgsql_configure( BUZZ_DATA_PGSQL(bzdata), NULL, NULL, "buzz_coca",
                               "develop", "test123", "config" );

    buzz_write_config(BUZZ(bzTwitter), buzz_value_string_new( "twitter_consumer_key", "your_consumer_key") );
    buzz_write_config(BUZZ(bzTwitter), buzz_value_string_new( "twitter_consumer_secret", "your_consumer_secret") );
    buzz_write_config(BUZZ(bzTwitter), buzz_value_string_new( "twitter_token", "your_app_token") );
    buzz_write_config(BUZZ(bzTwitter), buzz_value_string_new( "twitter_token_secret", "your_app_secret") );
    buzz_write_config(BUZZ(bzTwitter), buzz_value_string_new( "twitter_keywords", "coca,cocacola,coca-cola") );
    buzz_write_config(BUZZ(bzTwitter), buzz_value_string_new( "twitter_filter", "[cC][oO][cC][aA][ -cC]") );
    buzz_write_config(BUZZ(bzTwitter), buzz_value_string_new( "twitter_collect", json_twitter_columns ) );
    buzz_write_config(BUZZ(bzTwitter), buzz_value_string_new( "twitter_language", "pt,und" ) );

    buzz_collect(bzTwitter);

    return 0;
}
