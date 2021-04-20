#ifndef _UTIL_H
#define _UTIL_H
#include <glib.h>

extern void strhash_dumpfunc(gpointer key, gpointer value, gpointer user_data);
extern void strhash_removefunc(gpointer key, gpointer value, gpointer user_data);
#endif

