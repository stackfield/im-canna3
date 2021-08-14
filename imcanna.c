/* GTK - The GIMP Toolkit
 * Copyright (C) 2000 Red Hat Software
 * Copyright (C) 2002 Yusuke Tabata
 * Copyright (C) 2003-2004 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors: Owen Taylor <otaylor@redhat.com>
 *          Yusuke Tabata <tee@kuis.kyoto-u.ac.jp>
 *          Yukihiro Nakai <ynakai@redhat.com>
 *
 */

#include <stdio.h>

#include <string.h>
#include <canna/jrkanji.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkkeysyms-compat.h>

#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>

#include "im-canna-intl.h"
#include "keydefs.h"
#include "handle_canna.h"

#include "enc.h"
#include "util.h"

#ifdef BUFSIZ
#undef BUFSIZ
#define BUFSIZ 1024
#endif

#include "furigana.h"
#include "imsss.h"

GType type_canna = 0;

/*
static guint snooper_id = 0;
*/

static GtkIMContext* focused_context = NULL;

static void im_canna_class_init (GtkIMContextClass *class);
static void im_canna_init (GtkIMContext *im_context);
static void im_canna_finalize(GObject *obj);
static gboolean im_canna_filter_keypress(GtkIMContext *context,
		                GdkEventKey *key);
static void im_canna_get_preedit_string(GtkIMContext *, gchar **str,
					PangoAttrList **attrs,
					gint *cursor_pos);
static void im_canna_focus_in(GtkIMContext* context);
static void im_canna_focus_out(GtkIMContext* context);
static void im_canna_set_client_window(GtkIMContext* context, GdkWindow *win);
static void im_canna_set_cursor_location (GtkIMContext *context,
					  GdkRectangle *area);
static void im_canna_reset(GtkIMContext* context);
static gboolean return_false(GtkIMContext* context, GdkEventKey* key);
static void im_canna_kill(IMContextCanna* cn);

static guint focus_id = 0;

/* sub_function_mode.c */
extern gboolean im_canna_function_mode (GtkIMContext *context, GdkEventKey *key);

/* sub_cand_modewin.c */
extern void im_canna_update_candwin(IMContextCanna* cn);
extern void im_canna_update_modewin(IMContextCanna* cn);
extern void im_canna_move_modewin(IMContextCanna* cn);

static void
scroll_cb(GtkWidget* widget, GdkEventScroll* event, IMContextCanna* cn) {
  switch(event->direction) {
  case GDK_SCROLL_UP:
    jrKanjiString(cn->canna_context, 0x02, cn->kakutei_buf, BUFSIZ, &cn->ks);
    break;
  case GDK_SCROLL_DOWN:
    jrKanjiString(cn->canna_context, 0x06, cn->kakutei_buf, BUFSIZ, &cn->ks);
    break;
  default:
    break;
  }
}

gboolean
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

  handle_modebuf(cn);
  im_canna_update_modewin(cn);
  
  handle_gline(cn);

  if(cn->gline_length > 0) {
    im_canna_update_candwin(cn);
  }
  
  return TRUE;
}

void
im_canna_register_type (GTypeModule *module)
{
  static const GTypeInfo object_info =
  {
    sizeof (IMContextCannaClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) im_canna_class_init,
    NULL,           /* class_finalize */
    NULL,           /* class_data */
    sizeof (IMContextCanna),
    0,
    (GInstanceInitFunc) im_canna_init,
  };

  type_canna = 
    g_type_module_register_type (module,
				 GTK_TYPE_IM_CONTEXT,
				 "IMContextCanna",
				 &object_info, 0);
}

static void
im_canna_class_init (GtkIMContextClass *class)
{
  GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS(class);
  GObjectClass *object_class = G_OBJECT_CLASS(class);

  im_context_class->set_client_window = im_canna_set_client_window;
  im_context_class->filter_keypress = im_canna_filter_keypress;
  im_context_class->get_preedit_string = im_canna_get_preedit_string;
  im_context_class->focus_in = im_canna_focus_in;
  im_context_class->focus_out = im_canna_focus_out;
  im_context_class->reset = im_canna_reset;
  im_context_class->set_cursor_location = im_canna_set_cursor_location;

  object_class->finalize = im_canna_finalize;
}

