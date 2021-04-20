#ifndef _ENC_H
#define _ENC_H

#include <glib.h>
#include <glib/gprintf.h>

extern guchar* euc2utf8(const guchar* str);
extern gint eucpos2utf8pos(guchar* euc, gint eucpos);

extern gint im_canna_get_utf8_len_from_euc (guchar *text_euc);
extern gint im_canna_get_utf8_pos_from_euc_pos (guchar *text_euc, gint pos);

extern gboolean im_canna_is_euc_char(guchar* p);
extern gboolean im_canna_is_euc_hiragana(guchar* text);
extern gboolean im_canna_is_euc_kanji(guchar* text);

extern gint im_canna_get_pre_hiragana_len (guchar* text);
extern gint im_canna_get_post_hiragana_len (guchar* text);
extern gboolean im_canna_is_euc_hiragana_char(guchar* p);

extern int index_mb2utf8(gchar* mbstr, int revPos);
#endif
