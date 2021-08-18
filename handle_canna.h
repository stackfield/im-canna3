#ifndef _HANDLE_CANNA_H
#define _HANDLE_CANNA_H
extern void handle_modebuf (IMContextCanna* cn);
extern void handle_gline (IMContextCanna* cn);
extern void clear_gline (IMContextCanna* cn);

extern int im_canna_get_num_of_canna_mode(IMContextCanna* cn);
extern void im_canna_force_change_mode(IMContextCanna* cn, int mode);
#endif

