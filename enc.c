#include <string.h>

#include <glib.h>
#include <glib/gprintf.h>

#include "enc.h"

guchar* euc2utf8(const guchar* str);
gint eucpos2utf8pos(guchar* euc, gint eucpos);

gint im_canna_get_utf8_len_from_euc (guchar *text_euc);
gint im_canna_get_utf8_pos_from_euc_pos (guchar *text_euc, gint pos);

gboolean im_canna_is_euc_char(guchar* p);
gboolean im_canna_is_euc_hiragana(guchar* text);
gboolean im_canna_is_euc_kanji(guchar* text);

gint im_canna_get_pre_hiragana_len (guchar* text);
gint im_canna_get_post_hiragana_len (guchar* text);
gboolean im_canna_is_euc_hiragana_char(guchar* p);

int index_mb2utf8(gchar* mbstr, int revPos);

guchar *
euc2utf8 (const guchar *str)
{
  GError *error = NULL;
  gchar *result = NULL;
  gsize bytes_read = 0, bytes_written = 0;

  result = g_convert (str, -1,
		      "UTF-8", "EUC-JP",
		      &bytes_read, &bytes_written, &error);
  if (error) {
    /* Canna outputs ideograph codes of where EUC-JP is not assigned. */
    gchar* eucprefix = (bytes_read == 0) ? g_strdup("") : g_strndup(str, bytes_read);
    gchar* prefix = euc2utf8(eucprefix);
    /* 2 bytes skip */
    gchar* converted = euc2utf8(str + bytes_read + 2);
    if( result )
      g_free(result);
    /* Replace to a full-width space */
    result = g_strconcat(prefix, "\xe3\x80\x80", converted, NULL);
    g_free(prefix);
    g_free(eucprefix);
    g_free(converted);
  }

#if 0
  if (error)
    {
      g_warning ("Error converting text from IM to UTF-8: %s\n", error->message);
      g_print("Error: bytes_read: %d\n", bytes_read);
      g_print("%02X %02X\n", str[bytes_read], str[bytes_read+1]);
      g_print("Error: bytes_written: %d\n", bytes_written);
      g_error_free (error);
    }
#endif

  return result;
}

gint eucpos2utf8pos(guchar* euc, gint eucpos) {
  guchar* tmpeuc = NULL;
  gchar* utf8str = NULL;
  gint result;

  if ( eucpos <= 0  || euc == NULL || *euc == '\0' )
    return 0;

  tmpeuc = g_strndup(euc, eucpos);
  utf8str = euc2utf8(tmpeuc);

  result = utf8str ? strlen(utf8str) : 0 ;

  if( utf8str )
    g_free(utf8str);
  g_free(tmpeuc);

  return result;
}

/***/
gint im_canna_get_utf8_len_from_euc (guchar *text_euc)
{
  gchar *text_utf8;
  gint utf8_len;

  text_utf8 = euc2utf8 (text_euc);

  utf8_len = g_utf8_strlen (text_utf8, -1);

  g_free (text_utf8);


  return utf8_len;
}

gint im_canna_get_utf8_pos_from_euc_pos (guchar *text_euc,
				    gint    pos)
{
  gchar *tmp_euc;
  gchar *tmp_utf8;
  gint utf8_pos;

  tmp_euc = g_strndup (text_euc, pos);
  tmp_utf8 = euc2utf8 (tmp_euc);

  utf8_pos = g_utf8_strlen (tmp_utf8, -1);

  g_free (tmp_euc);
  g_free (tmp_utf8);


  return utf8_pos;
}

