#ifndef _ENC_H
#define _ENC_H

#include <glib.h>
#include <glib/gprintf.h>

extern guchar* euc2utf8(const guchar* str);
extern gint eucpos2utf8pos(guchar* euc, gint eucpos);
extern int index_mb2utf8(gchar* mbstr, int revPos);
#endif
