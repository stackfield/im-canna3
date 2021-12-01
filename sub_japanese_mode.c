#include <canna/jrkanji.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkkeysyms-compat.h>

#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>

#include "im-canna-intl.h"
#include "keydefs.h"
#include "handle_canna.h"
#include "enc.h"

void routine_for_preedit_signal(GtkIMContext* context) {
  IMContextCanna *cn = IM_CONTEXT_CANNA(context);
  gint prevlen = cn->preedit_prevlen;

  cn->preedit_prevlen = cn->preedit_length;

  if(cn->preedit_length == 0 && prevlen > 0) {
    g_signal_emit_by_name(cn, "preedit_changed");
    g_signal_emit_by_name(cn, "preedit_end");
  } else if(cn->preedit_length > 0 && prevlen == 0) {
    g_signal_emit_by_name(cn, "preedit_start");
    g_signal_emit_by_name(cn, "preedit_changed");
  } else if(cn->preedit_length > 0) {
    g_signal_emit_by_name(cn, "preedit_changed");
  }

  return;
}

static gboolean
roma2kana_canna(GtkIMContext* context, gchar newinput) {
  gint nbytes;

  IMContextCanna *cn = IM_CONTEXT_CANNA(context);

  memset(cn->kakutei_buf, 0, BUFSIZ);
  nbytes = jrKanjiString(cn->canna_context, newinput, cn->kakutei_buf, BUFSIZ, &cn->ks);

  if( nbytes > 0 && !(cn->kakutei_buf[0] < 0x20)) {
    gchar* euc = g_strndup(cn->kakutei_buf, nbytes);
    gchar* utf8 = euc2utf8(euc);
    g_free(cn->commit_str);
    cn->commit_str = g_strdup(utf8);
    g_free(utf8);
    g_free(euc);

    g_signal_emit_by_name(cn, "commit", cn->commit_str);
    g_free(cn->commit_str);
    cn->commit_str = NULL;
  }

  handle_preedit(cn);
  routine_for_preedit_signal(cn);

  return TRUE;
}

gboolean
im_canna_enter_japanese_mode(GtkIMContext *context, GdkEventKey *key)
{
  IMContextCanna *cn = IM_CONTEXT_CANNA(context);
  guchar canna_code = 0;
  gboolean ret = FALSE;

  /* No preedit char yet */
  if( cn->ks.length == 0 ) {
    if( im_canna_is_key_of_emacs_like_bindkey(key) == TRUE )
      return FALSE;
    if( im_canna_is_key_need_pass_in_no_preedit(key) == TRUE )
      return FALSE;
  }

  if (im_canna_is_key_of_no_use_in_canna(key))
    return FALSE;

  canna_code = get_canna_keysym(key->keyval, key->state);

  if( canna_code != 0 )
    ret = roma2kana_canna(context, canna_code);
  else
    ret = roma2kana_canna(context, key->keyval)

  if( key->keyval == GDK_Home ) {
    im_canna_update_candwin(cn);
    gtk_widget_show(cn->candwin);
  }

  return ret;
}
