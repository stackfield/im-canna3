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

static struct _gdk2canna_keytable {
  guint mask; /* Modifier key mask */
  guint gdk_keycode; /* Gdk key symbols */
  guint canna_keycode; /* Canna Key code or raw hex code */
} gdk2canna_keytable[] = {
  { MASK_CONTROL,  GDK_c, 0x03 },
  { MASK_CONTROL,  GDK_h, 0x08 },
  { MASK_CONTROL,  GDK_u, 0x15 },
  { MASK_CONTROL,  GDK_l, 0x0c },
  { MASK_CONTROL,  GDK_k, 0x0b },
  { MASK_CONTROL,  GDK_d, 0x04 },
  { MASK_CONTROL,  GDK_o, 0x0f },
  { MASK_CONTROL,  GDK_i, 0x09 },
  { MASK_NONE, GDK_Henkan, 0x0e },

  { MASK_CONTROL,  GDK_b, 0x02 }, /* Alias to GDK_Left */
  { MASK_NONE, GDK_Up, 0x10 }, /* Alias to GDK_Page_Up */
  { MASK_NONE, GDK_Left, 0x02 },
  { MASK_CONTROL,  GDK_f, 0x06 }, /* Alias to GDK_Right */
  { MASK_NONE, GDK_Down, 0x0e }, /* Alias to GDK_Page_Down */
  { MASK_NONE, GDK_Right, 0x06 },
  { MASK_CONTROL,  GDK_n, 0x0e }, /* Alias to GDK_Page_Down */
  { MASK_NONE, GDK_Page_Down, 0x0e },
  { MASK_CONTROL,  GDK_p, 0x10 }, /* Alias to GDK_Page_Up */
  { MASK_NONE, GDK_Page_Up, 0x10 },
  { MASK_NONE, GDK_Return, 0x0D },
  { MASK_NONE, GDK_BackSpace, 0x08 },
  { MASK_CONTROL,  GDK_a, CANNA_KEY_Home }, /* Alias to GDK_Home */
  { MASK_NONE, GDK_Home, CANNA_KEY_Home  },
  { MASK_CONTROL,  GDK_e, CANNA_KEY_End }, /* Alias to GDK_End */
  { MASK_NONE, GDK_End, CANNA_KEY_End  },
  { MASK_CONTROL,  GDK_g, 0x07 },
  { MASK_NONE,  GDK_Muhenkan, 0x07 },

  { MASK_SHIFT, GDK_Right, CANNA_KEY_Shift_Right },
  { MASK_SHIFT, GDK_Left, CANNA_KEY_Shift_Left },
  { MASK_SHIFT, GDK_Mode_switch, 0x10 },

  { MASK_NONE, 0, 0 },
};

extern gboolean im_canna_is_key_kind_of_enter(GdkEventKey *key);
extern gboolean im_canna_is_key_of_emacs_like_bindkey(GdkEventKey *key);
extern gboolean im_canna_is_key_of_no_use_in_canna(GdkEventKey *key);
extern guint get_canna_keysym(guint keyval, guint state);
extern gboolean
im_canna_is_modechangekey(GtkIMContext *context, GdkEventKey *key);
#endif
