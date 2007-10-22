#include "config.h"

#include "dmlangsel.h"

#include <gtk/gtkliststore.h>
#include "gdmlanguages.h"

#include <gtk/gtk.h>

#include <glib/gi18n.h>

static struct {
    char *lang;
    GtkListStore *model;
    GtkWidget *window;
    GtkWidget *list;
    GtkWidget *label;
    GtkWidget *close;
    GtkWidget *hbox;
    GtkWidget *notice;
    gboolean running;
} dm;

static void
update_for_lang ()
{
    setlocale (LC_ALL, dm.lang);
    if (dm.window) {
        gtk_window_set_title (GTK_WINDOW (dm.window), _("Select a Language"));
    }
    if (dm.label) {
        gtk_label_set_markup_with_mnemonic (GTK_LABEL (dm.label), _("_Select the language for your session to use:"));
    }
    if (dm.close) {
        gtk_button_set_label (GTK_BUTTON (dm.close), GTK_STOCK_CLOSE);
    }
    if (dm.notice) {
        gtk_label_set_text (GTK_LABEL (dm.notice), _("Changes will take effect next time you log in."));
    }
}

static void
selection_changed (GtkTreeSelection *selection,
		   gpointer          data)
{
    GtkTreeIter iter;
    DMLS_ENTER;
    if (gtk_tree_selection_get_selected (selection, NULL, &iter)) {
        g_free (dm.lang);
        gtk_tree_model_get (GTK_TREE_MODEL (dm.model), &iter, LOCALE_COLUMN, &dm.lang, -1);
        update_for_lang ();
        if (dm.running) {
            gtk_widget_show (dm.hbox);
        }
        DMLS_TRACE ("now selected: %s", dm.lang);
    }
    DMLS_EXIT;
}

static void
tree_row_activated (GtkTreeView         *view,
                    GtkTreePath         *path,
                    GtkTreeViewColumn   *column,
                    gpointer            data)
{
    GtkTreeIter iter;
    DMLS_ENTER;
    if (gtk_tree_model_get_iter (GTK_TREE_MODEL (dm.model), &iter, path)) {
        g_free (dm.lang);
        gtk_tree_model_get (GTK_TREE_MODEL (dm.model), &iter,
                            LOCALE_COLUMN, &dm.lang,
                            -1);
        gtk_dialog_response (GTK_DIALOG (dm.window), GTK_RESPONSE_CLOSE);
    }
    DMLS_EXIT;
}

static GtkWidget *
get_tree (void)
{
    GtkWidget *swindow;
    DMLS_ENTER;
    dm.list = gtk_tree_view_new ();
    gtk_label_set_mnemonic_widget (GTK_LABEL (dm.label), dm.list);
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (dm.list), TRUE);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (dm.list), FALSE);
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (dm.list),
                                                 GTK_DIALOG_MODAL,
                                                 NULL,
                                                 gtk_cell_renderer_text_new (),
                                                 "text", TRANSLATED_NAME_COLUMN,
                                                 NULL);
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (dm.list),
                                                 GTK_DIALOG_MODAL,
                                                 NULL,
                                                 gtk_cell_renderer_text_new (),
                                                 "markup",
                                                 UNTRANSLATED_NAME_COLUMN,
                                                 NULL);
    g_signal_connect (G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (dm.list))),
                      "changed", (GCallback) selection_changed, NULL);
    g_signal_connect (G_OBJECT (dm.list), "row_activated", (GCallback) tree_row_activated, NULL);

    swindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (swindow), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (swindow), dm.list);
    
    gtk_tree_view_set_model (GTK_TREE_VIEW (dm.list), GTK_TREE_MODEL (dm.model));
    DMLS_EXIT;
    return swindow;
}

static void
select_lang (char *language)
{
   char *locale;
   GtkTreeSelection *selection;
   GtkTreeIter iter = {0};

   DMLS_ENTER;

   selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dm.list));
   gtk_tree_selection_unselect_all (selection);

   if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (dm.model), &iter)) {
      do {
         gtk_tree_model_get (GTK_TREE_MODEL (dm.model), &iter, LOCALE_COLUMN, &locale, -1);
         if (locale != NULL && strcmp (locale, language) == 0) {
            GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (dm.model), &iter);

            gtk_tree_selection_select_iter (selection, &iter);
            gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (dm.list), path, NULL, FALSE, 0.0, 0.0);
            gtk_tree_path_free (path);
            break;
         }
      } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (dm.model), &iter));
   }

   update_for_lang ();

   DMLS_EXIT;
}


