#include <string.h>
#include <canna/jrkanji.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkkeysyms-compat.h>

#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>

#include "im-canna-intl.h"
#include "enc.h"
#include "handle_canna.h"

static void im_canna_show_candwin(IMContextCanna* cn, gchar* candstr);
void im_canna_update_candwin(IMContextCanna* cn);

static void
scroll_cb(GtkWidget* widget, GdkEventScroll* event, IMContextCanna* cn) {
  switch(event->direction) {
  case GDK_SCROLL_UP:
    jrKanjiString(cn->canna_context, 0x02, cn->kakutei_buf,
		  IM_CANNA3_BUFSIZ, &cn->ks);
    break;
  case GDK_SCROLL_DOWN:
    jrKanjiString(cn->canna_context, 0x06, cn->kakutei_buf,
		  IM_CANNA3_BUFSIZ, &cn->ks);
    break;
  default:
    break;
  }
}

void im_canna_create_candwin(IMContextCanna* cn) {
  cn->candwin = gtk_window_new(GTK_WINDOW_POPUP);  
  cn->candlabel = gtk_label_new("");
  gtk_container_add(GTK_CONTAINER(cn->candwin), cn->candlabel);
  cn->candwin_area.x = cn->candwin_area.y = 0;
  cn->candwin_area.width = cn->candwin_area.height = 0;
  gtk_window_resize (GTK_WINDOW(cn->candwin), 1, 1);
  
  gtk_widget_add_events(cn->candwin, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(cn->candwin, "scroll_event", G_CALLBACK(scroll_cb), cn);

  cn->layout  = gtk_widget_create_pango_layout(cn->candwin, "");
  
  gtk_window_set_accept_focus(cn->candwin, FALSE);
}

static void im_canna_show_candwin(IMContextCanna* cn, gchar* candstr) {
  gchar* labeltext = euc2utf8(candstr);
  PangoAttrList* attrlist = pango_attr_list_new();
  PangoAttribute* attr;
  PangoFontDescription* df;

  if(*candstr == '\0')
    return;
  
  gtk_label_set_text(GTK_LABEL(cn->candlabel), labeltext);

  df = pango_font_description_from_string(CANDWIN_FONT);
  attr = pango_attr_font_desc_new(df);
  attr->start_index = 0;
  attr->end_index = strlen(labeltext);
  pango_attr_list_insert(attrlist, attr);
  
  attr = pango_attr_background_new(0, 0, 0);
  attr->start_index = eucpos2utf8pos(candstr, cn->gline_revPos);
  attr->end_index = eucpos2utf8pos(candstr, cn->gline_revPos + cn->gline_revLen);
  pango_attr_list_insert(attrlist, attr);

  attr = pango_attr_foreground_new(0xffff, 0xffff, 0xffff);
  attr->start_index = eucpos2utf8pos(candstr, cn->gline_revPos);
  attr->end_index = eucpos2utf8pos(candstr, cn->gline_revPos + cn->gline_revLen);
  pango_attr_list_insert(attrlist, attr);

  gtk_label_set_attributes(GTK_LABEL(cn->candlabel), attrlist);
  
  pango_font_description_free (df);  
  g_free(labeltext);
  
  gtk_window_get_size(GTK_WINDOW(cn->candwin),
		      &cn->candwin_area.width, &cn->candwin_area.height);
}

void im_canna_update_candwin(IMContextCanna* cn)
{
  if( cn->gline_length == 0 ) {
    gtk_widget_hide(cn->candwin);
  } else {
    im_canna_show_candwin(cn, cn->gline_message);
    gtk_widget_show_all(cn->candwin);
  }
}

