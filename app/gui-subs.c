
/*
 * The Real SoundTracker - GUI support routines
 *
 * Copyright (C) 1998-2001 Michael Krause
 * Copyright (C) 2005 Yury Aliaev (GTK+-2 porting)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <string.h>

#include "i18n.h"
#include "gui.h"
#include "gui-subs.h"
#include "extspinbutton.h"

static const char *status_messages[] = {
    N_("Ready."),
    N_("Playing song..."),
    N_("Playing pattern..."),
    N_("Loading module..."),
    N_("Module loaded."),
    N_("Saving module..."),
    N_("Module saved."),
    N_("Loading sample..."),
    N_("Sample loaded."),
    N_("Saving sample..."),
    N_("Sample saved."),
    N_("Loading instrument..."),
    N_("Instrument loaded."),
    N_("Saving instrument..."),
    N_("Instrument saved."),
    N_("Saving song..."),
    N_("Song saved."),
};

static GtkWidget *aacdialog = NULL;
static void(*aaccallback)(gint,gpointer);
static gpointer aaccallbackdata;

void
statusbar_update (int message, gboolean force_update)
{
#ifdef USE_GNOME
    gnome_appbar_set_status(GNOME_APPBAR(status_bar), _(status_messages[message]));
#else
    gtk_statusbar_pop(GTK_STATUSBAR(status_bar), statusbar_context_id);
    gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context_id, _(status_messages[message]));
#endif
    /* Take care here... GUI callbacks can be called at this point. */
    if(force_update) {
	while (gtk_events_pending())
	    gtk_main_iteration();
    }
}

int
find_current_toggle (GtkWidget **widgets, int count)
{
    int i;
    for (i = 0; i < count; i++) {
	if(GTK_TOGGLE_BUTTON(*widgets++)->active) {
	    return i;
	}
    }
    return -1;
}

void
add_empty_hbox (GtkWidget *tobox)
{
    GtkWidget *thing = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (thing);
    gtk_box_pack_start (GTK_BOX (tobox), thing, TRUE, TRUE, 0);
}

void
add_empty_vbox (GtkWidget *tobox)
{
    GtkWidget *thing = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (thing);
    gtk_box_pack_start (GTK_BOX (tobox), thing, TRUE, TRUE, 0);
}

void
make_radio_group_full (const char **labels,
		       GtkWidget *tobox,
		       GtkWidget **saveptr,
		       gint t1,
		       gint t2,
		       void (*sigfunc) (void),
		       gpointer data)
{
    GtkWidget *thing = NULL;

    while (*labels) {
	thing = gtk_radio_button_new_with_label ((thing
						  ? gtk_radio_button_get_group (GTK_RADIO_BUTTON (thing))
						  : NULL),
						 gettext(*labels++));
	*saveptr++ = thing;
	gtk_widget_show (thing);
	gtk_box_pack_start (GTK_BOX (tobox), thing, t1, t2, 0);
	if(sigfunc) {
	    g_signal_connect (thing, "clicked", G_CALLBACK(sigfunc), data);
	}
    }
}

void
make_radio_group (const char **labels, GtkWidget *tobox,
		  GtkWidget **saveptr, gint t1, gint t2,
		  void (*sigfunc) (void))
{
    make_radio_group_full(labels, tobox, saveptr, t1, t2, sigfunc, NULL);
}

GtkWidget*
make_labelled_radio_group_box_full (const char *title,
				    const char **labels,
				    GtkWidget **saveptr,
				    void (*sigfunc) (void),
				    gpointer data)
{
    GtkWidget *box, *thing;

    box = gtk_hbox_new(FALSE, 4);
  
    thing = gtk_label_new(title);
    gtk_widget_show(thing);
    gtk_box_pack_start(GTK_BOX(box), thing, FALSE, TRUE, 0);
   
    make_radio_group_full(labels, box, saveptr, FALSE, TRUE, sigfunc, data);

    return box;
}

GtkWidget*
make_labelled_radio_group_box (const char *title,
			       const char **labels,
			       GtkWidget **saveptr,
			       void (*sigfunc) (void))
{
    return make_labelled_radio_group_box_full(title, labels, saveptr, sigfunc, NULL);
}

