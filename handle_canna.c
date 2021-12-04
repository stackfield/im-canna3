#include <canna/jrkanji.h>
#include <glib.h>

#include "im-canna-intl.h"
#include "enc.h"

#include "handle_canna.h"

int im_canna_get_num_of_canna_mode(IMContextCanna* cn);
void im_canna_force_change_mode(IMContextCanna* cn, int mode);

void handle_modebuf (IMContextCanna* cn) {
  int len = 0;
  gchar* modebuf = NULL;

  im_canna_get_string_of_canna_mode(cn, &modebuf);

  if(cn->modebuf_utf8 != NULL)
    g_free(cn->modebuf_utf8);
  
  cn->modebuf_utf8 = euc2utf8(modebuf);

  g_free(modebuf);
}

void handle_gline (IMContextCanna* cn)
{
  /* printf("GLineInfo ks.mode: %s\n", cn->ks.mode); */
  /* printf("GLineInfo ks.gline.line: %sA\n", cn->ks.gline.line); */
  /* printf("GLineInfo ks.gline.length: %d\n", cn->ks.gline.length); */

  if (!(cn->ks.info & KanjiGLineInfo))
    return;
  
  if (cn->gline_message != NULL)
    g_free(cn->gline_message);

  cn->gline_message = g_strdup(cn->ks.gline.line);
  cn->gline_length = cn->ks.gline.length;
  cn->gline_revPos = cn->ks.gline.revPos;
  cn->gline_revLen = cn->ks.gline.revLen;
}

void clear_gline (IMContextCanna* cn)
{
  /* printf("GLineInfo ks.mode: %s\n", cn->ks.mode); */
  /* printf("GLineInfo ks.gline.line: %sA\n", cn->ks.gline.line); */
  /* printf("GLineInfo ks.gline.length: %d\n", cn->ks.gline.length); */

  if (cn->gline_message != NULL)
    g_free(cn->gline_message);

  cn->gline_message = NULL;
  cn->gline_length = cn->ks.gline.revPos = cn->gline_revLen = 0;
}

void handle_preedit (IMContextCanna *cn)
{
  if (cn->ks.length < 0)
    return;

  if (cn->ks.length == 0) {
    int prevlen = cn->preedit_length;
    clear_preedit(cn);
    cn->preedit_prevlen = prevlen;
    return;
  }

  if (cn->preedit_string != NULL)
    g_free(cn->preedit_string);

  cn->preedit_prevlen = cn->preedit_length;
  cn->preedit_length = cn->ks.length;
  cn->preedit_revPos = cn->ks.revPos;
  cn->preedit_revLen = cn->ks.revLen;

  if(cn->ks.echoStr != NULL)
    cn->preedit_string = g_strdup(cn->ks.echoStr);
}

void im_canna_init_preedit (IMContextCanna *cn)
{
  clear_preedit(cn);
  cn->preedit_prevlen = 0;
}

void clear_preedit (IMContextCanna *cn)
{
  if (cn->preedit_string != NULL) 
    g_free(cn->preedit_string);

  cn->preedit_string = NULL;
  cn->preedit_length = 0;
  cn->preedit_revPos = cn->preedit_revLen = 0;
}

void
im_canna_get_string_of_canna_mode (IMContextCanna* cn, gchar **ret_string) {
  int len = 0;
  gchar* modebuf = NULL;

  len = jrKanjiControl(cn->canna_context, KC_QUERYMAXMODESTR, 0);
  modebuf = g_new0(gchar, len+1);
  jrKanjiControl(cn->canna_context, KC_QUERYMODE, modebuf);

  *ret_string = modebuf;
}

int
im_canna_get_num_of_canna_mode(IMContextCanna* cn) {
  char mode[1];
  int ret = - 1;
  
  jrKanjiControl(cn->canna_context, KC_SETMODEINFOSTYLE, 1);
  jrKanjiControl(cn->canna_context, KC_QUERYMODE, mode);
  ret = (unsigned int) (mode[0] - 0x40);
  jrKanjiControl(cn->canna_context, KC_SETMODEINFOSTYLE, 0);

  return ret;
}

void im_canna_force_change_mode(IMContextCanna* cn, int mode)
{
  jrKanjiStatusWithValue ksv;

  ksv.ks = &(cn->ks);
  ksv.val = mode;
  ksv.buffer = cn->workbuf;
  ksv.bytes_buffer = BUFSIZ;
  jrKanjiControl(cn->canna_context, KC_CHANGEMODE, (void*)&ksv);
}

void im_canna_kill_unspecified_string(IMContextCanna* cn)
{
  jrKanjiStatusWithValue ksv;

  ksv.ks = &(cn->ks);
  ksv.buffer = cn->workbuf;
  ksv.bytes_buffer = BUFSIZ;
  jrKanjiControl(cn->canna_context, KC_KILL, (void*)&ksv);

  if(im_canna_get_num_of_canna_mode(cn) == CANNA_MODE_AlphaMode)
    im_canna_force_change_mode(cn, cn->initinal_canna_mode);
}

int im_canna_connect_server(IMContextCanna* cn)
{
  jrKanjiControl(cn->canna_context, KC_INITIALIZE, 0);
  jrKanjiControl(cn->canna_context, KC_SETWIDTH, 62);

  return 0;
}

int im_canna_disconnect_server(IMContextCanna* cn)
{
  jrKanjiControl(cn->canna_context, KC_FINALIZE, 0);

  return 0;
}
