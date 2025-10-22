// gui.c
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#define COL_NAME 0
#define COL_TOP_INSTITUTE 1
#define COL_TOP_COURSE 2
#define COL_STATUS 3
#define NUM_COLS 4

GtkTreeStore *store;
GtkWidget *treeview;
HeapNode *root = NULL;

CollegeRow *college_data = NULL;
int college_count = 0;

/* Update TreeView row */
void update_treeview_row(HeapNode *node)
{
    GtkTreeIter iter;
    Offer *best_offer = (node->offer_count > 0) ? &node->offers[0] : NULL;

    gtk_tree_store_append(store, &iter, NULL);
    gtk_tree_store_set(store, &iter,
                       COL_NAME, node->student.name,
                       COL_TOP_INSTITUTE, best_offer ? best_offer->row.Institute : "None",
                       COL_TOP_COURSE, best_offer ? best_offer->row.Academic : "None",
                       COL_STATUS, best_offer ? (best_offer->status == OFFER_FROZEN ? "FROZEN" : best_offer->status == OFFER_FLOATED ? "FLOATED"
                                                                                             : best_offer->status == OFFER_SLID      ? "SLID"
                                                                                                                                     : "PENDING")
                                              : "PENDING",
                       -1);
}

/* Traverse heap */
void traverse_heap(HeapNode *node, void (*callback)(HeapNode *))
{
    if (!node)
        return;
    callback(node);
    traverse_heap(node->left, callback);
    traverse_heap(node->right, callback);
}

/* Refresh TreeView */
void refresh_treeview()
{
    gtk_tree_store_clear(store);
    traverse_heap(root, update_treeview_row);
}

/* Allocate seat for top offer */
void allocate_seat(HeapNode *node)
{
    if (!node || node->offer_count == 0)
        return;
    Offer *best = &node->offers[0];
    if (best->row.Seats_left > 0)
    {
        best->row.Seats_left--;
        best->status = OFFER_FROZEN;
        // Update global college_data
        college_data[best->idx].Seats_left = best->row.Seats_left;
        // If Seats_left==0, remove college from dataset
        if (college_data[best->idx].Seats_left == 0)
        {
            for (int i = best->idx; i < college_count - 1; i++)
                college_data[i] = college_data[i + 1];
            college_count--;
        }
        write_college_csv(csv_path, college_data, college_count);
    }
    refresh_treeview();
}

/* Add Student Dialog */
void on_add_student_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Add Student",
                                                    GTK_WINDOW(user_data),
                                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "_OK", GTK_RESPONSE_OK,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), grid);

    const char *labels[] = {"Name", "12th %", "JEE Rank", "Age", "Gender", "Aadhaar", "Email", "Phone"};
    GtkWidget *entries[8];
    for (int i = 0; i < 8; i++)
    {
        GtkWidget *label = gtk_label_new(labels[i]);
        entries[i] = gtk_entry_new();
        gtk_grid_attach(GTK_GRID(grid), label, 0, i, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), entries[i], 1, i, 1, 1);
    }

    gtk_widget_show_all(dialog);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
    {
        Student s;
        strncpy(s.name, gtk_entry_get_text(GTK_ENTRY(entries[0])), MAX_STR - 1);
        s.percentage12 = atof(gtk_entry_get_text(GTK_ENTRY(entries[1])));
        s.jee_rank = atoi(gtk_entry_get_text(GTK_ENTRY(entries[2])));
        s.age = atoi(gtk_entry_get_text(GTK_ENTRY(entries[3])));
        strncpy(s.gender, gtk_entry_get_text(GTK_ENTRY(entries[4])), 15);
        strncpy(s.aadhaar, gtk_entry_get_text(GTK_ENTRY(entries[5])), 15);
        strncpy(s.email, gtk_entry_get_text(GTK_ENTRY(entries[6])), 63);
        strncpy(s.phone, gtk_entry_get_text(GTK_ENTRY(entries[7])), 15);
        s.pref_count = 0;
        s.priorities = NULL;

        int offer_count = 0;
        Offer *offers = build_offers(&s, college_data, college_count, &offer_count);
        root = heap_insert(root, s, offers, offer_count);
        refresh_treeview();
    }
    gtk_widget_destroy(dialog);
}

/* Allocate top offer for selected student */
void on_allocate_clicked(GtkButton *button, gpointer user_data)
{
    allocate_seat(root);
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    college_data = read_college_csv(csv_path, &college_count);
    if (!college_data)
    {
        g_print("Failed to read colleges.csv\n");
        return 1;
    }

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "College Seat Allocation");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    store = gtk_tree_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    const char *titles[] = {"Student", "Top Institute", "Top Course", "Status"};
    for (int i = 0; i < NUM_COLS; i++)
    {
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(titles[i], renderer, "text", i, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    }

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), treeview, TRUE, TRUE, 0);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *btn_add = gtk_button_new_with_label("Add Student");
    GtkWidget *btn_alloc = gtk_button_new_with_label("Allocate Top Seat");
    gtk_box_pack_start(GTK_BOX(hbox), btn_add, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), btn_alloc, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(window), vbox);

    g_signal_connect(btn_add, "clicked", G_CALLBACK(on_add_student_clicked), window);
    g_signal_connect(btn_alloc, "clicked", G_CALLBACK(on_allocate_clicked), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    free_heap(root);
    free(college_data);
    return 0;
}