void
gui_put_labelled_spin_button (GtkWidget *destbox,
			      const char *title,
			      int min,
			      int max,
			      GtkWidget **spin,
			      void(*callback)(),
			      void *callbackdata)
{
    GtkWidget *hbox, *thing;

    hbox = gtk_hbox_new(FALSE, 4);
    gtk_box_pack_start(GTK_BOX(destbox), hbox, FALSE, TRUE, 0);
    gtk_widget_show(hbox);

    thing = gtk_label_new(title);
    gtk_box_pack_start(GTK_BOX(hbox), thing, FALSE, TRUE, 0);
    gtk_widget_show(thing);

    add_empty_hbox(hbox);

    *spin = extspinbutton_new(GTK_ADJUSTMENT(gtk_adjustment_new(min, min, max, 1.0, 5.0, 0.0)), 0, 0);
    gtk_box_pack_start(GTK_BOX(hbox), *spin, FALSE, TRUE, 0);
    gtk_widget_show(*spin);
    g_signal_connect(*spin, "value-changed",
		       GTK_SIGNAL_FUNC(callback), callbackdata);
}

GtkWidget*
file_selection_create (const gchar *title,
		       void(*clickfunc)())
{
    GtkWidget *window;

    window = gtk_file_selection_new (title);
    gtk_window_position (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
	
    g_signal_connect(window, "destroy",
		       G_CALLBACK(gtk_main_quit),
		       NULL);
    g_signal_connect(GTK_FILE_SELECTION (window)->ok_button,
		       "clicked", G_CALLBACK(clickfunc),
		       window);
 
    g_signal_connect(window, "delete_event",
		       G_CALLBACK(gtk_widget_hide),
		       window);
    g_signal_connect_swapped(GTK_FILE_SELECTION (window)->cancel_button,
			      "clicked", G_CALLBACK(gtk_widget_hide),
			      GTK_OBJECT (window));

    return window;
}

void
file_selection_save_path (const gchar *fn,
			  gchar *store)
{
    gchar *dn = g_dirname(fn);

    strncpy(store, dn, 127);
    strncat(store, "/", 127);

    g_free(dn);
}

/* Return TRUE if there is a non-empty basename in the given
 * filename. */
gboolean
file_selection_is_valid (const gchar *fn)
{
    gchar *basename = g_basename(fn);

    return basename != NULL && strlen(basename) > 0;
}

static void
gui_subs_slider_update_1 (GtkWidget *w,
			  gui_subs_slider *s)
{
    int v = s->adjustment1->value;
    gtk_adjustment_set_value(s->adjustment2, v);
    if(!s->update_without_signal) {
	s->changedfunc(v);
    }
}

static void
gui_subs_slider_update_2 (GtkSpinButton *spin,
			  gui_subs_slider *s)
{
    int v = gtk_spin_button_get_value_as_int(spin);
    if(s->type == GUI_SUBS_SLIDER_WITH_HSCALE) {
	if(v != s->adjustment1->value) {
	    /* the 'if' is only needed for gtk+-1.0 */
	    gtk_adjustment_set_value(s->adjustment1, v);
	}
    }
    if(!s->update_without_signal) {
	s->changedfunc(v);
    }
}

GtkWidget *
gui_subs_create_slider (gui_subs_slider *s)
{
    GtkWidget *thing, *box;

    box = gtk_hbox_new(FALSE, 4);

    thing = gtk_label_new(gettext(s->title));
    gtk_widget_show(thing);
    gtk_box_pack_start(GTK_BOX(box), thing, FALSE, TRUE, 0);

    if(s->type == GUI_SUBS_SLIDER_WITH_HSCALE) {
	s->adjustment1 = GTK_ADJUSTMENT(gtk_adjustment_new(s->min, s->min, s->max, 1, (s->max - s->min) / 10, 0));
	s->slider = gtk_hscale_new(s->adjustment1);
	gtk_scale_set_draw_value(GTK_SCALE(s->slider), FALSE);
	gtk_widget_show(s->slider);
	gtk_box_pack_start(GTK_BOX(box), s->slider, TRUE, TRUE, 0);
	g_signal_connect(s->adjustment1, "value_changed",
			    G_CALLBACK(gui_subs_slider_update_1), s);
    } else {
	add_empty_hbox(box);
    }

    s->adjustment2 = GTK_ADJUSTMENT(gtk_adjustment_new(s->min, s->min, s->max, 1, (s->max - s->min) / 10, 0));
    thing = extspinbutton_new(s->adjustment2, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), thing, FALSE, TRUE, 0);
    gtk_widget_show(thing);
    g_signal_connect(thing, "value-changed",
		       G_CALLBACK(gui_subs_slider_update_2), s);

    s->update_without_signal = FALSE;

    return box;
}

