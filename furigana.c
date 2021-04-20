#include "enc.h"
#include "furigana.h"
#include "im-canna-intl.h"
#include "util.h"

GSList* discard_strict_furigana(gchar* ptext, GSList* furigana_slist);
GSList* im_canna_get_furigana(IMContextCanna* cn);

static int debug_loop = 0;

/*
 * Strict Furigana Discard function
 * Utf8 used internally
 */
GSList* discard_strict_furigana(gchar* ptext, GSList* furigana_slist)
{
  return furigana_slist;
}

static GSList *
im_canna_get_furigana_slist (gchar *kakutei_text_euc,
			     gchar *muhenkan_text_euc)
{
  GSList *furigana_slist;
  gint kakutei_text_euc_len;
  gint muhenkan_text_euc_len;
  gint pre_hiragana_len;
  gint post_hiragana_len;
  gboolean as_is;


  furigana_slist = NULL;

  pre_hiragana_len  = im_canna_get_pre_hiragana_len  (kakutei_text_euc);
  post_hiragana_len = im_canna_get_post_hiragana_len (kakutei_text_euc);

  kakutei_text_euc_len  = strlen (kakutei_text_euc);
  muhenkan_text_euc_len = strlen (muhenkan_text_euc);

  as_is = FALSE;

  if ((as_is == FALSE) &&
      ((pre_hiragana_len + post_hiragana_len) > muhenkan_text_euc_len))
    {
      as_is = TRUE;
    }

  if ((as_is == FALSE) &&
      (pre_hiragana_len > 0))
    {
      if ((strncmp (kakutei_text_euc,
		    muhenkan_text_euc,
		    pre_hiragana_len) != 0))
	{
	  as_is = TRUE;
	}
    }

  if ((as_is == FALSE) &&
      (post_hiragana_len > 0))
    {
      if ((strncmp (((kakutei_text_euc
		      + kakutei_text_euc_len)
		     - (post_hiragana_len)),
		    ((muhenkan_text_euc
		      + muhenkan_text_euc_len)
		     - (post_hiragana_len)),
		    post_hiragana_len) != 0))
	{
	  as_is = TRUE;
	}
    }

  if (as_is == FALSE)
    {
      gchar *tmp_kakutei_text_euc;
      gchar *tmp_muhenkan_text_euc;
      gint   tmp_kakutei_text_euc_len;
      gint   tmp_muhenkan_text_euc_len;

      tmp_kakutei_text_euc
	= g_strndup ((kakutei_text_euc + pre_hiragana_len),
		     (kakutei_text_euc_len
		      - pre_hiragana_len
		      - post_hiragana_len));
      tmp_muhenkan_text_euc
	= g_strndup ((muhenkan_text_euc + pre_hiragana_len),
		     (muhenkan_text_euc_len
		      - pre_hiragana_len
		      - post_hiragana_len));

      tmp_kakutei_text_euc_len  = strlen (tmp_kakutei_text_euc);
      tmp_muhenkan_text_euc_len = strlen (tmp_muhenkan_text_euc);

      {
	struct _Hiragana {
	  gint pos;
	  gint len;

	  gint pos2;
	};
	typedef struct _Hiragana Hiragana;

	GList *hiragana_list;
	GList *tmp;
	Hiragana *hiragana;
	int i;
	gchar *p;

	hiragana_list = NULL;
	hiragana = NULL;

	i = 0;
	while (i < tmp_kakutei_text_euc_len)
	  {
	    if ((tmp_kakutei_text_euc_len - i) < 2)
	      {
		if (hiragana != NULL)
		  {
		    hiragana->len = (i - hiragana->pos);
		    hiragana = NULL;
		  }

		break;
	      }

	    p = (tmp_kakutei_text_euc + i);

	    if (im_canna_is_euc_hiragana_char (p) == TRUE)
	      {
		if (hiragana == NULL)
		  {
		    hiragana = g_new0 (Hiragana, 1);

		    hiragana->pos = i;
		    hiragana_list = g_list_append (hiragana_list, hiragana);
		  }

		i += 2;
	      }
	    else
	      {
		if (hiragana != NULL)
		  {
		    hiragana->len = (i - hiragana->pos);
		    hiragana = NULL;
		  }

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

	if (hiragana_list != NULL)
	  {
	    GList *tmp2;
	    Hiragana *h;
	    gint l1;
	    gint l2;
	    gboolean match;
	    gchar *p;

	    tmp = hiragana_list;
	    
	    while (tmp != NULL)
	      {
		hiragana = (Hiragana *)tmp->data;
		tmp = g_list_next (tmp);

		l1 = 0;
		tmp2 = g_list_previous (tmp);
		while (tmp2 != NULL)
		  {
		    h = (Hiragana *)tmp2->data;
		    tmp2 = g_list_previous (tmp2);

		    l1 += h->len;
		  }

		l2 = 0;
		tmp2 = g_list_next (tmp);
		while (tmp2 != NULL)
		  {
		    h = (Hiragana *)tmp2->data;
		    tmp2 = g_list_next (tmp2);

		    l2 += h->len;
		  }

		match = FALSE;
		i = 0;
		while (i < (tmp_muhenkan_text_euc_len - l1 - l2 ))
		  {
		    if (((tmp_muhenkan_text_euc_len - l1 - l2) - i)
			< (hiragana->len))
		      {
			if (match == FALSE)
			  {
			    as_is = TRUE;
			  }
			break;
		      }

		    p = (tmp_muhenkan_text_euc + l1 + i);

		    if (im_canna_is_euc_char (p) == TRUE)
		      {
			if (strncmp (p,
				     (tmp_kakutei_text_euc + hiragana->pos),
				     hiragana->len)
			    == 0)
			  {
			    if (match == FALSE)
			      {
				match = TRUE;
				hiragana->pos2 = (l1 + i);
			      }
			    else
			      {
				as_is = TRUE;
				break;
			      }
			  }
			
			i += 2;
		      }
		    else
		      {
			i += 1;
		      }
		  }

		if (match == FALSE)
		  {
		    as_is = TRUE;
		    break;
		  }
	      }

	    if (as_is == FALSE)
	      {
		Furigana *furigana;
		gint pos;
		gint pos2;
		gchar *tmp_euc;

		pos = 0;
		pos2 = 0;

		{
		  hiragana = g_new0 (Hiragana, 1);

		  hiragana->pos = tmp_kakutei_text_euc_len;
		  hiragana->len = 0;
		  hiragana->pos2 = tmp_muhenkan_text_euc_len;

		  hiragana_list = g_list_append (hiragana_list, hiragana);
		}

		tmp = hiragana_list;
		while (tmp != NULL)
		  {
		    hiragana = (Hiragana *)tmp->data;
		    tmp = g_list_next (tmp);

		    furigana = g_new0 (Furigana, 1);

		    {
		      gint p1;
		      gint p2;

		      p1 = im_canna_get_utf8_pos_from_euc_pos
				(kakutei_text_euc,
				 (pre_hiragana_len +pos));
		      p2 = im_canna_get_utf8_pos_from_euc_pos
				(kakutei_text_euc,
				 (pre_hiragana_len + hiragana->pos));

		      pos = (hiragana->pos + hiragana->len);

		      furigana->offset = p1;
		      furigana->length = (p2 - p1);
		    }

		    {
		      gchar *tmp_utf8;
		      
		      tmp_euc = g_strndup ((tmp_muhenkan_text_euc + pos2),
					   (hiragana->pos2 - pos2));
		      tmp_utf8 = euc2utf8 (tmp_euc);
		      g_free (tmp_euc);

		      pos2 = (hiragana->pos2 + hiragana->len);

		      furigana->text = tmp_utf8;
		    }

		    furigana_slist = g_slist_append(furigana_slist, furigana);
		  }
	      }


	    g_list_foreach (hiragana_list, (GFunc)g_free, NULL);
	    g_list_free (hiragana_list);
	  }
	else
	  {
	    Furigana *furigana;
	    gchar *text_utf8;
	    gint length;
	    gint offset;

	    
	    text_utf8 = euc2utf8 (tmp_muhenkan_text_euc);
	    length = im_canna_get_utf8_len_from_euc (tmp_kakutei_text_euc);
	    offset = im_canna_get_utf8_pos_from_euc_pos (kakutei_text_euc,
							 pre_hiragana_len);


	    furigana = g_new0 (Furigana, 1);

	    furigana->text   = text_utf8;
	    furigana->length = length;
	    furigana->offset = offset;

	    furigana_slist = g_slist_append(furigana_slist, furigana);
	  }
      }
      
      g_free (tmp_kakutei_text_euc);
      g_free (tmp_muhenkan_text_euc);
    }

  if (as_is == TRUE)
    {
      Furigana *furigana;
      gchar *text_utf8;
      gchar *tmp_utf8;


      text_utf8 = euc2utf8 (muhenkan_text_euc);
      tmp_utf8  = euc2utf8 (kakutei_text_euc);


      furigana = g_new0 (Furigana, 1);

      furigana->text   = text_utf8;
      furigana->length = g_utf8_strlen (tmp_utf8, -1);
      furigana->offset = 0;


      g_free (tmp_utf8);


      furigana_slist = g_slist_append(furigana_slist, furigana);
    }

  return furigana_slist;
}
/***/

GSList* im_canna_get_furigana(IMContextCanna* cn)
{
  GSList* furigana_slist = NULL;

  gchar* finalstr = NULL; /* Final input string */
  gchar buffer[BUFSIZ]; /* local buffer */

  Furigana* f;
  guchar* reftext;
  guchar* candtext = NULL;
  guchar* recovertext = NULL;
  gchar* tmpbuf = NULL;

  gint   chunk_kakutei_text_euc_pos;
  gchar *chunk_kakutei_text_euc;
  gchar *chunk_muhenkan_text_euc;

  g_return_val_if_fail(cn->kslength > 0, NULL);

  chunk_kakutei_text_euc_pos = -1;
  chunk_kakutei_text_euc  = NULL;
  chunk_muhenkan_text_euc = NULL;


  finalstr = g_strndup(cn->ks.echoStr, cn->kslength);

  /* Discard Furigana for Hiragana */
  if ( im_canna_is_euc_hiragana(finalstr) ) {
    g_object_set_data(G_OBJECT(cn), "furigana", NULL);
    g_free(finalstr);
    return NULL;
  }

  memset(buffer, 0, BUFSIZ);
  strcpy(buffer, finalstr);

  debug_loop = 0;

  while(cn->ks.revPos != 0) {

    jrKanjiString(cn->canna_context, 0x06, buffer, BUFSIZ, &cn->ks); /* Ctrl+F */

    //    printf("pos: %d\n", cn->ks.revPos);
    //    printf("Len: %d\n", cn->ks.revLen);

    if( debug_loop++ > 100 ) {
      /* Happens when the candidate window */
      // printf("FURIGANA: while loop exceeded. (>100) Bug!!\n");
      g_object_set_data(G_OBJECT(cn), "furigana", NULL);
      return NULL;
    } 
  }

  g_assert(cn->ks.revLen > 0);

  do {
    recovertext = NULL;
    reftext = g_strndup(cn->ks.echoStr+cn->ks.revPos, cn->ks.revLen);
    // printf("reftext: %s\n", reftext);
    if( im_canna_is_euc_hiragana(reftext) ) {
      g_free(reftext);
      jrKanjiString(cn->canna_context, 0x06, buffer, BUFSIZ, &cn->ks); /* Ctrl+F */
      continue;
    }
    
    chunk_kakutei_text_euc_pos = cn->ks.revPos;
    chunk_kakutei_text_euc     = g_strdup (reftext);

    if( !im_canna_is_euc_hiragana(reftext) ) {
      GHashTable* loop_check_ht = g_hash_table_new(g_str_hash, g_str_equal);
      candtext = g_strdup(reftext);
      recovertext = g_strdup(""); /* Something free-able but not reftext */
      /* Loop until Hiragana for Furigana shows up */
	while( !im_canna_is_euc_hiragana(candtext) ) {
	  guint strcount = 0;

	  g_free(candtext);
          jrKanjiString(cn->canna_context, 0x0e, buffer, BUFSIZ, &cn->ks); /* Ctrl+N */
          candtext = g_strndup(cn->ks.echoStr+cn->ks.revPos, cn->ks.revLen);
	  
	  strcount = (guint)g_hash_table_lookup(loop_check_ht, candtext);

	  if( strcount >= 4 ) { /* specify threshold */
	    // printf("Looping, No Furigana: %s\n", reftext);
	    g_hash_table_foreach(loop_check_ht, strhash_dumpfunc, NULL);
	    g_free(candtext);
	    candtext = NULL;
	    break;
	  }
	  
	  g_hash_table_insert(loop_check_ht, g_strdup(candtext),
			      (gpointer)++strcount);
	}
	g_hash_table_foreach(loop_check_ht, strhash_removefunc, NULL);
	g_hash_table_destroy(loop_check_ht);
	// printf("candtext: %s\n", candtext ? candtext : (guchar*)("NO-FURIGANA"));

	chunk_muhenkan_text_euc = g_strdup (candtext);

	loop_check_ht = g_hash_table_new(g_str_hash, g_str_equal);
	/* Loop again to recover the final kakutei text */
	while( strcmp(recovertext, reftext) != 0 ) {
	  guint strcount;

	  g_free(recovertext);
	  jrKanjiString(cn->canna_context, 0x0e, buffer, BUFSIZ, &cn->ks); /* Ctrl+N */
	  recovertext = g_strndup(cn->ks.echoStr+cn->ks.revPos, cn->ks.revLen);

	  strcount = (guint)g_hash_table_lookup(loop_check_ht, recovertext);
	  if( strcount >= 4 ) { /* threshold */
	    /* Loop detected - Give up recovery */
	    jrKanjiString(cn->canna_context, 0x07, buffer, BUFSIZ, &cn->ks); /* Ctrl+G */
	    printf("Cannot recover: %s\n", reftext);
	    g_hash_table_foreach(loop_check_ht, strhash_dumpfunc, NULL);
	    if( candtext )
	      g_free(candtext);
	    candtext = NULL;
	    break;
	  }
	  g_hash_table_insert(loop_check_ht, g_strdup(recovertext),
			      (gpointer)++strcount);
	}
	g_hash_table_foreach(loop_check_ht, strhash_removefunc, NULL);
	g_hash_table_destroy(loop_check_ht);
 
	if( recovertext != NULL )
	  g_free(recovertext);
	if( reftext != NULL )
	  g_free(reftext);
	reftext = candtext;
    }
    if ((chunk_muhenkan_text_euc != NULL) &&
	(*chunk_muhenkan_text_euc != '\0'))
      {
	gint offset0;
	GSList *tmp_slist;
	GSList *tmp;
	Furigana *furigana;

	offset0 = im_canna_get_utf8_pos_from_euc_pos
	  		(finalstr, chunk_kakutei_text_euc_pos);

	tmp_slist = im_canna_get_furigana_slist (chunk_kakutei_text_euc,
						 chunk_muhenkan_text_euc);

	tmp = tmp_slist;
	while (tmp != NULL)
	  {
	    furigana = (Furigana *)tmp->data;
	    tmp = g_slist_next (tmp);

	    furigana->offset += offset0;
	  }

	furigana_slist = g_slist_concat (furigana_slist, tmp_slist);
      }

    if (chunk_kakutei_text_euc != NULL)
      {
	g_free (chunk_kakutei_text_euc);
      }
    if (chunk_muhenkan_text_euc != NULL)
      {
	g_free (chunk_muhenkan_text_euc);
      }
    
    jrKanjiString(cn->canna_context, 0x06, buffer, BUFSIZ, &cn->ks); /* Ctrl+F */
  } while ( cn->ks.revPos > 0 && cn->ks.revLen > 0 );

  furigana_slist = discard_strict_furigana(finalstr, furigana_slist);
  g_free(finalstr);
   
  if (0) { /* Dump furigana slist */
    int i = 0;
    for(i=0;i<g_slist_length(furigana_slist);i++) {
      Furigana* f = g_slist_nth_data(furigana_slist, i);
      g_print("IM_CANNA: furigana[%d]=\"%s\", pos=%d, len=%d\n",
	i, f->text, f->offset, f->length);
    }
  }

  g_object_set_data(G_OBJECT(cn), "furigana", furigana_slist);
  
  return furigana_slist;
}
