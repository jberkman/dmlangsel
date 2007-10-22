#ifndef DMLANGSEL_COMPAT_GDM_H
#define DMLANGSEL_COMPAT_GDM_H

#include <gdk/gdktypes.h>
#include <errno.h>

G_BEGIN_DECLS

#define   gdm_debug              g_debug

#define STX 0x2			/* Start of txt */
#define BEL 0x7			/* Bell, used to interrupt login for
				 * say timed login or something similar */

#define GDM_INTERRUPT_SELECT_LANG 'O'

#define GDM_RESPONSE_CANCEL "GDM_RESPONSE_CANCEL"

/* Testing for existance of a certain locale */
gboolean       ve_locale_exists (const char *loc);

#define VE_IGNORE_EINTR(expr) \
	do {		\
		errno = 0;	\
		expr;		\
	} while G_UNLIKELY (errno == EINTR);

#define        ve_string_empty(x) ((x)==NULL||(x)[0]=='\0')
#define        ve_sure_string(x) ((x)!=NULL?(x):"")

gboolean gdm_common_locale_is_displayable (const gchar *locale);

extern GdkRectangle gdm_wm_screen;

/*
 * Not really a WM function, center a gtk window on current screen
 * by setting uposition
 */
#define gdm_wm_center_window(cw)
#define gdm_wm_query_dialog(primary_message,       \
                            secondary_message,     \
                            posbutton,              \
                            negbutton,              \
                            has_cancel) 0

/* Refuse to focus the login window, poor mans modal dialogs */
#define gdm_wm_no_login_focus_push(a)
#define gdm_wm_no_login_focus_pop(a)

void lang_set_custom_callback (gchar *language);

void           gdm_daemon_config_get_user_session_lang (char **usrsess,
                                                        char **usrlang,
                                                        const char *homedir,
                                                        gboolean *savesess);
void	       gdm_daemon_config_set_user_session_lang (gboolean savesess,
                                                        gboolean savelang,
                                                        const char *home_dir,
                                                        const char *save_session,
                                                        const char *save_language);

G_END_DECLS

#endif /* DMLANGSEL_COMPAT_GDM_H */