void
gui_subs_set_slider_value (gui_subs_slider *s,
			   int v)
{
    gtk_adjustment_set_value(s->adjustment2, v);
}

int
gui_subs_get_slider_value (gui_subs_slider *s)
{
    return s->adjustment2->value;
}

void
gui_update_spin_adjustment (GtkSpinButton *spin,
			    int min,
			    int max)
{
    GtkAdjustment* adj;
    int p;

    adj = gtk_spin_button_get_adjustment(spin);
    p = adj->value;
    if(p < min)
	p = min;
    else if(p > max)
	p = max;

    /* p+1 and the extra set_value is required due to a bug in gtk+ */
    adj = GTK_ADJUSTMENT(gtk_adjustment_new(p+1, min, max, 1, 1, 0));
    gtk_spin_button_set_adjustment(spin, adj); /* old adjustment is freed automagically */
    gtk_adjustment_set_value(adj, p);
}

void
gui_update_range_adjustment (GtkRange *range,
			     int pos,
			     int upper,
			     int window,
			     void(*func)())
{
    GtkAdjustment* adj;

    adj = gtk_range_get_adjustment(range);
    if(adj->upper != upper || adj->page_size != window) {
	/* pos+1 and the extra set_value is required due to a bug in gtk 1.0.4 */
	adj = GTK_ADJUSTMENT(gtk_adjustment_new(pos+1, 0, upper, 1, window-2, window));
	gtk_range_set_adjustment(range, adj); /* old adjustment is freed automagically */
	gtk_adjustment_set_value(adj, pos);
	g_signal_connect(adj, "value_changed", G_CALLBACK(func), NULL);
    } else {
	if((int)(adj->value) != pos)
	    gtk_adjustment_set_value(adj, pos);
    }
}

void
gui_hang_tooltip (GtkWidget *widget, const gchar *text)
{
    GtkTooltips *tips = NULL;
    
    if(!tips) {
	tips = gtk_tooltips_new();
    }
    gtk_tooltips_set_tip(tips, widget, text, NULL);
}

// Stolen from testgtk.c and modified
GtkWidget *
gui_build_option_menu (OptionMenuItem items[],
		   gint           num_items,
		   gint           history)
{
  GtkWidget *omenu;
  GtkWidget *menu;
  GtkWidget *menu_item;
  GSList *group;
  gint i;

  omenu = gtk_option_menu_new ();
      
  menu = gtk_menu_new ();
  group = NULL;
  
  for (i = 0; i < num_items; i++)
    {
      menu_item = gtk_radio_menu_item_new_with_label (group, items[i].name);
      g_signal_connect (menu_item, "activate",
			  G_CALLBACK(items[i].func), GINT_TO_POINTER(i));
      group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (menu_item));
      gtk_menu_append (GTK_MENU (menu), menu_item);
      if (i == history)
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), TRUE);
      gtk_widget_show (menu_item);
    }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (omenu), menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (omenu), history);
  
  return omenu;
}

GtkWidget *
gui_stringlist_in_scrolled_window (int n, gchar **tp,  GtkWidget *hbox)
{
    GType *types;
    GtkWidget *list;
    guint i;
    
    types = g_new(GType, n);
    for(i = 0; i < n; i++)
	types[i] = G_TYPE_STRING;
    list = gui_list_in_scrolled_window(n, tp, hbox, types, NULL, NULL, GTK_SELECTION_BROWSE);
    g_free(types);
    return list;
}

extern inline void
gui_list_clear (GtkWidget *list)
{
    gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list))));
}

extern inline void
gui_list_clear_with_model (GtkTreeModel *model)
{
    gtk_list_store_clear(GTK_LIST_STORE(model));
}

GtkTreeModel *
gui_list_freeze (GtkWidget *list)
{
    GtkTreeModel *model;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
    g_object_ref(model);
    gtk_tree_view_set_model(GTK_TREE_VIEW(list), NULL);

    return model;
}

void
gui_list_thaw (GtkWidget *list, GtkTreeModel *model)
{
    gtk_tree_view_set_model(GTK_TREE_VIEW(list), model);
    g_object_unref(model);
}

static gboolean hover_changed (GtkTreeView *widget, GdkEvent* event, gpointer data)
{
    gboolean is_hover = (gboolean) data;
    gtk_tree_view_set_hover_selection(widget, is_hover);
    return FALSE;
}

