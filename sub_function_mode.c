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
  int mode, omode;

  gtk_widget_show_all (cn->candwin);
  
  if (im_canna_is_key_of_no_use_in_canna(key))
    return FALSE;

  omode = im_canna_get_num_of_canna_mode(cn);

  if (im_canna_is_key_kind_of_enter(key)) {
    canna_code = GDK_Return;
  } else {
    canna_code = get_canna_keysym(key->keyval, key->state);
  }

  if (canna_code != 0) {
    gint nbytes;
    gchar *back_gline_message = g_strdup(cn->gline_message);

    nbytes = jrKanjiString(cn->canna_context, canna_code, cn->kakutei_buf, BUFSIZ, &cn->ks);

    mode = im_canna_get_num_of_canna_mode(cn);

    if ((nbytes >= 2)) {
      gchar* euc = g_strndup(cn->kakutei_buf, nbytes);
      gchar* utf8 = euc2utf8(euc);
      g_signal_emit_by_name(cn, "commit", utf8);
      g_free(utf8);
      g_free(euc);
      
      if (key->keyval == GDK_Return) {
	if (omode == CANNA_MODE_KigoMode) {
	  g_free(cn->gline_message);
	  cn->gline_message = back_gline_message;
	  im_canna_force_change_mode(cn, CANNA_MODE_KigoMode);
	} else if (omode >= CANNA_MODE_HexMode) {
	  if (mode == CANNA_MODE_EmptyMode) {
	    im_canna_update_modewin(cn);
	    gtk_widget_show_all(cn->modewin);
	  }
	}
      }
    }

    if ((omode == CANNA_MODE_KigoMode && mode == CANNA_MODE_EmptyMode)
	|| (omode >= CANNA_MODE_HexMode && mode == CANNA_MODE_EmptyMode)) {
      gtk_widget_hide(cn->candwin);
      im_canna_update_modewin(cn);
      gtk_widget_show_all(cn->modewin);
    }

    if ((omode == CANNA_MODE_KigoMode || omode >= CANNA_MODE_HexMode)
	&& (mode != CANNA_MODE_KigoMode || !(mode >= CANNA_MODE_HexMode))) {
      im_canna_update_modewin(cn);
      gtk_widget_show_all(cn->modewin);
    }

    /*
    if ( cn->ks.length != -1 )
      cn->kslength = cn->ks.length;
    */
    
    if ( strlen(cn->kakutei_buf) == 1 && cn->kakutei_buf[0] == canna_code ) {
      cn->kakutei_buf[0] = '\0';
    }

    handle_gline(cn);
    if(cn->gline_length > 0) {
      im_canna_update_candwin(cn);
    }
    
    return TRUE;
  }
  return TRUE;
}
