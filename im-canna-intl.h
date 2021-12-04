#ifndef _IM_CANNA_INTL_H_
#define _IM_CANNA_INTL_H_

#define ENABLE_NLS 1

#ifdef ENABLE_NLS
#include<libintl.h>
#define _(String) dgettext(GETTEXT_PACKAGE,String)
#define P_(String) dgettext(GETTEXT_PACKAGE "-properties",String)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#else /* NLS is disabled */
#define _(String) (String)
#define P_(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain) 
#endif

#include "config.h"

#include <canna/jrkanji.h>

#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>

typedef struct _IMContextCanna {
  GtkIMContext parent;
  guchar* workbuf;
  guchar* kakutei_buf;
  jrKanjiStatus ks;
  gint canna_context; /* Cast from pointer - FIXME */
  int cand_stat;
  GtkWindow* candwin;
  GtkWindow* candlabel;
  PangoLayout* layout;
  GdkWindow* client_window;

  gboolean ja_input_mode; /* JAPANESE input mode or not */
  gboolean focus_in_candwin_show; /* the candidate window should be up at focus in */

  GtkWidget* modewin;
  GtkWidget* modelabel;
  GdkRectangle candwin_area;

  gchar *commit_str;
  gchar* modebuf;
  gchar* modebuf_utf8;

  gchar *preedit_string;
  gint preedit_revPos;
  gint preedit_revLen;
  gint preedit_length;

  gint preedit_prevlen;

  gchar* gline_message;
  gint gline_length;
  gint gline_revPos, gline_revLen;

  int initinal_canna_mode;

  uint prevkeytime;
  gchar *init_mode_string;
} IMContextCanna;

typedef struct _IMContextCannaClass {
  GtkIMContextClass parent_class;
} IMContextCannaClass;

extern GType type_canna;

#define IM_CONTEXT_TYPE_CANNA (type_canna)
#define IM_CONTEXT_CANNA(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), type_canna, IMContextCanna))
#define IM_CONTEXT_CANNA_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), IM_CONTEXT_TYPE_CANNA, IMContextCannaClass))

#define CANDWIN_FONT "monospace 12"

#endif