/***/
gboolean im_canna_is_euc_char(guchar* p) {
  if ((p != NULL)      &&
      (*(p+0) != '\0') &&
      (*(p+1) != '\0') &&
      (*(p+0) & 0x80) &&
      (*(p+1) & 0x80))
    {
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

/* Checking Hiragana in EUC-JP
 *  212b, 212c, 213c, 2421-2473 (JIS)
 *  A1AB, A1AC, A1BC, A4A1-A4F3 (EUC)
 */
gboolean im_canna_is_euc_hiragana_char(guchar* p) {
  if ((p == NULL)      ||
      (*(p+0) == '\0') ||
      (*(p+1) == '\0'))
    {
      return FALSE;
    }
  else
    {
      guint16 mb;

      mb = ((*(p+0) << 8) |
	    (*(p+1) << 0));

      if ((mb == 0xA1AB) ||
	  (mb == 0xA1AC) ||
	  (mb == 0xA1BC) ||
	  ((mb >= 0xA4A1) && (mb <= 0xA4F3)))
	{
	  return TRUE;
	}
      else
	{
	  return FALSE;
	}
    }
}

gboolean im_canna_is_euc_hiragana(guchar* text) {
  guchar *p = text;

  if( text == NULL || *text == '\0' )
    return FALSE;

  if(strlen(p) < 2)
    return FALSE;

  while(strlen(p) >= 2) {
    if (im_canna_is_euc_hiragana_char (p) == FALSE) {
      return FALSE;
    }

    p += 2;

    if( strlen(p) == 1 )
      return FALSE;

    if( strlen(p) == 0 )
      return TRUE;
  }

  return FALSE;
}

/* Checking Kanji in EUC-JP
 *  (rough area for faster calc, and jisx0208 only)
 *  3021-7424 (JIS)
 *  B0A1-F4A4 (EUC)
 */
gboolean im_canna_is_euc_kanji(guchar* text) {
  guchar *p = text;

  if( text == NULL || *text == '\0' )
    return FALSE;

  if(strlen(p) < 2)
    return FALSE;

  while(strlen(p) >= 2) {
    guint16 mb = (p[0]<<8) + p[1];

    if( !(mb >= 0xB0A1 && mb <= 0xF4A4) ) {
      return FALSE;
    }

    p += 2;

    if( strlen(p) == 1 )
      return FALSE;

    if( strlen(p) == 0 )
      return TRUE;
  }

  return FALSE;
}

/***/
gint im_canna_get_pre_hiragana_len (guchar* text)
{
  gint pre_hiragana_len;
  guchar *p;
  int text_len;
  int i;


  pre_hiragana_len = 0;

  text_len = strlen (text);

  i = 0;
  while (i < text_len)
    {
      if ((text_len - i) < 2)
	{
	  break;
	}

      p = (text + i);

      if (im_canna_is_euc_hiragana_char (p) == TRUE)
	{
	  i += 2;

	  pre_hiragana_len += 2;
	}
      else
	{
	  break;
	}
    }


  return pre_hiragana_len;
}

gint im_canna_get_post_hiragana_len (guchar* text)
{
  gint post_hiragana_len;
  guchar *p;
  guchar *tmp;
  int text_len;
  int i;


  tmp = NULL;
  text_len = strlen (text);

  i = 0;
  while (i < text_len)
    {
      if ((text_len - i) < 2)
	{
	  tmp = NULL;
	  break;
	}

      p = (text + i);

      if (im_canna_is_euc_hiragana_char (p) == TRUE)
	{
	  if (tmp == NULL)
	    {
	      tmp = p;
	    }

	  i += 2;
	}
      else
	{
	  tmp = NULL;

	  if (im_canna_is_euc_char (p) == TRUE)
	    {
	      i += 2;
	    }
	  else
	    {
	      i += 1;
	    }
	}
    }

  if (tmp != NULL)
    {
      post_hiragana_len = ((text + text_len) - tmp);
    }
  else
    {
      post_hiragana_len = 0;
    }


  return post_hiragana_len;
}

/*
 * index_mb2utf8:
 *     return utf8 index from multibyte string and multibyte index
 */
int index_mb2utf8(gchar* mbstr, int revPos) {
  gchar* u8str;
  gchar* mbbuf;
  gint ret = 0;

  if( mbstr == NULL || *mbstr == '\0' ) {
    return 0;
  }

  if( revPos <= 0 || strlen(mbstr) < revPos ) {
    return 0;
  }

  mbbuf = g_strndup(mbstr, revPos);
  u8str = euc2utf8 (mbbuf);
  g_free(mbbuf);

  ret = strlen(u8str);
  g_free(u8str);

  return ret;
}
