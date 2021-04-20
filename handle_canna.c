#include <canna/jrkanji.h>
#include <glib.h>

#include "im-canna-intl.h"
#include "enc.h"
#include "imsss.h"

/* [ ¤¢ ] */
#define CANNA_MODESTR_NORMAL "\x5b\x20\xa4\xa2\x20\x5d"
/* [³ÈÄ¥] */
#define CANNA_MODESTR_EXTENDED "\x5b\xb3\xc8\xc4\xa5\x5d"

#include "handle_canna.h"

int im_canna_get_num_of_canna_mode(IMContextCanna* cn);
void im_canna_force_change_mode(IMContextCanna* cn, int mode);

void handle_modebuf (IMContextCanna* cn) {
  int len = 0;
  gchar* modebuf = NULL;
  gboolean imsss_ret;
  /*  
  if( cn->ja_input_mode == FALSE) {
    imsss_set_status(IMSSS_STATUS_NONE);
    return;
  } else {
    len = jrKanjiControl(cn->canna_context, KC_QUERYMAXMODESTR, 0);
    modebuf = g_new0(gchar, len+1);
    jrKanjiControl(cn->canna_context, KC_QUERYMODE, modebuf);
    
    if( !strcmp(modebuf, CANNA_MODESTR_NORMAL) ) {
      imsss_ret = imsss_set_status(IMSSS_STATUS_KANA);
    } else {
      imsss_ret = imsss_set_status(IMSSS_STATUS_KANJI);
    }
    if( imsss_ret ) {
      g_free(modebuf);
      return;
    }
  }
  */
  len = jrKanjiControl(cn->canna_context, KC_QUERYMAXMODESTR, 0);
  modebuf = g_new0(gchar, len+1);
  jrKanjiControl(cn->canna_context, KC_QUERYMODE, modebuf);
  
  if(cn->modebuf_utf8 != NULL)
    g_free(cn->modebuf_utf8);
  
  if(cn->ja_input_mode == TRUE)
    cn->modebuf_utf8 = g_strdup(euc2utf8(modebuf));
  else
    cn->modebuf_utf8 = g_strdup("");

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

  cn->kslength = 0;

  ksv.ks = &(cn->ks);
  ksv.val = mode;
  ksv.buffer = cn->workbuf;
  ksv.bytes_buffer = BUFSIZ;
  jrKanjiControl(cn->canna_context, KC_CHANGEMODE, (void*)&ksv);
  cn->ja_input_mode == TRUE;
}