static void
im_canna_init (GtkIMContext *im_context)
{
  IMContextCanna *cn = IM_CONTEXT_CANNA(im_context);

  PangoAttrList* attrs;
  PangoAttribute* attr;

  cn->canna_context = (int)cn;
  cn->cand_stat = 0;
  cn->kslength = 0;
  cn->workbuf = g_new0(guchar, BUFSIZ);
  cn->kakutei_buf = g_new0(guchar, BUFSIZ);

  cn->modebuf_utf8 = NULL;

  cn->gline_message = g_strdup("TEST");
  cn->gline_revPos = cn->gline_revLen = 0;
  cn->gline_length = 0;
  
  jrKanjiControl(cn->canna_context, KC_INITIALIZE, 0);
  jrKanjiControl(cn->canna_context, KC_SETWIDTH, 62);
  
  cn->candwin = gtk_window_new(GTK_WINDOW_POPUP);  
  cn->candlabel = gtk_label_new("");
  gtk_container_add(GTK_CONTAINER(cn->candwin), cn->candlabel);
  cn->candwin_area.x = cn->candwin_area.y = 0;
  cn->candwin_area.width = cn->candwin_area.width = 100;

  gtk_widget_add_events(cn->candwin, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(cn->candwin, "scroll_event", G_CALLBACK(scroll_cb), cn);

  cn->layout  = gtk_widget_create_pango_layout(cn->candwin, "");
  cn->client_window = NULL;
  cn->focus_in_candwin_show = FALSE;
  cn->ja_input_mode = FALSE;
  cn->modewin = gtk_window_new(GTK_WINDOW_POPUP);
  cn->modelabel = gtk_label_new("");
  gtk_container_add(GTK_CONTAINER(cn->modewin), cn->modelabel);

  im_canna_force_change_mode(cn, CANNA_MODE_HenkanMode);
  im_canna_update_modewin(cn);

  cn->commit_str = NULL;
}

static void
im_canna_finalize(GObject *obj) {
  IMContextCanna* cn = IM_CONTEXT_CANNA(obj);

  jrKanjiControl(cn->canna_context, KC_FINALIZE, 0);

  g_free(cn->gline_message);
  g_free(cn->modebuf_utf8);
  g_free(cn->commit_str);
  g_free(cn->workbuf);
  g_free(cn->kakutei_buf);
  gtk_widget_destroy(cn->candlabel);
  gtk_widget_destroy(cn->candwin);
  g_object_unref(cn->layout);
  gtk_widget_destroy(cn->modelabel);
  gtk_widget_destroy(cn->modewin);
}

static const GtkIMContextInfo im_canna_info = { 
  "Canna",		   /* ID */
  N_("Canna"),             /* Human readable name */
  "im-canna",		   /* Translation domain */
   IM_LOCALEDIR,	   /* Dir for bindtextdomain (not strictly needed for "gtk+") */
   "ja"			   /* Languages for which this module is the default */
};

static const GtkIMContextInfo *info_list[] = {
  &im_canna_info
};

void
im_module_init (GTypeModule *module)
{
  im_canna_register_type(module);
}

void 
im_module_exit (void)
{
}

void 
im_module_list (const GtkIMContextInfo ***contexts,
		int                      *n_contexts)
{
  *contexts = info_list;
  *n_contexts = G_N_ELEMENTS (info_list);
}

GtkIMContext *
im_module_create (const gchar *context_id)
{
  if (strcmp (context_id, "Canna") == 0)
    return GTK_IM_CONTEXT (g_object_new (type_canna, NULL));
  else
    return NULL;
}

gboolean
im_canna_filter_keypress(GtkIMContext *context, GdkEventKey *key)
{
  IMContextCanna *cn = IM_CONTEXT_CANNA(context);
  guchar canna_code = 0;
  int mode = -1;

  if( key->type == GDK_KEY_RELEASE ) {
    return FALSE;
  };
  
  /* Editable widget should pass mnemonic if ja-input-mode is on */
  g_object_set_data(G_OBJECT(context), "immodule-needs-mnemonic",
      (gpointer)cn->ja_input_mode);
  
  if (im_canna_is_modechangekey(context, key)) {
    if( cn->ja_input_mode == FALSE ) {
      im_canna_force_change_mode(cn, CANNA_MODE_HenkanMode);
      cn->ja_input_mode = TRUE;
      memset(cn->workbuf, 0, BUFSIZ);
      memset(cn->kakutei_buf, 0, BUFSIZ);
      g_signal_emit_by_name(cn, "preedit_changed");
      g_signal_emit_by_name(cn, "preedit_start");
      handle_modebuf(cn);
      im_canna_update_modewin(cn);
      gtk_widget_show(cn->modewin);
    } else {
      im_canna_force_change_mode(cn, CANNA_MODE_HenkanMode);
      cn->ja_input_mode = FALSE;
      gtk_widget_hide(cn->candwin);
      memset(cn->workbuf, 0, BUFSIZ);
      memset(cn->kakutei_buf, 0, BUFSIZ);
      im_canna_update_modewin(cn);
      gtk_widget_hide(cn->modewin);
      g_signal_emit_by_name(cn, "preedit_changed");
      g_signal_emit_by_name(cn, "preedit_end");
    }    
    return TRUE;
  }

  mode = im_canna_get_num_of_canna_mode(cn);

  /* Function mode */
  if (mode >= CANNA_MODE_HexMode || mode == CANNA_MODE_KigoMode) {
    return im_canna_function_mode(context, key);
  } else {  
  handle_modebuf(cn);
  im_canna_update_modewin(cn);
  }
  
  /* English mode */
  if( cn->ja_input_mode == FALSE ) {
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

  /*** Japanese mode ***/
  /* No preedit char yet */
  if( cn->kslength == 0 ) {
    gchar ubuf[7];
    gunichar keyinput;
    memset(ubuf, 0, 7);

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
        return FALSE;
        break;
      default:
        break;
      }
    }
    
    switch(key->keyval) {
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

  switch(key->keyval) {
  case GDK_Return:    
    if( !gtk_widget_get_visible(cn->candwin) ) {
      /*
       * Disable Furigana support code because it doesn't make sense
       * when kakutei-if-end-of-bunsetsu is set as 't' in ~/.canna
       * 
       */
#if 0
      im_canna_get_furigana(cn);
#endif      
    }
    break;
  default:
    if (im_canna_is_key_of_no_use_in_canna(key))
      return FALSE;
    else
      canna_code = get_canna_keysym(key->keyval, key->state);
    break;
  }
  
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
    
    mode = im_canna_get_num_of_canna_mode(cn);
    if(mode >= CANNA_MODE_HexMode || mode == CANNA_MODE_KigoMode) {
      handle_gline(cn);
      im_canna_update_candwin(cn);
      gtk_widget_hide(cn->modewin);
      gtk_widget_show_all(cn->candwin);
    } else {
      gtk_widget_show_all(cn->modewin);
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

  /* Pass char to Canna, anyway */  
  if(roma2kana_canna(context, key->keyval) ) {
    if (cn->commit_str != NULL) {
      g_signal_emit_by_name(cn, "commit", cn->commit_str);
      g_free(cn->commit_str);
      cn->commit_str = NULL;
    }
    
    mode = im_canna_get_num_of_canna_mode(cn);
    if(mode >= CANNA_MODE_HexMode || mode == CANNA_MODE_KigoMode) {
      handle_gline(cn);
      im_canna_update_candwin(cn);
      gtk_widget_hide(cn->modewin);
      gtk_widget_show_all(cn->candwin);
    } else {
      gtk_widget_show_all(cn->modewin);
    }
    return TRUE;
  }
  return FALSE;
}

void
im_canna_get_preedit_string(GtkIMContext *ic, gchar **str,
			    PangoAttrList **attrs, gint *cursor_pos)
{
  IMContextCanna *cn = IM_CONTEXT_CANNA(ic);
  gchar* eucstr = NULL;
  gint mode;
  
  if (attrs) {
    *attrs = pango_attr_list_new();
  }

  if (cursor_pos != NULL) {
    *cursor_pos = 0;
  }
  
  if (cn->kslength == 0) {
    *str = g_strdup("");
    return;
  }

  if (cn->ks.echoStr == NULL || *cn->ks.echoStr == '\0') {
    *str = g_strdup("");
    return;
  }

  if (((guint16 *)cn->ks.echoStr)[0] < 0x0020) {
    *str = g_strdup("");
    return;
  }

  mode = im_canna_get_num_of_canna_mode(cn);

  if (!(mode == CANNA_MODE_AlphaMode ||
	mode == CANNA_MODE_EmptyMode ||
	mode == CANNA_MODE_YomiMode ||
	mode == CANNA_MODE_AdjustBunsetsuMode)) {
    *str = g_strdup("");
    return;    
  }

  eucstr = g_strndup(cn->ks.echoStr, cn->kslength);

  *str = euc2utf8(eucstr);
  g_free(eucstr);

  if(*str == NULL || strlen(*str) == 0) return;

  if (attrs) {
    PangoAttribute *attr;
    attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
    attr->start_index = 0;
    attr->end_index = strlen(*str) ;
    pango_attr_list_insert(*attrs, attr);

    if (cn->ks.revLen > 0) {
      attr = pango_attr_background_new(0, 0, 0);
      attr->start_index = index_mb2utf8(cn->ks.echoStr, cn->ks.revPos);
      attr->end_index = index_mb2utf8(cn->ks.echoStr,
				      cn->ks.revPos+cn->ks.revLen);
      pango_attr_list_insert(*attrs, attr);

      attr = pango_attr_foreground_new(0xffff, 0xffff, 0xffff);
      attr->start_index = index_mb2utf8(cn->ks.echoStr, cn->ks.revPos);
      attr->end_index = index_mb2utf8(cn->ks.echoStr,
				      cn->ks.revPos+cn->ks.revLen);
      pango_attr_list_insert(*attrs, attr);
    }
  }

  if (cursor_pos != NULL) {
    int charpos = 0, eucpos = 0, curpos = 0;

    curpos = cn->ks.revPos;
    if(cn->ks.revPos != 0 && cn->ks.revLen > 0)
      curpos += cn->ks.revLen;
    
    while (curpos > eucpos) {
      unsigned char c = *(cn->ks.echoStr + eucpos);
	
      if (c < 0x80)
	eucpos += 1; // EUC G0 (ASCII) == GL
      else if (c == 0x8E)
	eucpos += 2; // EUC G2 (Half Width Kana) == SS2
      else if (c == 0x8F)
	eucpos += 3; // EUC G3 (JIS 3-4 level Kanji) == SS3
      else
	eucpos += 2; // EUC G1 (JIS 1-2 level Kanji) == GR
      
      charpos++;
    }
    
    *cursor_pos = charpos;
  }
}

static void
im_canna_set_client_window(GtkIMContext* context, GdkWindow *win) {
  IMContextCanna* cn = IM_CONTEXT_CANNA(context);

  cn->client_window = win;
}

static void
im_canna_focus_in (GtkIMContext* context) {
  IMContextCanna* cn = IM_CONTEXT_CANNA(context);

  focused_context = context;

  if (cn->ja_input_mode == TRUE) {
    im_canna_force_change_mode(cn, CANNA_MODE_HenkanMode);
    memset(cn->workbuf, 0, BUFSIZ);
    memset(cn->kakutei_buf, 0, BUFSIZ);
    g_signal_emit_by_name(cn, "preedit_changed");
  }

  im_canna_update_modewin(cn);
  
  if (cn->modebuf_utf8 != NULL)
    gtk_widget_show(cn->modewin);
}

static void
im_canna_focus_out (GtkIMContext* context) {
  IMContextCanna* cn = IM_CONTEXT_CANNA(context);

  focused_context = NULL;

  gtk_widget_hide(cn->candwin);
  gtk_widget_hide(cn->modewin);
}

static void
im_canna_set_cursor_location (GtkIMContext *context, GdkRectangle *area)
{
  IMContextCanna *cn = IM_CONTEXT_CANNA(context);
  GdkRectangle screen_area;
  GdkMonitor *current_monitor;
  gint x, y;

  if ( cn->client_window == NULL )
    return;

  handle_gline(cn);
  im_canna_update_candwin(cn);
  gdk_window_get_origin(cn->client_window, &x, &y);
  current_monitor = gdk_display_get_primary_monitor(gdk_display_get_default());
  gdk_monitor_get_workarea(current_monitor, &screen_area);

  cn->candwin_area.x = x + area->x - 128;
  cn->candwin_area.y = y + area->y + area->height;
  
  if (cn->candwin_area.x < 0)
    cn->candwin_area.x = 0;
  if (cn->candwin_area.x + cn->candwin_area.width >= screen_area.width)
    cn->candwin_area.x = screen_area.width - cn->candwin_area.width;
  
  if (cn->candwin_area.y + cn->candwin_area.height > screen_area.height)
    cn->candwin_area.y = screen_area.height - cn->candwin_area.height;
  
  gtk_window_move(GTK_WINDOW(cn->candwin),
		  cn->candwin_area.x,
		  cn->candwin_area.y);
}

/*
static gboolean
return_false(GtkIMContext* context, GdkEventKey* key) {
  return FALSE;
}
*/

/* im_canna_reset() commit all preedit string and clear the buffers.
   It should not reset input mode.

   Applications need to call gtk_im_context_reset() when they get
   focus out event. And when each editable widget losed focus, apps
   need to call gtk_im_context_reset() too to commit the string.
 */
static void
im_canna_reset(GtkIMContext* context) {
  IMContextCanna* cn = (IMContextCanna*)context;

  if( cn->kslength ) {
    gchar* utf8 = NULL;
    memset(cn->workbuf, 0, BUFSIZ);
    strncpy(cn->workbuf, cn->ks.echoStr, cn->kslength);
    utf8 = euc2utf8(cn->workbuf);
    /*g_signal_emit_by_name(cn, "commit", utf8);*/
    cn->kslength = 0;
    g_free(utf8);
  }

  if(cn->commit_str != NULL) {
    g_free(cn->commit_str);
    cn->commit_str = NULL;
  }

  memset(cn->workbuf, 0, BUFSIZ);
  memset(cn->kakutei_buf, 0, BUFSIZ);
  g_signal_emit_by_name(cn, "preedit_changed");
}

/* im_canna_kakutei() just do kakutei, commit and flush.
 * This works for generic canna usage, but doesn't for furigana support.
 */
static void
im_canna_kakutei(IMContextCanna* cn) {
  jrKanjiStatusWithValue ksv;
  guchar buf[BUFSIZ];
  int len = 0;
  gchar* utf8 = NULL;

  ksv.ks = &cn->ks;
  ksv.buffer = buf;
  ksv.bytes_buffer = BUFSIZ;

  len = jrKanjiControl(cn->canna_context, KC_KAKUTEI, (void*)&ksv);
  utf8 = euc2utf8(buf);

  g_free(cn->commit_str);
  cn->commit_str = g_strdup(utf8);
  
  memset(cn->workbuf, 0, BUFSIZ);
  memset(cn->kakutei_buf, 0, BUFSIZ);
  g_signal_emit_by_name(cn, "preedit_changed");

  g_free(utf8);
}

static void im_canna_kill(IMContextCanna* cn) {
  jrKanjiStatusWithValue ksv;
  guchar buf[BUFSIZ];
  int len = 0;
  gchar* utf8 = NULL;
    
  ksv.ks = &cn->ks;
  ksv.buffer = buf;
  ksv.bytes_buffer = BUFSIZ;
    
  len = jrKanjiControl(cn->canna_context, KC_KILL, (void*)&ksv);
  utf8 = euc2utf8(buf);
  
  memset(cn->workbuf, 0, BUFSIZ);
  memset(cn->kakutei_buf, 0, BUFSIZ);
  g_signal_emit_by_name(cn, "preedit_changed");

  jrKanjiControl(cn->canna_context, KC_INITIALIZE, 0);

  g_free(utf8);
}
