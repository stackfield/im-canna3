#include "keydefs.h"

gboolean im_canna_is_key_of_no_use_in_canna(GdkEventKey *key);
gboolean im_canna_is_key_of_emacs_like_bindkey(GdkEventKey *key);
gboolean im_canna_is_key_kind_of_enter(GdkEventKey *key) ;

guint get_canna_keysym(guint keyval, guint state);
gboolean im_canna_is_modechangekey(GtkIMContext *context, GdkEventKey *key);

gboolean im_canna_is_key_of_emacs_like_bindkey(GdkEventKey *key)
{
  if( key->state & GDK_CONTROL_MASK ) {
    switch(key->keyval) {
    case GDK_a:
    case GDK_b:
    case GDK_f:
    case GDK_e:
    case GDK_n:
    case GDK_p:
    case GDK_o:
    case GDK_h:
      return TRUE;
      break;
    }
  }

  return FALSE;
}

gboolean im_canna_is_key_of_no_use_in_canna(GdkEventKey *key)
{
  switch (key->keyval) {
  case GDK_Shift_L:
  case GDK_Shift_R:
  case GDK_F1:
  case GDK_F2:
  case GDK_F3:
  case GDK_F4:
  case GDK_F5:
  case GDK_F6:
  case GDK_F7:
  case GDK_F8:
  case GDK_F9:
  case GDK_F10:
  case GDK_F11:
  case GDK_F12:
  case GDK_Scroll_Lock:
  case GDK_Pause:
  case GDK_Num_Lock:
  case GDK_Eisu_toggle:
  case GDK_Control_L:
  case GDK_Control_R:
  case GDK_Alt_L:
  case GDK_Alt_R:
  case GDK_Hiragana_Katakana:
  case GDK_Delete:
  case GDK_Insert:
  case GDK_KP_Home:
  case GDK_KP_Divide:
  case GDK_KP_Multiply:
  case GDK_KP_Subtract:
  case GDK_KP_Add:
  case GDK_KP_Insert:
  case GDK_KP_Delete:
  case GDK_KP_Enter:
  case GDK_KP_Left:
  case GDK_KP_Up:
  case GDK_KP_Right:
  case GDK_KP_Down:
  case GDK_KP_Begin:
  case GDK_KP_Page_Up:
  case GDK_KP_Page_Down:
  case GDK_Escape:
  case 0x0000: /* Unknown(Unassigned) key */
    return TRUE;
    break;
  default:
    return FALSE;
    break;
  }
}

guint get_canna_keysym(guint keyval, guint state)
{
  guint i = 0;

  while( gdk2canna_keytable[i].gdk_keycode != 0
         && gdk2canna_keytable[i].canna_keycode != 0 ) {
    guint mask = gdk2canna_keytable[i].mask;
    if( (state & GDK_CONTROL_MASK) == (mask & GDK_CONTROL_MASK)
        && (state & GDK_SHIFT_MASK) == (mask & GDK_SHIFT_MASK)
        && gdk2canna_keytable[i].gdk_keycode == keyval ) {
      return gdk2canna_keytable[i].canna_keycode;
    }

    i++;
  }

  return keyval;
}

/* Mode change key combination (Shift+Space etc) or not? */
gboolean im_canna_is_modechangekey(GtkIMContext *context, GdkEventKey *key) {
  /* Kinput2 style - Shift + Space */
  if( key->state & GDK_SHIFT_MASK && key->keyval == GDK_space ) {
    return TRUE;
  /* Chinese/Korean style - Control + Space */
  } else if( key->state & GDK_CONTROL_MASK && key->keyval == GDK_space ) {
    return TRUE;
  /* Windows Style - Alt + Kanji */
  } else if( key->keyval == GDK_Kanji ) {
    return TRUE;
  /* Egg style - Control + '\' */
  } else if( key->state & GDK_CONTROL_MASK && key->keyval == GDK_backslash ) {
    return TRUE;
  }
  /* or should be customizable with dialog */

  return FALSE;
}

gboolean im_canna_is_key_kind_of_enter(GdkEventKey *key)
{
  /* default */
  if( key->keyval == GDK_Return )
    return TRUE;

  /* Emacs Like */
  if( key->state & GDK_CONTROL_MASK )
    switch (key->keyval) {
    case GDK_m:
    case GDK_j:
      return TRUE;
      break;
    }
 
  return FALSE;
}
