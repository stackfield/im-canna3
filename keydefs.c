#include "im-canna-intl.h"
#include "keydefs.h"

gboolean im_canna_is_key_of_no_use_in_canna(GdkEventKey *key);
gboolean im_canna_is_key_of_emacs_like_bindkey(GdkEventKey *key);
gboolean im_canna_is_key_kind_of_enter(GdkEventKey *key) ;

guint get_canna_keysym(guint keyval, guint state);
gboolean im_canna_is_modechangekey(GtkIMContext *context, GdkEventKey *key);

static struct _gdk2canna_keytable {
  guint mask; /* Modifier key mask */
  guint gdk_keycode; /* Gdk key symbols */
  guint canna_keycode; /* Canna Key code or raw hex code */
} gdk2canna_keytable[] = {

  /*** Arrow Key ***/
  { MASK_NONE, GDK_Down, 0x0e }, /* Alias to GDK_Page_Down */
  { MASK_NONE, GDK_Left, 0x02 },
  { MASK_NONE, GDK_Right, 0x06 },
  { MASK_NONE, GDK_Up, 0x10 }, /* Alias to GDK_Page_Up */
  /* For Shift - Ctrl Arrow Key */
  { MASK_CONTROL, GDK_Down, CANNA_KEY_Cntrl_Down },
  { MASK_CONTROL, GDK_Left, CANNA_KEY_Cntrl_Left },
  { MASK_CONTROL, GDK_Right, CANNA_KEY_Cntrl_Right },
  { MASK_CONTROL, GDK_Right, CANNA_KEY_Cntrl_Right },
  { MASK_CONTROL, GDK_Up, CANNA_KEY_Cntrl_Up },
  { MASK_SHIFT, GDK_Down, CANNA_KEY_Shift_Down },
  { MASK_SHIFT, GDK_Left, CANNA_KEY_Shift_Left },
  { MASK_SHIFT, GDK_Right, CANNA_KEY_Shift_Right },
  { MASK_SHIFT, GDK_Right, CANNA_KEY_Shift_Right },
  { MASK_SHIFT, GDK_Up, CANNA_KEY_Shift_Up },

  { MASK_NONE, GDK_Page_Down, 0x0e },
  { MASK_NONE, GDK_Page_Up, 0x10 },
  { MASK_NONE, GDK_BackSpace, 0x08 },
  { MASK_NONE, GDK_Home, CANNA_KEY_Home  },
  { MASK_NONE, GDK_End, CANNA_KEY_End  },

  /* For Function Key */
  { MASK_NONE, GDK_KEY_F1, CANNA_KEY_F1},
  { MASK_NONE, GDK_KEY_F2, CANNA_KEY_F2},
  { MASK_NONE, GDK_KEY_F3, CANNA_KEY_F3},
  { MASK_NONE, GDK_KEY_F4, CANNA_KEY_F4},
  { MASK_NONE, GDK_KEY_F5, CANNA_KEY_F5},
  { MASK_NONE, GDK_KEY_F6, CANNA_KEY_F6},
  { MASK_NONE, GDK_KEY_F7, CANNA_KEY_F7},
  { MASK_NONE, GDK_KEY_F8, CANNA_KEY_F8},
  { MASK_NONE, GDK_KEY_F9, CANNA_KEY_F9},
  { MASK_SHIFT, GDK_KEY_F1, CANNA_KEY_F1},
  { MASK_SHIFT, GDK_KEY_F2, CANNA_KEY_F2},
  { MASK_SHIFT, GDK_KEY_F3, CANNA_KEY_F3},
  { MASK_SHIFT, GDK_KEY_F4, CANNA_KEY_F4},
  { MASK_SHIFT, GDK_KEY_F5, CANNA_KEY_F5},
  { MASK_SHIFT, GDK_KEY_F6, CANNA_KEY_F6},
  { MASK_SHIFT, GDK_KEY_F7, CANNA_KEY_F7},
  { MASK_SHIFT, GDK_KEY_F8, CANNA_KEY_F8},
  { MASK_SHIFT, GDK_KEY_F9, CANNA_KEY_F9},
  { MASK_CONTROL, GDK_KEY_F1, CANNA_KEY_F1},
  { MASK_CONTROL, GDK_KEY_F2, CANNA_KEY_F2},
  { MASK_CONTROL, GDK_KEY_F3, CANNA_KEY_F3},
  { MASK_CONTROL, GDK_KEY_F4, CANNA_KEY_F4},
  { MASK_CONTROL, GDK_KEY_F5, CANNA_KEY_F5},
  { MASK_CONTROL, GDK_KEY_F6, CANNA_KEY_F6},
  { MASK_CONTROL, GDK_KEY_F7, CANNA_KEY_F7},
  { MASK_CONTROL, GDK_KEY_F8, CANNA_KEY_F8},
  { MASK_CONTROL, GDK_KEY_F9, CANNA_KEY_F9},

  /* For Japanese 106 keyboards */
  { MASK_NONE, GDK_Muhenkan, CANNA_KEY_Nfer },
  { MASK_NONE, GDK_Henkan, CANNA_KEY_Xfer },
  { MASK_SHIFT, GDK_Muhenkan, CANNA_KEY_Shift_Nfer },
  { MASK_SHIFT, GDK_Henkan, CANNA_KEY_Shift_Xfer },
  { MASK_CONTROL, GDK_Muhenkan, CANNA_KEY_Cntrl_Nfer },
  { MASK_CONTROL, GDK_Henkan, CANNA_KEY_Cntrl_Xfer },

  { MASK_NONE, GDK_Eisu_toggle, CANNA_KEY_EISU },
  { MASK_SHIFT, GDK_Eisu_toggle, CANNA_KEY_EISU },
  { MASK_CONTROL, GDK_Eisu_toggle, CANNA_KEY_EISU },
  
  { MASK_NONE, GDK_Hiragana_Katakana, CANNA_KEY_HIRAGANA },
  { MASK_SHIFT, GDK_Hiragana_Katakana, CANNA_KEY_KATAKANA },
  { MASK_CONTROL, GDK_Hiragana_Katakana, CANNA_KEY_HIRAGANA },

  { MASK_NONE, GDK_Zenkaku_Hankaku, CANNA_KEY_HANKAKUZENKAKU },
  { MASK_SHIFT, GDK_Zenkaku_Hankaku, CANNA_KEY_HANKAKUZENKAKU },
  { MASK_CONTROL, GDK_Zenkaku_Hankaku, CANNA_KEY_HANKAKUZENKAKU },

  { MASK_NONE, 0, 0 },
};

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
  if (key->state & GDK_MOD1_MASK)
    return TRUE;

  switch (key->keyval) {
  case GDK_F10:
  case GDK_F11:
  case GDK_F12:
  case GDK_Scroll_Lock:
  case GDK_Pause:
  case GDK_Num_Lock:
  case GDK_Control_L:
  case GDK_Control_R:
  case GDK_Alt_L:
  case GDK_Alt_R:
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

#define KEYCODE_UPPER_LETTER_BASE 48
#define KEYCODE_LOWER_LETTER_BASE 96
  if (state & GDK_CONTROL_MASK) {
    if (keyval >= KEYCODE_LOWER_LETTER_BASE)
      return keyval - KEYCODE_LOWER_LETTER_BASE;
    if (keyval >= KEYCODE_UPPER_LETTER_BASE)
      return keyval - KEYCODE_UPPER_LETTER_BASE;
  }
  
  return 0;
}

/* Mode change key combination (Shift+Space etc) or not? */
gboolean im_canna_is_modechangekey(GtkIMContext *context, GdkEventKey *key) {
  IMContextCanna *cn = IM_CONTEXT_CANNA(context);

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
  /* PC-9801/21 Style - Xfer */ 
  } else if( key->keyval == GDK_Henkan ) {
    if( key->state & GDK_CONTROL_MASK && cn->ja_input_mode == TRUE)
      return TRUE;
    if( cn->ja_input_mode == FALSE )
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
      return TRUE;
      break;
    }
  
  return FALSE;
}

gboolean im_canna_is_key_need_pass_in_no_preedit (GdkEventKey *key)
{
  switch(key->keyval) {
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
    return TRUE; /* Functions key handling depends on afterward codes */
    break;
  }

  return FALSE;
}
