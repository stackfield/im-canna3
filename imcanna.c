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

#ifdef BUFSIZ
#undef BUFSIZ
#define BUFSIZ 1024
#endif

#define KEY_TIMEOUT (1 * 1000) /* sec * millsec */

GType type_canna = 0;

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

/* sub_function_mode.c */
extern gboolean im_canna_function_mode (GtkIMContext *context, GdkEventKey *key);

/* sub_candwin.c */
extern void im_canna_create_candwin(IMContextCanna* cn);
extern void im_canna_update_candwin(IMContextCanna* cn);

/* sub_modewin.c */
extern void im_canna_create_modewin(IMContextCanna* cn);
extern void im_canna_update_modewin(IMContextCanna* cn);
extern void im_canna_move_modewin(IMContextCanna* cn);
extern void im_canna_update_modewin(IMContextCanna* cn);

/* sub_direct_mode.c */
extern gboolean
im_canna_enter_direct_mode(GtkIMContext *context, GdkEventKey *key);

/* sub_japanese_mode.c */
extern gboolean
im_canna_enter_japanese_mode(GtkIMContext *context, GdkEventKey *key);
extern void routine_for_preedit_signal(GtkIMContext* context);

/*** For KeySnooper ***/
#ifdef USE_KEYSNOOPER
static guint snooper_id = 0;
static GtkIMContext* focused_context = NULL;
static guint focus_id = 0;

static gboolean
snooper_func(GtkWidget* widget, GdkEventKey* event, gpointer data) {
  if( focused_context )
    return im_canna_filter_keypress(focused_context, event);

  return FALSE;
}

static gboolean
return_false(GtkIMContext* context, GdkEventKey* key) {
  return FALSE;
}
#endif

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
/* Use snooper */
#ifdef USE_KEYSNOOPER
  im_context_class->filter_keypress = return_false;
#else
  im_context_class->filter_keypress = im_canna_filter_keypress;
#endif
  im_context_class->get_preedit_string = im_canna_get_preedit_string;
  im_context_class->focus_in = im_canna_focus_in;
  im_context_class->focus_out = im_canna_focus_out;
  im_context_class->reset = im_canna_reset;
  im_context_class->set_cursor_location = im_canna_set_cursor_location;

  object_class->finalize = im_canna_finalize;
}

static void
im_canna_enable_ja_input_mode_with_mode(GtkIMContext *context, int mode) {
  IMContextCanna *cn = IM_CONTEXT_CANNA(context);
  cn->ja_input_mode = TRUE;
  
  clear_preedit(cn);
  clear_gline(cn);
  im_canna_connect_server(cn);
  im_canna_force_change_mode(cn, mode);
  im_canna_update_modewin(cn);
}

static void im_canna_enable_ja_input_mode(GtkIMContext *context) {
  IMContextCanna *cn = IM_CONTEXT_CANNA(context);
  im_canna_enable_ja_input_mode_with_mode (context,
					   cn->initinal_canna_mode);
}

static void im_canna_disable_ja_input_mode(GtkIMContext *context) {
  IMContextCanna *cn = IM_CONTEXT_CANNA(context);
  cn->ja_input_mode = FALSE;
  clear_preedit(cn);
  clear_gline(cn);
  cn->prevkeytime = 0;
  im_canna_disconnect_server(cn);
}

static void
im_canna_init (GtkIMContext *im_context)
{
  IMContextCanna *cn = IM_CONTEXT_CANNA(im_context);

  PangoAttrList* attrs;
  PangoAttribute* attr;

  cn->canna_context = (gint) im_context;
  cn->cand_stat = 0;
  cn->workbuf = g_new0(guchar, BUFSIZ);
  cn->kakutei_buf = g_new0(guchar, BUFSIZ);

  cn->modebuf_utf8 = NULL;

  cn->client_window = NULL;
  cn->focus_in_candwin_show = FALSE;
  cn->ja_input_mode = FALSE;
  cn->commit_str = NULL;

  cn->prevkeytime = 0;
  cn->need_canna_reset = FALSE;

  clear_gline(cn);
  im_canna_init_preedit(cn);

  im_canna_connect_server(cn);
  cn->initinal_canna_mode = im_canna_get_num_of_canna_mode(cn);

  if ( cn->initinal_canna_mode == CANNA_MODE_AlphaMode ) {
    cn->initinal_canna_mode = CANNA_MODE_HenkanMode;
  }

  im_canna_get_string_of_canna_mode(cn, &cn->init_mode_string);

  im_canna_create_modewin(cn);
  im_canna_create_candwin(cn);

  im_canna_disconnect_server(cn);

#ifdef USE_KEYSNOOPER
  snooper_id = gtk_key_snooper_install((GtkKeySnoopFunc)snooper_func, NULL);
#endif

}

