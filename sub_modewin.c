#include <string.h>
#include <canna/jrkanji.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkkeysyms-compat.h>

#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>

#include "im-canna-intl.h"
#include "enc.h"
#include "handle_canna.h"

void im_canna_create_modewin(IMContextCanna* cn);
void im_canna_update_modewin(IMContextCanna* cn);
void im_canna_move_modewin(IMContextCanna* cn);

void im_canna_create_modewin(IMContextCanna* cn) {
  cn->modewin = gtk_window_new(GTK_WINDOW_POPUP);
  cn->modelabel = gtk_label_new("");
  gtk_container_add(GTK_CONTAINER(cn->modewin), cn->modelabel);
  
  im_canna_update_modewin(cn);
}

void im_canna_update_modewin(IMContextCanna* cn) {
  PangoAttrList* attrs;
  PangoAttribute* attr;

  /*
     Hide when the candidate window is up.
     The mode window should play the secondary role while the candidate
     window plays the leading role, because candidate window has more
     important info for user input.
   */
  /*
  if(gtk_widget_get_visible(cn->candwin)) {
    gtk_widget_hide(cn->modewin);
  }
  */

  handle_modebuf(cn);

  if(cn->modebuf_utf8 == NULL)
    cn->modebuf_utf8 = g_strdup("");
  
  gtk_label_set_label(GTK_LABEL(cn->modelabel),
		      cn->modebuf_utf8);

  /* Set Mode Window Background Color to Blue */
  attrs = gtk_label_get_attributes(GTK_LABEL(cn->modelabel));

  attrs = pango_attr_list_new();
  attr = pango_attr_background_new(0xDB00, 0xE900, 0xFF00);
  attr->start_index = 0;
  attr->end_index = strlen(cn->modebuf_utf8);
  pango_attr_list_insert(attrs, attr);
  
  gtk_label_set_attributes(GTK_LABEL(cn->modelabel), attrs);
  /* - Coloring Done - */
  if( attrs ) {
    pango_attr_list_unref(attrs);
  }

  gtk_window_resize (GTK_WINDOW(cn->modewin), 1, 1);
  im_canna_move_modewin(cn);
  gtk_widget_show(cn->modelabel);
}

void im_canna_move_modewin(IMContextCanna* cn)
{
  GdkScreen* screen;
  gint x, y, width, height, scr_width, scr_height;

  if ( cn->client_window == NULL )
    return;

  screen = gdk_drawable_get_screen (cn->client_window);
  scr_width  = gdk_screen_get_width  (screen);
  scr_height = gdk_screen_get_height (screen);

  x = 0;
  y = scr_height;
  
  gtk_window_get_size(GTK_WINDOW(cn->modewin), &width, &height);

  if (x + width >= scr_width)
    x = width - scr_width;
  
  if (y + height > scr_height)
    y = scr_height - height;

  gtk_window_move(GTK_WINDOW(cn->modewin), x, y);
}
