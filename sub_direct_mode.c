#include <string.h>
#include <canna/jrkanji.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkkeysyms-compat.h>

#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>

#include "im-canna-intl.h"

gboolean im_canna_enter_direct_mode(GtkIMContext *context, GdkEventKey *key)
{
  IMContextCanna *cn = IM_CONTEXT_CANNA(context);
  gunichar keyinput = gdk_keyval_to_unicode(key->keyval);
  gchar ubuf[7];

  gtk_widget_hide(cn->modewin);
    
  if( key->state & GDK_CONTROL_MASK )
    return FALSE;
  
  if( key->keyval == 0 )
    return FALSE;

  if( !g_unichar_isprint(keyinput) ) {
    return FALSE;
  }

  /* For real char keys */
  memset(ubuf, 0, 7);
  g_unichar_to_utf8(keyinput, ubuf);
  g_signal_emit_by_name(cn, "commit", ubuf);
  return TRUE;
}