static void
im_canna_finalize(GObject *obj) {
  IMContextCanna* cn = IM_CONTEXT_CANNA(obj);

  im_canna_disconnect_server(cn);

  g_free(cn->modebuf);
  g_free(cn->init_mode_string);
  g_free(cn->gline_message);
  g_free(cn->modebuf_utf8);
  g_free(cn->commit_str);
  g_free(cn->workbuf);
  g_free(cn->kakutei_buf);
  gtk_widget_destroy(GTK_WIDGET(cn->candlabel));
  gtk_widget_destroy(GTK_WIDGET(cn->candwin));
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

static gboolean
im_canna_is_need_reconnect_for_time(GtkIMContext *context, GdkEventKey *key)
{
  IMContextCanna *cn = IM_CONTEXT_CANNA(context);

  if (key->time - cn->prevkeytime <= KEY_TIMEOUT || cn->prevkeytime != 0 )
    return FALSE;

  if (cn->preedit_length != 0 && cn->gline_length != 0 )
    return FALSE;

  if (strcmp(cn->init_mode_string, cn->modebuf != 0))
    return FALSE;

  return TRUE;
}

gboolean
im_canna_filter_keypress(GtkIMContext *context, GdkEventKey *key)
{
  IMContextCanna *cn = IM_CONTEXT_CANNA(context);
  gboolean ret = FALSE;
  int mode = -1;

  if( key->type != GDK_KEY_PRESS) {
    return FALSE;
  };

  /* Editable widget should pass mnemonic if ja-input-mode is on */
  g_object_set_data(G_OBJECT(context), "immodule-needs-mnemonic",
		    (gpointer)cn->ja_input_mode);

  if (im_canna_is_modechangekey(context, key)) {    
    if( cn->preedit_length > 0) {
      clear_preedit(cn);
      im_canna_kill_unspecified_string(cn);
      routine_for_preedit_signal(context);
    }
    
    if( cn->ja_input_mode == FALSE ) {
      im_canna_enable_ja_input_mode(context);
      gtk_widget_show_all(GTK_WIDGET(cn->modewin));
      cn->prevkeytime = key->time;
    } else {
      im_canna_disable_ja_input_mode(context);
      im_canna_update_modewin(cn);
      gtk_widget_hide(GTK_WIDGET(cn->candwin));
      gtk_widget_hide(GTK_WIDGET(cn->modewin));
    }

    return TRUE;
  } else {
    if( cn->ja_input_mode == TRUE ) {
      if( cn->need_canna_reset == TRUE ||
	  im_canna_is_need_reconnect_for_time(context, key) ) {
	cn->need_canna_reset = FALSE;
	im_canna_disable_ja_input_mode(context);
	im_canna_enable_ja_input_mode(context);
	gtk_widget_show_all(cn->modewin);
      }
    }
  }
    
  if( cn->ja_input_mode == TRUE ) {
    if( im_canna_get_num_of_canna_mode(cn) == CANNA_MODE_AlphaMode ) {
      im_canna_disable_ja_input_mode(context);
      im_canna_enable_ja_input_mode(context);
      gtk_widget_show_all(cn->modewin);
    }
  }

  if( cn->ja_input_mode == FALSE ) {
    /*** direct mode ***/
    cn->prevkeytime = 0;
    return im_canna_enter_direct_mode(context, key);
  }

  /* Canna can't handle key with mod* key. */
#define GDK_MOD_MASK (GDK_MOD1_MASK | GDK_MOD2_MASK | GDK_MOD3_MASK \
		      | GDK_MOD4_MASK | GDK_MOD5_MASK)
  if (key->state & GDK_MOD_MASK)
      return FALSE;

  cn->prevkeytime = key->time;

  mode = im_canna_get_num_of_canna_mode(cn);
  
  if (mode >= CANNA_MODE_HexMode || mode == CANNA_MODE_KigoMode) {
    /*** Function mode ***/
    ret = im_canna_function_mode(context, key);
  } else {
    /*** Japanese mode ***/
    ret = im_canna_enter_japanese_mode(context, key);
  }

  /*** update cardwin & modewin ***/
  handle_gline(cn);

  if ((cn->gline_length > 0)) {
    im_canna_update_candwin(cn);
    gtk_widget_show(GTK_WIDGET(cn->candwin));
  } else {
    gtk_widget_hide(GTK_WIDGET(cn->candwin));
  }
    
  im_canna_update_modewin(cn);
  gtk_widget_show(cn->modewin);

  if( im_canna_get_num_of_canna_mode(cn) == CANNA_MODE_AlphaMode ) {
    gtk_widget_hide(GTK_WIDGET(cn->modewin));
    gtk_widget_hide(GTK_WIDGET(cn->candwin));
    im_canna_disable_ja_input_mode(context);
    return ret;
  }

  return ret;
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

  if (cn->preedit_length <= 0) {
    *str = g_strdup("");
    return;
  }

  if (cn->preedit_string == NULL || *cn->preedit_string == '\0') {
    *str = g_strdup("");
    return;
  }

  if (((guint16 *)cn->preedit_string)[0] < 0x0020) {
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

  eucstr = g_strndup(cn->preedit_string, cn->preedit_length);
  *str = euc2utf8(eucstr);
  g_free(eucstr);

  if(*str == NULL || strlen(*str) == 0) return;

  if (attrs) {
    PangoAttribute *attr;
    attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
    attr->start_index = 0;
    attr->end_index = strlen(*str) ;
    pango_attr_list_insert(*attrs, attr);

    if (cn->preedit_revLen > 0) {
      attr = pango_attr_background_new(0, 0, 0);
      attr->start_index = index_mb2utf8(cn->preedit_string, cn->preedit_revPos);
      attr->end_index = index_mb2utf8(cn->preedit_string,
				      cn->preedit_revPos+cn->preedit_revLen);
      pango_attr_list_insert(*attrs, attr);

      attr = pango_attr_foreground_new(0xffff, 0xffff, 0xffff);
      attr->start_index = index_mb2utf8(cn->preedit_string, cn->preedit_revPos);
      attr->end_index = index_mb2utf8(cn->preedit_string,
				      cn->preedit_revPos+cn->preedit_revLen);
      pango_attr_list_insert(*attrs, attr);
    }
  }

  if (cursor_pos != NULL) {
    int charpos = 0, eucpos = 0, curpos = 0;

    curpos = cn->preedit_revPos;
    if(cn->preedit_revPos != 0 && cn->preedit_revLen > 0)
      curpos += cn->preedit_revLen;
    
    while (curpos > eucpos) {
      unsigned char c = *(cn->preedit_string + eucpos);
	
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
#ifdef USE_KEYSNOOPER  
  focused_context = context;
#endif

  if (cn->ja_input_mode == TRUE) {
    im_canna_update_modewin(cn);
    gtk_widget_show(GTK_WIDGET(cn->modewin));
  }
}

static void
im_canna_focus_out (GtkIMContext* context) {
  IMContextCanna* cn = IM_CONTEXT_CANNA(context);

#ifdef USE_KEYSNOOPER  
  focused_context = NULL;
#endif

  if (cn->ja_input_mode == TRUE) {    
    if(cn->preedit_length > 0) {
      gchar* str = NULL;
      
      if(cn->commit_str != NULL) {
	g_free(cn->commit_str);
	cn->commit_str = NULL;
      }

      str = euc2utf8(cn->preedit_string);
      g_signal_emit_by_name(cn, "commit", str);
      g_free(str);

      clear_preedit(cn);
      routine_for_preedit_signal(context);
    }

    clear_gline(cn);
    cn->need_canna_reset = TRUE;

    gtk_widget_hide(GTK_WIDGET(cn->modewin));
    gtk_widget_hide(GTK_WIDGET(cn->candwin));
  }
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

/* im_canna_reset() commit all preedit string and clear the buffers.
   It should not reset input mode.

   Applications need to call gtk_im_context_reset() when they get
   focus out event. And when each editable widget losed focus, apps
   need to call gtk_im_context_reset() too to commit the string.
 */
static void
im_canna_reset(GtkIMContext* context) {
  IMContextCanna* cn = (IMContextCanna*)context;

  if (cn->ja_input_mode == TRUE) {
    if(cn->preedit_length > 0) {
      gchar* str = NULL;
      
      if(cn->commit_str != NULL) {
	g_free(cn->commit_str);
	cn->commit_str = NULL;
      }

      str = euc2utf8(cn->preedit_string);
      g_signal_emit_by_name(cn, "commit", str);
      g_free(str);

      clear_preedit(cn);
      routine_for_preedit_signal(context);
    }

    clear_gline(cn);
    cn->need_canna_reset = TRUE;

    gtk_widget_hide(GTK_WIDGET(cn->modewin));
    gtk_widget_hide(GTK_WIDGET(cn->candwin));
  }
}

