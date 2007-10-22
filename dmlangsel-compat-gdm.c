#include "config.h"

#include "dmlangsel-compat-gdm.h"

#include "dmlangsel.h"

#include "gdm-common-config.h"

#include <fontconfig/fontconfig.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

GdkRectangle gdm_wm_screen = {0,0,0,0};

gboolean
ve_locale_exists (const char *loc)
{
	gboolean ret;
	char *old = g_strdup (setlocale (LC_MESSAGES, NULL));
	if (setlocale (LC_MESSAGES, loc) != NULL)
		ret = TRUE;
	else
		ret = FALSE;
	setlocale (LC_MESSAGES, old);
	g_free (old);
	return ret;
}

typedef enum
{
  LOCALE_UP_TO_LANGUAGE = 0,
  LOCALE_UP_TO_COUNTRY,
  LOCALE_UP_TO_ENCODING,
  LOCALE_UP_TO_MODIFIER,
} LocaleScope;

static char *
get_less_specific_locale (const char *locale,
                          LocaleScope scope)
{
  char *generalized_locale;
  char *end;

  generalized_locale = strdup (locale);

  end = strchr (generalized_locale, '_');

  if (end != NULL && scope <= LOCALE_UP_TO_LANGUAGE)
    {
      *end = '\0';
      return generalized_locale;
    }

  end = strchr (generalized_locale, '.');

  if (end != NULL && scope <= LOCALE_UP_TO_COUNTRY)
    {
      *end = '\0';
      return generalized_locale;
    }

  end = strchr (generalized_locale, '@');

  if (end != NULL && scope <= LOCALE_UP_TO_ENCODING)
    {
      *end = '\0';
      return generalized_locale;
    }

  return generalized_locale;
}

gboolean
gdm_common_locale_is_displayable (const gchar *locale)
{
  char *language_code;
  gboolean is_displayable;

  FcPattern *pattern;
  FcObjectSet *object_set;
  FcFontSet *font_set;

  /* DMLS_ENTER; */

  is_displayable = FALSE;
  pattern = NULL;
  object_set = NULL;
  font_set = NULL;

  language_code = get_less_specific_locale (locale, LOCALE_UP_TO_LANGUAGE);

  pattern = FcPatternBuild (NULL, FC_LANG, FcTypeString, language_code, NULL);

  if (pattern == NULL) {
      DMLS_TRACE ("displayable: %s: no pattern...", locale);
      goto done;
  }

  object_set = FcObjectSetBuild (NULL, NULL);

  if (object_set == NULL) {
      DMLS_TRACE ("displayable: %s: no object set", locale);
      goto done;
  }

  font_set = FcFontList (NULL, pattern, object_set);

  if (font_set == NULL) {
      DMLS_TRACE ("displayable: %s: no font set", locale);
      goto done;
  }

  is_displayable = (font_set->nfont > 0);

  if (!is_displayable) {
      DMLS_TRACE ("displayable: %s: not displayable", locale);
  }

done:

  if (font_set != NULL)
    FcFontSetDestroy (font_set);

  if (object_set != NULL)
    FcObjectSetDestroy (object_set);

  if (pattern != NULL)
    FcPatternDestroy (pattern);

  g_free (language_code);

  /* DMLS_EXIT; */

  return is_displayable;
}

/*
 * This function does nothing for gdmlogin, but gdmgreeter does do extra
 * work in this callback function.
 */
void
lang_set_custom_callback (gchar *language)
{
}

/**
 * gdm_daemon_config_get_user_session_lang
 *
 * These functions get and set the user's language and setting in their
 * $HOME/.dmrc file.
 */
void
gdm_daemon_config_set_user_session_lang (gboolean savesess,
					 gboolean savelang,
					 const char *home_dir,
					 const char *save_session,
					 const char *save_language)
{
	GKeyFile *dmrc;
	gchar *cfgstr;

	cfgstr = g_build_filename (home_dir, ".dmrc", NULL);
	dmrc = gdm_common_config_load (cfgstr, NULL);
	if (dmrc == NULL) {
                gint fd = -1;
		gdm_debug ("file: %s does not exist - creating it", cfgstr);
		VE_IGNORE_EINTR (fd = g_open (cfgstr, O_CREAT | O_TRUNC | O_RDWR, 0644));
		if (fd < 0) return;
		write (fd, "\n", 2);
		close (fd);
                dmrc = gdm_common_config_load (cfgstr, NULL);
                if (dmrc == NULL) {
                    gdm_debug ("failed to open %s after creating it", cfgstr);
                    return;
                } 
                
        }

	if (savesess) {
		g_key_file_set_string (dmrc, "Desktop", "Session", ve_sure_string (save_session));
	}

	if (savelang) {
		if (ve_string_empty (save_language)) {
			/* we chose the system default language so wipe the
			 * lang key */
			g_key_file_remove_key (dmrc, "Desktop", "Language", NULL);
		} else {
			g_key_file_set_string (dmrc, "Desktop", "Language", save_language);
		}
	}

	if (dmrc != NULL) {
		mode_t oldmode;
		oldmode = umask (077);
		gdm_common_config_save (dmrc, cfgstr, NULL);
		umask (oldmode);
	}

	g_free (cfgstr);
	g_key_file_free (dmrc);
}

void
gdm_daemon_config_get_user_session_lang (char      **usrsess,
					 char      **usrlang,
					 const char *home_dir,
					 gboolean   *savesess)
{
	char *p;
	char *cfgfile;
	GKeyFile *cfg;
	char *session;
	char *lang;
	gboolean save;

	cfgfile = g_build_filename (home_dir, ".dmrc", NULL);
	cfg = gdm_common_config_load (cfgfile, NULL);
	g_free (cfgfile);

	save = FALSE;
	session = NULL;
	lang = NULL;

	gdm_common_config_get_string (cfg, "Desktop/Session", &session, NULL);
	if (session == NULL) {
		session = g_strdup ("");
	}

	/* this is just being truly anal about what users give us, and in case
	 * it looks like they may have included a path whack it. */
	p = strrchr (session, '/');
	if (p != NULL) {
		char *tmp = g_strdup (p + 1);
		g_free (session);
		session = tmp;
	}

	/* ugly workaround for migration */
	if (strcmp (session, "Default") == 0 ||
	    strcmp (session, "Default.desktop") == 0) {
		g_free (session);
		session = g_strdup ("default");
		save = TRUE;
	}

	gdm_common_config_get_string (cfg, "Desktop/Language", &lang, NULL);
	if (lang == NULL) {
		lang = g_strdup ("");
	}

	if (usrsess != NULL) {
		*usrsess = g_strdup (session);
	}
	g_free (session);

	if (savesess != NULL) {
		*savesess = save;
	}

	if (usrlang != NULL) {
		*usrlang = g_strdup (lang);
	}
	g_free (lang);

	g_key_file_free (cfg);
}
