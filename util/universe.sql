CREATE TABLE universe (
	sid				serial,
	created_at			character varying(64) not null,
	id_str				character varying(32) not null,
	text				character varying(256) not null,
	in_reply_to_status_id_str	character varying(64),
	in_reply_to_user_id_str 	character varying(64),
	in_reply_to_screen_name		character varying(64),
	user__id_str			character varying(32),
	user__screen_name		character varying(64),
	user__location			character varying(128),
	user__followers_count		integer,
	user__friends_count		integer,
	user__listed_count		integer,
	user__geo_enabled		integer,
	user__verified			integer,
	place__id			character varying(32),
	place__country			character varying(32),
	entities__hashtags		character varying(140),
	entities__symbols		character varying(140),
	entities__user_mentions		character varying(140),
	favorited			integer,
	retweeted			integer,
	lang				character varying(8),
	timestamp_str			character varying(24)
);

CREATE OR REPLACE FUNCTION OnINSERTCallback() AS $$
DECLARE

BEGIN


END
$$ LANGUAGE plpgsql;
