#include <string.h>

#include <canna/jrkanji.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkkeysyms-compat.h>

#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>

#include "im-canna-intl.h"
#include "keydefs.h"

#include "enc.h"
#include "handle_canna.h"

gboolean im_canna_function_mode (GtkIMContext *context, GdkEventKey *key) {
  IMContextCanna *cn = IM_CONTEXT_CANNA(context);
  guchar canna_code = 0;
  gint nbytes = 0;

  if (im_canna_is_key_of_no_use_in_canna(key))
    return FALSE;

  if (im_canna_is_key_kind_of_enter(key)) {
    canna_code = GDK_Return;
  } else {
    canna_code = get_canna_keysym(key->keyval, key->state);
  }

  if (canna_code != 0)
    nbytes = jrKanjiString(cn->canna_context, canna_code, cn->kakutei_buf,
			   IM_CANNA3_BUFSIZ, &cn->ks);
  else
    nbytes = jrKanjiString(cn->canna_context, key->keyval, cn->kakutei_buf,
			   IM_CANNA3_BUFSIZ, &cn->ks);

  if (nbytes >= 2) {
    gchar* euc = g_strndup(cn->kakutei_buf, nbytes);
    gchar* utf8 = euc2utf8(euc);
    g_signal_emit_by_name(cn, "commit", utf8);
    g_free(utf8);
    g_free(euc);
  }

  return TRUE;
}
