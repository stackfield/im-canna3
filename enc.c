#include <string.h>

#include <glib.h>
#include <glib/gprintf.h>

#include "enc.h"

guchar* euc2utf8(const guchar* str);
gint eucpos2utf8pos(guchar* euc, gint eucpos);
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
