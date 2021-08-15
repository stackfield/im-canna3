#include <canna/jrkanji.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkkeysyms-compat.h>

#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>

#include "im-canna-intl.h"
#include "keydefs.h"
#include "handle_canna.h"
#include "enc.h"

static gboolean
roma2kana_canna(GtkIMContext* context, gchar newinput) {
  gint nbytes;

  IMContextCanna *cn = IM_CONTEXT_CANNA(context);

  if( cn->kslength == 0 ) {
    bzero(cn->workbuf, sizeof(cn->workbuf));
    bzero(cn->kakutei_buf, sizeof(cn->kakutei_buf));
  }

  nbytes = jrKanjiString(cn->canna_context, newinput, cn->kakutei_buf, BUFSIZ, &cn->ks);

  if( cn->ks.length == -1 ) {
    return FALSE;
  }

  cn->kslength = cn->ks.length;
  
  if( nbytes > 0 && !(cn->kakutei_buf[0] < 0x20)) {
    gchar* euc = g_strndup(cn->kakutei_buf, nbytes);
    gchar* utf8 = euc2utf8(euc);
      g_free(cn->commit_str);
      cn->commit_str = g_strdup(utf8);
      g_free(utf8);
      g_free(euc);
  }
  
  memset(cn->workbuf, 0, BUFSIZ);
  strncpy(cn->workbuf, cn->ks.echoStr, cn->kslength);
  g_signal_emit_by_name(cn, "preedit_changed");
  
  return TRUE;
}

gboolean
im_canna_enter_japanese_mode(GtkIMContext *context, GdkEventKey *key)
{
  IMContextCanna *cn = IM_CONTEXT_CANNA(context);
  guchar canna_code = 0;
  GdkEventKey nkey = *key;
  
  /* No preedit char yet */
  if( cn->kslength == 0 ) {
    if( im_canna_is_key_of_emacs_like_bindkey(&nkey) == TRUE )
      return FALSE;
    
    switch(nkey.keyval) {
      /*
	case GDK_space:
	g_signal_emit_by_name(cn, "commit", " ");
	return TRUE;
	break;
      */
    case GDK_Return:
    case GDK_BackSpace:
    case GDK_Left:
    case GDK_Up:
    case GDK_Right:
    case GDK_Down:
    case GDK_Page_Up:
    case GDK_Page_Down:
    case GDK_End:
    case GDK_Insert:
    case GDK_Delete:
    case GDK_Shift_L:
    case GDK_Shift_R:
      return FALSE; /* Functions key handling depends on afterward codes */
      break;
    case GDK_Home:
      im_canna_update_candwin(cn);
      gtk_widget_show_all(cn->candwin);
      break;
    default:
      break;
    }
  }

  canna_code = 0;

  if (im_canna_is_key_of_no_use_in_canna(&nkey))
    return FALSE;
  
  if (im_canna_is_key_kind_of_enter(&nkey)) {
    nkey.keyval = GDK_Return;
  } else {
    canna_code = get_canna_keysym(nkey.keyval, nkey.state);
  }
#if 0
  if( canna_code != 0 ) {
    memset(cn->kakutei_buf, 0, BUFSIZ);
    jrKanjiString(cn->canna_context, canna_code, cn->kakutei_buf, BUFSIZ, &cn->ks);
    if( cn->ks.length != -1 )
      cn->kslength = cn->ks.length;
    if( strlen(cn->kakutei_buf) == 1 && cn->kakutei_buf[0] == canna_code ) {
      cn->kakutei_buf[0] = '\0';
    }

    /* NAKAI */
    if( *cn->kakutei_buf != '\0' ) {
      gchar* utf8 = euc2utf8(cn->kakutei_buf);
      g_free(cn->commit_str);
      cn->commit_str = g_strdup(utf8);
      memset(cn->kakutei_buf, 0, BUFSIZ);
      memset(cn->workbuf, 0, BUFSIZ);
      cn->kslength = 0;
      g_free(utf8);
    }

    if(cn->ks.echoStr != NULL) {
      if(*cn->ks.echoStr != '\0' || canna_code == 0x08)
	g_signal_emit_by_name(cn, "preedit_changed");
    }
    
    /*

      Dirty Hack for pre-52 firefox.
      if a user uses backspace or Ctrl-h(Emacs Keybind) to clear a preedit,
      firefox can't handle next backspace key.

    */
#ifdef USE_HACK_FOR_FIREFOX52
    if(cn->kslength == 0 && canna_code == 0x08) {
      g_signal_emit_by_name(cn, "commit", "A"); /* dummy */
      return FALSE;
    }
#endif

    return TRUE;
  }
#endif
  /* Pass char to Canna, anyway */  
  if(roma2kana_canna(context, nkey.keyval) ) {
    if (cn->commit_str != NULL) {
      g_signal_emit_by_name(cn, "commit", cn->commit_str);
      g_free(cn->commit_str);
      cn->commit_str = NULL;
    }    
    return TRUE;
  }
  return FALSE;
}
