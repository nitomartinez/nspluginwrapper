/*
 *  utils.c - Utility functions
 *
 *  nspluginwrapper (C) 2005-2007 Gwenole Beauchesne
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sysdeps.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <glib.h> /* <glib/ghash.h> */

#include "utils.h"
#include "rpc.h"

#ifndef TEST_UTILS
#define XP_UNIX 1
#define MOZ_X11 1
#include <npapi.h>
#endif

#define DEBUG 1
#include "debug.h"


/* ====================================================================== */
/* === Hashes                                                         === */
/* ====================================================================== */

static GHashTable *g_ids;

static inline void *id_key(uint32_t id)
{
  return (void *)(uintptr_t)id;
}

bool id_init(void)
{
  return (g_ids = g_hash_table_new(NULL, NULL)) != NULL;
}

void id_kill(void)
{
  if (g_ids) {
	g_hash_table_destroy(g_ids);
	g_ids = NULL;
  }
}

void id_link(int id, void *ptr)
{
  g_hash_table_insert(g_ids, id_key(id), ptr);
}

int id_create(void *ptr)
{
  static int id = 0;
  id_link(++id, ptr);
  return id;
}

bool id_remove(int id)
{
  return g_hash_table_remove(g_ids, id_key(id));
}

void *id_lookup(int id)
{
  return g_hash_table_lookup(g_ids, id_key(id));
}

static gboolean id_match_value(gpointer key, gpointer value, gpointer user_data)
{
  if (value == *(gpointer *)user_data) {
	*(int *)user_data = (uintptr_t)key;
	return true;
  }
  return false;
}

int id_lookup_value(void *ptr)
{
  return g_hash_table_find(g_ids, id_match_value, &ptr) ? (uintptr_t)ptr : -1;
}


/* ====================================================================== */
/* === String expansions                                              === */
/* ====================================================================== */

#ifndef TEST_UTILS
const char *string_of_NPError(int error)
{
  const char *str;

  switch ((NPError)error) {
#define _(VAL) case VAL: str = #VAL; break;
	_(NPERR_NO_ERROR);
	_(NPERR_GENERIC_ERROR);
	_(NPERR_INVALID_INSTANCE_ERROR);
	_(NPERR_INVALID_FUNCTABLE_ERROR);
	_(NPERR_MODULE_LOAD_FAILED_ERROR);
	_(NPERR_OUT_OF_MEMORY_ERROR);
	_(NPERR_INVALID_PLUGIN_ERROR);
	_(NPERR_INVALID_PLUGIN_DIR_ERROR);
	_(NPERR_INCOMPATIBLE_VERSION_ERROR);
	_(NPERR_INVALID_PARAM);
	_(NPERR_INVALID_URL);
	_(NPERR_FILE_NOT_FOUND);
	_(NPERR_NO_DATA);
	_(NPERR_STREAM_NOT_SEEKABLE);
#undef _
  default:
	str = "<unknown error>";
	break;
  }

  return str;
}

const char *string_of_NPReason(int reason)
{
  const char *str;

  switch ((NPReason)reason) {
#define _(VAL) case VAL: str = #VAL; break;
    _(NPRES_DONE);
    _(NPRES_NETWORK_ERR);
    _(NPRES_USER_BREAK);
#undef _
  default:
	str = "<unknown reason>";
	break;
  }

  return str;
}

const char *string_of_NPStreamType(int stype)
{
  const char *str;

  switch (stype) {
#define _(VAL) case VAL: str = #VAL; break;
    _(NP_NORMAL);
    _(NP_SEEK);
    _(NP_ASFILE);
    _(NP_ASFILEONLY);
#undef _
  default:
	str = "<unknown stream type>";
	break;
  }

  return str;
}
#endif


/* ====================================================================== */
/* === Misc utility functions                                         === */
/* ====================================================================== */

void npw_perror(const char *prefix, int error)
{
  if (prefix && *prefix)
	npw_printf("ERROR: %s: %s\n", prefix, npw_strerror(error));
  else
	npw_printf("ERROR: %s\n", npw_strerror(error));
}

const char *npw_strerror(int error)
{
  if (error > -1100 && error <= -1000)	// RPC errors
	return rpc_strerror(error);

  switch (error) {
  case 0:	return "No error";
  }

  return "Unknown error";
}


/* ====================================================================== */
/* === Test Program                                                   === */
/* ====================================================================== */

#ifdef TEST_UTILS
int main(void)
{
  char *str;
  int i, id;

  id_init();

#define N_CELLS_PER_SLOT 8
#define N_STRINGS ((2 * N_CELLS_PER_SLOT) + 3)
  char *strings[N_STRINGS];
  int ids[N_STRINGS];

  for (i = 0; i < N_STRINGS; i++) {
	str = malloc(10);
	sprintf(str, "%d", i);
	strings[i] = str;
	id = id_create(str);
	if (id < 0) {
	  fprintf(stderr, "ERROR: failed to allocate ID for '%s'\n", str);
	  return 1;
	}
	ids[i] = id;
  }

  // basic lookup
  id = ids[N_CELLS_PER_SLOT / 2];
  str = id_lookup(id);
  printf("str(%d) : '%s'\n", id, str);

  // basic unlink
  id = ids[N_CELLS_PER_SLOT];
  if (id_remove(id) < 0) {
	fprintf(stderr, "ERROR: failed to unlink ID %d\n", id);
	return 1;
  }
  ids[N_CELLS_PER_SLOT] = 0;

  // remove slot 1
  for (i = 0; i < N_CELLS_PER_SLOT; i++) {
	id = ids[N_CELLS_PER_SLOT + i];
	if (id && id_remove(id) < 0) {
	  fprintf(stderr, "ERROR: failed to unlink ID %d from slot 1\n", id);
	  return 1;
	}
	ids[N_CELLS_PER_SLOT + i] = 0;
  }

  // basic lookup after slot removal
  id = ids[2 * N_CELLS_PER_SLOT];
  str = id_lookup(id);
  printf("str(%d) : '%s'\n", id, str);

  // check slot 1 was removed and slots 0 & 2 linked together
  for (i = 0; i < N_STRINGS; i++) {
	id = ids[i];
	if (id && id_remove(id) < 0) {
	  fprintf(stderr, "ERROR: failed to unlink ID %d for final cleanup\n", id);
	  return 1;
	}
  }

  for (i = 0; i < N_STRINGS; i++)
	free(strings[i]);

  id_kill();
  return 0;
}
#endif