static char *
load_lang (void)
{
    char *lang;
    DMLS_ENTER;

    gdm_daemon_config_get_user_session_lang (NULL, &lang, g_get_home_dir (), NULL);

    if (!*lang) {
        const gchar * const *langs;
        int i;

        g_free (lang);
        lang = NULL;
        
        langs = g_get_language_names ();
        for (i = 0; langs[i]; i++) {
            if (gdm_common_locale_is_displayable (langs[i])) {
                lang = g_strdup (langs[i]);
                break;
            }
        }
        if (!lang) {
            lang = g_strdup ("");
        }
    }
    DMLS_TRACE ("got our lang: %s", lang);

    DMLS_EXIT;
    return lang;
}

static void
save_lang (const char *lang)
{
    DMLS_ENTER;

    DMLS_TRACE ("saving the lang: %s", lang);
    gdm_daemon_config_set_user_session_lang (FALSE, TRUE, g_get_home_dir (), NULL, lang);

    DMLS_EXIT;
}

static GtkListStore *
get_model (void)
{
    GtkListStore *model;
    char *aliases[] = { "/etc/gdm/locale.alias", "/etc/opt/gnome/gdm/locale.alias", NULL };
    GtkTreeIter iter;
    int i;

    DMLS_ENTER;

    for (i = 0; aliases[i]; i++) {
        if (g_file_test (aliases[i], G_FILE_TEST_EXISTS)) {
#if 0
            GList *list, *li;
            
            list = gdm_lang_read_locale_file (aliases[i]);
            g_print ("got %d languages\n", g_list_length (list));
            for (li = list; li != NULL; li = li->next) {
                g_print ("got lang: %s\n", (char *)li->data);
            }
#endif

            gdm_lang_initialize_model (aliases[i]);
            break;
        }
    }
    if (!aliases[i]) {
        gdm_lang_initialize_model ("/dev/null");
    }
    
    model = gdm_lang_get_model ();
    gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter);
    /* remove "last" and "system" */
    gtk_list_store_remove (model, &iter);
    gtk_list_store_remove (model, &iter);

    DMLS_EXIT;

    return model;
}

static GtkWidget *
get_dialog (void)
{
    GtkWidget *w;
    GtkWidget *main_vbox;

    w = gtk_dialog_new ();
    gtk_dialog_set_has_separator (GTK_DIALOG (w), FALSE);
    dm.close = gtk_dialog_add_button (GTK_DIALOG (w), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
    
    gtk_container_set_border_width (GTK_CONTAINER (w), 5);
    gtk_window_set_default_size (GTK_WINDOW (w), 400, 300);

    main_vbox = gtk_vbox_new (FALSE, 6);
    gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 5);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (w)->vbox),
                        main_vbox, TRUE, TRUE, 0);

    dm.label = gtk_label_new ("");
    gtk_misc_set_alignment (GTK_MISC (dm.label), 0, 0.5);
    gtk_label_set_use_markup (GTK_LABEL (dm.label), TRUE);
    gtk_box_pack_start (GTK_BOX (main_vbox),
                        dm.label, FALSE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (main_vbox), get_tree (), TRUE, TRUE, 0);

    dm.hbox = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start (GTK_BOX (dm.hbox),
                        gtk_image_new_from_stock (GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_MENU),
                        FALSE, FALSE, 0);

    dm.notice = gtk_label_new ("");
    gtk_box_pack_start (GTK_BOX (dm.hbox), dm.notice, FALSE, FALSE, 0);
    
    gtk_box_pack_start (GTK_BOX (main_vbox), dm.hbox, FALSE, FALSE, 0);

    gtk_widget_show_all (w);
    gtk_widget_hide (dm.hbox);
                        
    return w;
}

int
main (int argc, char *argv[])
{
    DMLS_ENTER;

    setlocale (LC_ALL, "");
    bindtextdomain(PACKAGE_NAME, LOCALEDIR);
    bind_textdomain_codeset (PACKAGE_NAME, "UTF-8");
    textdomain(PACKAGE_NAME);

    gtk_init (&argc, &argv);

    dm.model = get_model ();

    dm.window = get_dialog ();
    select_lang (load_lang ());

    dm.running = TRUE;
    gtk_dialog_run (GTK_DIALOG (dm.window));

    if (dm.lang) {
        save_lang (dm.lang);
    }

    DMLS_EXIT;

    return 0;
}
