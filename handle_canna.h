#ifndef _HANDLE_CANNA_H
#define _HANDLE_CANNA_H
extern void handle_modebuf (IMContextCanna* cn);
extern void handle_gline (IMContextCanna* cn);
extern void clear_gline (IMContextCanna* cn);
extern void handle_preedit (IMContextCanna *cn);
extern void im_canna_init_preedit (IMContextCanna *cn);
extern void clear_preedit (IMContextCanna *cn);

extern void im_canna_get_string_of_canna_mode(IMContextCanna* cn, gchar **ret_string);
extern int im_canna_get_num_of_canna_mode(IMContextCanna* cn);
extern void im_canna_force_change_mode(IMContextCanna* cn, int mode);
extern void im_canna_kill_unspecified_string(IMContextCanna* cn);
extern int im_canna_connect_server(IMContextCanna* cn);
extern int im_canna_disconnect_server(IMContextCanna* cn);
#endif

