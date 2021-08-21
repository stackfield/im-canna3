#ifndef _IM_CANNA_KEYDEF
#define _IM_CANNA_KEYDEF

#include <canna/jrkanji.h>

#include <glib.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkkeysyms-compat.h>

#include <gtk/gtk.h>

#define MASK_CONTROL	GDK_CONTROL_MASK
#define MASK_SHIFT	GDK_SHIFT_MASK
#define MASK_CTLSFT	(GDK_SHIFT_MASK | GDK_CONTROL_MASK)
#define MASK_NONE	0

extern gboolean im_canna_is_key_kind_of_enter(GdkEventKey *key);
extern gboolean im_canna_is_key_of_emacs_like_bindkey(GdkEventKey *key);
extern gboolean im_canna_is_key_of_no_use_in_canna(GdkEventKey *key);
extern guint get_canna_keysym(guint keyval, guint state);
extern gboolean im_canna_is_modechangekey(GtkIMContext *context, GdkEventKey *key);
extern gboolean im_canna_is_key_need_pass_in_no_preedit (GdkEventKey *key);
#endif
