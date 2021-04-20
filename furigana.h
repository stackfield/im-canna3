#ifndef _FURIGANA_H
#define _FURIGANA_H

#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkkeysyms-compat.h>

#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>

#include <im-canna-intl.h>

typedef struct _Furigana {
  gchar* text; /* Furigana is in UTF-8 */
  gint foffset; /* Unused */
  gint offset; /* Offset is in UTF-8 chars, not bytes */
  gint length; /* Length is in UTF-8 chars, not bytes */
} Furigana;

void furigana_add_offset(GSList* furigana_slist, gint offsetdiff);

extern GSList* discard_strict_furigana(gchar* ptext, GSList* furigana_slist);
extern GSList* im_canna_get_furigana(IMContextCanna* cn);
#endif /* _FURIGANA_H */