GtkWidget *
gui_list_in_scrolled_window (int n, gchar **tp,  GtkWidget *hbox,
			     GType *types, gfloat *alignments, gboolean *expands,
			     GtkSelectionMode mode)
{
    GtkWidget *list;
    GtkWidget *sw;
    guint i;
    GtkListStore *list_store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    GtkTreeSelection *sel;

    list_store = gtk_list_store_newv(n, types);
    list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
    for(i = 0; i < n; i++) {
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(tp[i], renderer, "text", i, NULL);
	if(alignments) {
	    g_object_set(G_OBJECT(renderer), "xalign", alignments[i], NULL);
	    gtk_tree_view_column_set_alignment(column, alignments[i]);
	}
	g_object_set(G_OBJECT(renderer), "ypad", 0, NULL);
	if(expands)
	    gtk_tree_view_column_set_expand(column, expands[i]);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
    }

    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));
    gtk_tree_selection_set_mode(sel, mode);

    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(sw), list);

    /* Making the pointer following the cursor when the button is pressed (like in gtk+-1) */
    /* TODO: enabling autoscrolling when the pointer is moved up/down */
    g_signal_connect(list, "button-press-event", G_CALLBACK(hover_changed), (gpointer)TRUE);
    g_signal_connect(list, "button-release-event", G_CALLBACK(hover_changed), (gpointer)FALSE);
    g_signal_connect(list, "leave-notify-event", G_CALLBACK(hover_changed), (gpointer)FALSE);

    gtk_widget_show(sw);
    gtk_box_pack_start(GTK_BOX(hbox), sw, TRUE, TRUE, 0);
    /* According to Gtk+ documentation this is not recommended but the lists are not strippy by default...*/
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(list), TRUE);
    /* This speeds ip a bit */ //!!!but currently not working
//    gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(list), TRUE);
    gtk_widget_show(list);
    return list;
}

void
gui_list_handle_selection (GtkWidget *list, GCallback handler, gpointer data)
{
    GtkTreeSelection *sel;

    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));
    g_signal_connect_after(sel, "changed", handler, data);
}

extern inline gboolean
gui_list_get_iter (guint n, GtkListStore *tree_model, GtkTreeIter *iter)
{
    gchar *path;
    gboolean result;
    
    path = g_strdup_printf("%u", n);
    result = gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(tree_model), iter, path);
    g_free(path);
    return result;
}

extern inline void
gui_string_list_set_text (GtkWidget *list, guint row, guint col, const gchar *string)
{
    GtkTreeIter iter;
    GtkListStore *list_store;

    if(gui_list_get_iter(row, list_store = GUI_GET_LIST_STORE(list), &iter))
	gtk_list_store_set(list_store, &iter, col, string, -1);
}

extern inline void
gui_list_select (GtkWidget *list, guint row, gboolean use_align, gfloat align)
{
    gchar *path_string;
    GtkTreePath *path;
    GtkTreeIter iter;
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));

    if(!gui_list_get_iter(row, GUI_GET_LIST_STORE(list), &iter))
       return;
    gtk_tree_selection_select_iter(sel, &iter);
    path_string = g_strdup_printf("%u", row);
    path = gtk_tree_path_new_from_string(path_string);
    gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(list), path, NULL,
			         use_align, align, 0.0);

    g_free(path_string);
    gtk_tree_path_free(path);
}

GtkWidget *
gui_button (GtkWidget * win, char *stock,
	    void *callback, gpointer userdata, GtkWidget * box)
{
   GtkWidget *button;

   button = gtk_button_new_from_stock (stock);
   g_signal_connect(button, "clicked",
                    G_CALLBACK(callback), userdata);
   gtk_widget_show (button);

   if (box)
      gtk_container_add (GTK_CONTAINER (box), button);

   return button;
}

static void
aacdialog_close (gpointer data)
{
    gtk_widget_destroy(aacdialog);
    aacdialog = NULL;
    
    aaccallback((gint)data, aaccallbackdata);
}

