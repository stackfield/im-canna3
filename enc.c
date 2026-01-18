#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "enc.h"

/*
 * euc2utf8:
 * Convert EUC-JP string to UTF-8.
 * Always returns newly allocated string or NULL.
 */
guchar *
euc2utf8 (const guchar *str)
{
    GError *error = NULL;
    guchar *result = NULL;
    gsize bytes_read = 0, bytes_written = 0;

    if (str == NULL || *str == '\0') {
        return g_strdup("");
    }

    result = g_convert((const gchar *)str, -1,
                       "UTF-8", "EUC-JP",
                       &bytes_read, &bytes_written, &error);

    if (error) {
        gchar *prefix = NULL;
        gchar *suffix = NULL;

        if (bytes_read > 0) {
            prefix = g_convert((const gchar *)str, bytes_read,
                               "UTF-8", "EUC-JP",
                               NULL, NULL, NULL);
        }

        if (str[bytes_read] != '\0') {
            suffix = g_convert((const gchar *)(str + bytes_read + 1), -1,
                               "UTF-8", "EUC-JP",
                               NULL, NULL, NULL);
        }

        g_clear_error(&error);
        g_free(result);

        result = g_strconcat(prefix ? prefix : "",
                              "\xE3\x80\x80", /* full-width space */
                              suffix ? suffix : "",
                              NULL);

        g_free(prefix);
        g_free(suffix);
    }

    return result;
}

/*
 * eucpos2utf8pos: 
 * Convert EUC byte position to UTF-8 byte position.
 */
gint
eucpos2utf8pos (guchar *euc, gint eucpos)
{
    gchar *utf8str;
    gchar *tmpeuc;
    gint result = 0;

    if (!euc || eucpos <= 0) {
        return 0;
    }

    tmpeuc = g_strndup((const gchar *)euc, eucpos);
    utf8str = (gchar *)euc2utf8((const guchar *)tmpeuc);

    if (utf8str) {
        result = strlen(utf8str);
        g_free(utf8str);
    }

    g_free(tmpeuc);
    return result;
}

/*
 * index_mb2utf8:
 * Return UTF-8 byte index from multibyte string and multibyte index.
 */
int
index_mb2utf8 (gchar *mbstr, int revPos)
{
    gchar *mbbuf;
    gchar *u8str;
    int ret = 0;

    if (!mbstr || revPos <= 0) {
        return 0;
    }

    if ((int)strlen(mbstr) < revPos) {
        return 0;
    }

    mbbuf = g_strndup(mbstr, revPos);
    u8str = (gchar *)euc2utf8((const guchar *)mbbuf);
    g_free(mbbuf);

    if (u8str) {
        ret = strlen(u8str);
        g_free(u8str);
    }

    return ret;
}
