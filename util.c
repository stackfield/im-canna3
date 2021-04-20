#include <stdio.h>

#include <glib.h>
#include "util.h"

/* Hash Table Functions */
void strhash_dumpfunc(gpointer key, gpointer value, gpointer user_data)
{
  printf("key: %s\n", (guchar*)key);
}

void strhash_removefunc(gpointer key, gpointer value, gpointer user_data)
{
  g_free(key);
}
