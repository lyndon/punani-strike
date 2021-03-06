/* This file is part of punani-strike
 * Copyright (c) 2011 Gianni Tedesco
 * Released under the terms of GPLv3
*/
#include <punani/punani.h>
#include <punani/vec.h>
#include <punani/renderer.h>
#include <punani/light.h>
#include <punani/asset.h>
#include <punani/tile.h>
#include <punani/blob.h>

#include "list.h"

#define TILE_INTERNAL 1
#include "tilefile.h"
#include "dessert-stroke.h"

static LIST_HEAD(tiles);

static struct _tile *tile_ref(struct _tile *t)
{
	t->t_ref++;
	return t;
}

static struct _tile *tile_open(asset_file_t f, const char *fn)
{
	const struct tile_hdr *hdr;
	const struct tile_item *x;
	const uint8_t *names;
	struct _tile *t = NULL;
	uint8_t *buf, *end;
	unsigned int i;
	size_t sz;

	buf = blob_from_file(fn, &sz);
	if ( NULL == buf ) {
		goto out;
	}

	end = buf + sz;
	if ( buf + sizeof(*hdr) > end ) {
		con_printf("tile_open: %s: corrupt file\n", fn);
		goto out_close;
	}

	hdr = (struct tile_hdr *)buf;
	if ( hdr->h_magic != TILEFILE_MAGIC ) {
		con_printf("tile_open: %s: bad magic\n", fn);
		goto out_close;
	}

	names = buf + sizeof(*hdr);
	if ( names + hdr->h_num_assets * TILEFILE_NAMELEN > end ) {
		con_printf("tile_open: %s: corrupt file\n", fn);
		goto out_close;
	}

	x = (struct tile_item *)(names + hdr->h_num_assets * TILEFILE_NAMELEN);
	if ( (uint8_t *)(x + hdr->h_num_items) > end ) {
		con_printf("tile_open: %s: corrupt file\n", fn);
		goto out_close;
	}

	t = calloc(1, sizeof(struct _item) * hdr->h_num_items +
			sizeof(struct _tile));
	if ( NULL == t ) {
		con_printf("tile_open: %s: calloc: %s\n", fn, strerror(errno));
		goto out_close;
	}

	t->t_fn = strdup(fn);
	if ( NULL == t->t_fn ) {
		con_printf("tile_open: %s: strdup: %s\n", fn, strerror(errno));
		goto out_free;
	}

	for(i = 0; i < hdr->h_num_items; i++) {
		char *name;
		t->t_items[i].x = x[i].i_x;
		t->t_items[i].y = x[i].i_y;
		name = (char *)(names + x[i].i_asset * TILEFILE_NAMELEN);
		t->t_items[i].asset = asset_file_get(f, name);
		if ( NULL == t->t_items[i].asset )
			goto out_free_all;
		t->t_num_items++;
	}

	/* success */
	t->t_ref = 1;
	list_add_tail(&t->t_list, &tiles);
	goto out_close;

out_free_all:
	for(i = 0; i < t->t_num_items; i++) {
		asset_put(t->t_items[i].asset);
	}
	free(t->t_fn);
out_free:
	free(t);
	t = NULL;
out_close:
	blob_free(buf, sz);
out:
	return t;
}

tile_t tile_get(asset_file_t f, const char *fn)
{
	struct _tile *t;

	list_for_each_entry(t, &tiles, t_list) {
		if ( !strcmp(fn, t->t_fn) )
			return tile_ref(t);
	}

	return tile_open(f, fn);
}

void tile_put(tile_t t)
{
	if ( t ) {
		t->t_ref--;
		if ( !t->t_ref ) {
			unsigned int i;
			list_del(&t->t_list);
			for(i = 0; i < t->t_num_items; i++) {
				asset_put(t->t_items[i].asset);
			}
			free(t->t_fn);
			free(t);
		}
	}
}

int tile_collide_line(tile_t t, const vec3_t a, const vec3_t b, vec3_t hit)
{
	unsigned int i;
	int ret = 0;

	for(i = 0; i < t->t_num_items; i++) {
		struct _item *item = &t->t_items[i];
		vec3_t start, end, h;

		/* translate line segment to item space */
		v_copy(start, a);
		v_copy(end, b);
		start[0] -= item->x;
		start[2] -= item->y;
		end[0] -= item->x;
		end[2] -= item->y;

		if ( asset_collide_line(item->asset, start, end, h) ) {
			/* translate the hit point back to tile space */
			h[0] += item->x;
			h[2] += item->y;

			if ( ret ) {
				vec3_t tmp;
				float da, db;

				v_sub(tmp, a, hit);
				da = fabs(v_len(tmp));

				v_sub(tmp, a, h);
				db = fabs(v_len(tmp));

				if ( db < da )
					v_copy(hit, h);
			}else{
				v_copy(hit, h);
				ret = 1;
			}
		}
	}

	return ret;
}

int tile_collide_sphere(tile_t t, const vec3_t c, float r, vec3_t hit)
{
	unsigned int i;
	int ret = 0;

	for(i = 0; i < t->t_num_items; i++) {
		struct _item *item = &t->t_items[i];
		vec3_t c2, h;

		v_copy(c2, c);
		c2[0] -= item->x;
		c2[2] -= item->y;

		if ( asset_collide_sphere(item->asset, c2, r, h) ) {
			if ( ret ) {
				vec3_t tmp;
				float da, db;

				v_sub(tmp, c, hit);
				da = fabs(v_len(tmp));

				v_sub(tmp, c, h);
				db = fabs(v_len(tmp));

				if ( db < da )
					v_copy(hit, h);
			}else{
				v_copy(hit, h);
				ret = 1;
			}
		}
	}

	return ret;
}