void
gui_yes_no_cancel_modal (GtkWidget *window,
			   const gchar *text,
			   void (*callback)(gint, gpointer),
			   gpointer data)
{
    GtkWidget *label, *button;

    g_return_if_fail(aacdialog == NULL);

    aaccallback = callback;
    aaccallbackdata = data;
    
    aacdialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_position (GTK_WINDOW(aacdialog), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(aacdialog), _("Question"));
    gtk_window_set_modal(GTK_WINDOW(aacdialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(aacdialog), GTK_WINDOW(window));

    label = gtk_label_new(text);
    gtk_container_border_width(GTK_CONTAINER(GTK_DIALOG(aacdialog)->vbox), 10);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(aacdialog)->vbox), label, TRUE, TRUE, 10);
    gtk_widget_show(label);
    
    button = gtk_button_new_with_label (_("Yes"));
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(aacdialog)->action_area), button, TRUE, TRUE, 10);
    g_signal_connect_swapped(button, "clicked",
                               G_CALLBACK(aacdialog_close), (gpointer)0);
    gtk_widget_grab_default (button);
    gtk_widget_show (button);
    
    button = gtk_button_new_with_label (_("No"));
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(aacdialog)->action_area), button, TRUE, TRUE, 10);
    g_signal_connect_swapped (button, "clicked",
                              G_CALLBACK(aacdialog_close), (gpointer)1);
    gtk_widget_show (button);
    
    button = gtk_button_new_with_label (_("Cancel"));
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(aacdialog)->action_area), button, TRUE, TRUE, 10);
    g_signal_connect_swapped (button, "clicked",
                               G_CALLBACK(aacdialog_close), (gpointer)2);
    gtk_widget_show (button);
    
    gtk_widget_show(aacdialog);
}


#ifndef USE_GNOME

static GtkWidget *cdialog = NULL;
static void(*ccallback)(gint,gpointer);
static gpointer ccallbackdata;

static void
cdialog_close (gpointer data)
{
    gtk_widget_destroy(cdialog);
    cdialog = NULL;
    
    ccallback(GPOINTER_TO_INT(data), ccallbackdata);
}

static void
dialog_close (gpointer data)
{
    gtk_widget_destroy(data);
}

void
gnome_app_ok_cancel_modal (GtkWidget *window,
			   const gchar *text,
			   void (*callback)(gint, gpointer),
			   gpointer data)
{
    GtkWidget *label, *button;

    g_return_if_fail(cdialog == NULL);

    ccallback = callback;
    ccallbackdata = data;
    
    cdialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_position (GTK_WINDOW(cdialog), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(cdialog), _("Question"));
    gtk_window_set_modal(GTK_WINDOW(cdialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(cdialog), GTK_WINDOW(window));

    label = gtk_label_new(text);
    gtk_container_border_width(GTK_CONTAINER(GTK_DIALOG(cdialog)->vbox), 10);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(cdialog)->vbox), label, TRUE, TRUE, 10);
    gtk_widget_show(label);
    
    button = gtk_button_new_with_label ("Ok");
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(cdialog)->action_area), button, TRUE, TRUE, 10);
    g_signal_connect_swapped(button, "clicked",
                               G_CALLBACK(cdialog_close), (gpointer)0);
    gtk_widget_grab_default (button);
    gtk_widget_show (button);
    
    button = gtk_button_new_with_label (_("Cancel"));
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(cdialog)->action_area), button, TRUE, TRUE, 10);
    g_signal_connect_swapped(button, "clicked",
                               G_CALLBACK(cdialog_close), (gpointer)1);
    gtk_widget_show (button);
    
    gtk_widget_show(cdialog);
}

void
gnome_warning_dialog (gchar *text)
{
    GtkWidget *label, *button;
    GtkWidget *dialog;

    dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_position (GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(dialog), _("Warning"));

    label = gtk_label_new(text);
    gtk_container_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), 10);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE, TRUE, 10);
    gtk_widget_show(label);
    
    button = gtk_button_new_with_label ("Ok");
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->action_area), button, TRUE, TRUE, 10);
    g_signal_connect_swapped(button, "clicked",
                               G_CALLBACK(dialog_close), (gpointer)dialog);
    gtk_widget_grab_default (button);
    gtk_widget_show (button);
    
    gtk_widget_show(dialog);
}

void
gnome_error_dialog (gchar *text)
{
    GtkWidget *label, *button;
    GtkWidget *dialog;

    dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_position (GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(dialog), _("Error!"));

    label = gtk_label_new(text);
    gtk_container_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), 10);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE, TRUE, 10);
    gtk_widget_show(label);
    
    button = gtk_button_new_with_label ("Ok");
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->action_area), button, TRUE, TRUE, 10);
    g_signal_connect_swapped(button, "clicked",
                               G_CALLBACK(dialog_close), (gpointer)dialog);
    gtk_widget_grab_default (button);
    gtk_widget_show (button);
    
    gtk_widget_show(dialog);
}

#endif /* USE_GNOME */
