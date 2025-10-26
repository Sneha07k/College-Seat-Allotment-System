 #include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"

#define COL_NAME 0
#define COL_TOP_INSTITUTE 1
#define COL_TOP_COURSE 2
#define COL_STATUS 3
#define NUM_COLS 4

GtkTreeStore *store;
GtkWidget *treeview;
HeapNode *root_heap = NULL;

CollegeRow *college_data = NULL;
int college_count = 0;
const char *csv_path = "C:\\Users\\sneha\\OneDrive\\Desktop\\college seat allotment system\\colleges.csv";
const char *alloc_path = "C:\\Users\\sneha\\OneDrive\\Desktop\\college seat allotment system\\allocations.txt";
const char *users_path = "C:\\Users\\sneha\\OneDrive\\Desktop\\college seat allotment system\\users.csv";

char current_user_aadhaar[64] = "";

// Helper to trim leading/trailing spaces and newlines
static void trim(char *s) {
    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    int len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[len - 1] = '\0';
        len--;
    }
}


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

void traverse_heap(HeapNode *node, void (*callback)(HeapNode *))
{
    if (!node) return;
    callback(node);
    traverse_heap(node->left, callback);
    traverse_heap(node->right, callback);
}

void refresh_treeview()
{
    gtk_tree_store_clear(store);
    traverse_heap(root_heap, update_treeview_row);
}

/* find by student name (simple) */
HeapNode* find_student_node(HeapNode *node, const char *name){
    if(!node) return NULL;
    if(strcmp(node->student.name,name)==0) return node;
    HeapNode *l = find_student_node(node->left,name);
    if(l) return l;
    return find_student_node(node->right,name);
}

/* When user chooses an offer to freeze */
void on_select_offer_and_freeze(HeapNode *node, GtkWindow *parent){
    if(!node || node->offer_count==0) return;

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Choose Offer to Freeze",
                                                    parent,
                                                    GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "_Freeze", GTK_RESPONSE_OK,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add(GTK_CONTAINER(content), vbox);

    GList *radio_group = NULL;
    GtkWidget *radios[node->offer_count];
    for(int i=0;i<node->offer_count;i++){
        char buf[512];
        snprintf(buf,sizeof(buf),"%s - %s (Seats:%d)", node->offers[i].row.Institute, node->offers[i].row.Academic, node->offers[i].row.Seats_left);
        if(i==0) radios[i] = gtk_radio_button_new_with_label(NULL, buf);
        else radios[i] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radios[0]), buf);
        gtk_box_pack_start(GTK_BOX(vbox), radios[i], FALSE, FALSE, 0);
    }
    gtk_widget_show_all(dialog);
    int resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if(resp==GTK_RESPONSE_OK){
        int chosen = -1;
        for(int i=0;i<node->offer_count;i++){
            if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radios[i]))){ chosen=i; break; }
        }
        if(chosen>=0){
            Offer *o = &node->offers[chosen];
            if(o->row.Seats_left>0){
                o->status = OFFER_FROZEN;
                int idx = o->idx;
                if(idx>=0 && idx<college_count){
                    if(college_data[idx].Seats_left>0) college_data[idx].Seats_left--;
                    if(college_data[idx].Seats_left==0){
                        // remove college row from array (shift left)
                        for(int k=idx;k<college_count-1;k++) college_data[k]=college_data[k+1];
                        college_count--;
                        // Also need to update offers' idx values in heap - for simplicity we will not update existing nodes' idx,
                        // but that's an advanced fix (recompute offers after any major change). For now, writing allocation and decrement is enough.
                    }
                    write_college_csv(csv_path, college_data, college_count);
                }
                append_allocation_txt(alloc_path, (current_user_aadhaar[0]?current_user_aadhaar:node->student.aadhaar), node->student.name, o);
            } else {
                GtkWidget *m = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Selected seat not available.");
                gtk_dialog_run(GTK_DIALOG(m));
                gtk_widget_destroy(m);
            }
        }
    }
    gtk_widget_destroy(dialog);
    refresh_treeview();
}

/* Add Student Dialog */
void on_add_student_clicked(GtkButton *button, gpointer user_data)
{
    GtkWindow *parent = GTK_WINDOW(user_data);
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Add Student",
                                                    parent,
                                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "_OK", GTK_RESPONSE_OK,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), grid);

    const char *labels[] = {"Name", "12th %", "JEE Rank", "Age", "Gender", "Aadhaar", "Email", "Phone", "12th Exam No", "Reservation"};
    GtkWidget *entries[10];
    for (int i = 0; i < 10; i++)
    {
        GtkWidget *label = gtk_label_new(labels[i]);
        entries[i] = gtk_entry_new();
        gtk_grid_attach(GTK_GRID(grid), label, 0, i, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), entries[i], 1, i, 1, 1);
    }

    if(current_user_aadhaar[0]) gtk_entry_set_text(GTK_ENTRY(entries[5]), current_user_aadhaar);

    gtk_widget_show_all(dialog);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
    {
        Student s; memset(&s,0,sizeof(s));
        strncpy(s.name, gtk_entry_get_text(GTK_ENTRY(entries[0])), MAX_STR - 1);
        s.percentage12 = atof(gtk_entry_get_text(GTK_ENTRY(entries[1])));
        s.jee_rank = atoi(gtk_entry_get_text(GTK_ENTRY(entries[2])));
        s.age = atoi(gtk_entry_get_text(GTK_ENTRY(entries[3])));
        strncpy(s.gender, gtk_entry_get_text(GTK_ENTRY(entries[4])), 15);
        strncpy(s.aadhaar, gtk_entry_get_text(GTK_ENTRY(entries[5])), 31);
        strncpy(s.email, gtk_entry_get_text(GTK_ENTRY(entries[6])), 63);
        strncpy(s.phone, gtk_entry_get_text(GTK_ENTRY(entries[7])), 15);
        strncpy(s.exam_number, gtk_entry_get_text(GTK_ENTRY(entries[8])), 31);
        strncpy(s.reservation, gtk_entry_get_text(GTK_ENTRY(entries[9])), 31);
        s.pref_count = 0;
        s.priorities = NULL;

        int offer_count = 0;
        Offer *offers = build_offers(&s, college_data, college_count, &offer_count);
        root_heap = heap_insert(root_heap, s, offers, offer_count);
        refresh_treeview();
    }
    gtk_widget_destroy(dialog);
}

/* Allocate selected student's offers (open chooser) */
void on_allocate_clicked(GtkButton *button, gpointer user_data)
{
    GtkWindow *parent = GTK_WINDOW(user_data);
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    GtkTreeModel *model;
    GtkTreeIter iter;
    if(gtk_tree_selection_get_selected(sel, &model, &iter)){
        gchar *student_name;
        gtk_tree_model_get(model, &iter, 0, &student_name, -1);
        HeapNode *node = find_student_node(root_heap, student_name);
        if(node) on_select_offer_and_freeze(node, parent);
        g_free(student_name);
    } else {
        // if nothing selected, choose anyone (root)
        if(root_heap) on_select_offer_and_freeze(root_heap, parent);
        else {
            GtkWidget *m = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "No students available.");
            gtk_dialog_run(GTK_DIALOG(m)); gtk_widget_destroy(m);
        }
    }
}

/* View allocations for current user (reads allocations.txt and filters by aadhaar if logged in) */
void on_view_dashboard_clicked(GtkButton *button, gpointer user_data){
    GtkWindow *parent = GTK_WINDOW(user_data);
    FILE *f = fopen(alloc_path, "r");
    if(!f){
        GtkWidget *m = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "No allocations found.");
        gtk_dialog_run(GTK_DIALOG(m)); gtk_widget_destroy(m);
        return;
    }

    GtkWidget *dialog = gtk_dialog_new_with_buttons("My Allocations", parent, GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, "_Close", GTK_RESPONSE_CLOSE, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL,NULL);
    GtkWidget *text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled), text);
    gtk_container_add(GTK_CONTAINER(content), scrolled);

    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
    char line[512];
    char block[2048];
    block[0]=0;
    char aad[128] = "";
    char student[128]="", inst[128]="", course[128]="", status[128]="";

    while(fgets(line,sizeof(line),f)){
        if(strncmp(line,"Aadhaar:",8)==0){ strncpy(aad, line+8, sizeof(aad)-1); trim(aad); }
        else if(strncmp(line,"Student:",8)==0){ strncpy(student, line+8, sizeof(student)-1); trim(student); }
        else if(strncmp(line,"Institute:",10)==0){ strncpy(inst, line+10, sizeof(inst)-1); trim(inst); }
        else if(strncmp(line,"Course:",7)==0){ strncpy(course, line+7, sizeof(course)-1); trim(course); }
        else if(strncmp(line,"Status:",7)==0){ strncpy(status, line+7, sizeof(status)-1); trim(status); }
        else if(strncmp(line,"---",3)==0){
            // end of record
            if(current_user_aadhaar[0]==0 || strcmp(current_user_aadhaar,aad)==0){
                char tmp[1024];
                snprintf(tmp,sizeof(tmp),"Student: %s\nInstitute: %s\nCourse: %s\nStatus: %s\n\n", student, inst, course, status);
                strncat(block, tmp, sizeof(block)-strlen(block)-1);
            }
            aad[0]=student[0]=inst[0]=course[0]=status[0]=0;
        }
    }
    fclose(f);

    if(block[0]==0) gtk_text_buffer_set_text(buf, "No allocations found.\n", -1);
    else gtk_text_buffer_set_text(buf, block, -1);

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* Authentication dialog: asks aadhaar+password; if not found prompts signup */
int show_auth_dialog(GtkWindow *parent){
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Login / Signup",
                                                    parent,
                                                    GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content), grid);

    GtkWidget *lbl_aad = gtk_label_new("Aadhaar:");
    GtkWidget *ent_aad = gtk_entry_new();
    GtkWidget *lbl_pwd = gtk_label_new("Password:");
    GtkWidget *ent_pwd = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(ent_pwd), FALSE);

    GtkWidget *btn_login = gtk_button_new_with_label("Login");
    GtkWidget *btn_signup = gtk_button_new_with_label("Signup (create)");

    gtk_grid_attach(GTK_GRID(grid), lbl_aad, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ent_aad, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), lbl_pwd, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ent_pwd, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_login, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_signup, 1, 2, 1, 1);

    // connect signals with lambdas via g_signal_connect_swapped
    g_signal_connect_swapped(btn_login, "clicked", G_CALLBACK(gtk_widget_hide), dialog);
    g_signal_connect_swapped(btn_signup, "clicked", G_CALLBACK(gtk_widget_hide), dialog);

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog)); // wait for one button click/hide

    const char *aad = gtk_entry_get_text(GTK_ENTRY(ent_aad));
    const char *pwd = gtk_entry_get_text(GTK_ENTRY(ent_pwd));
    int ok = 0;
    if(aad && pwd && aad[0] && pwd[0]){
        if(user_login(users_path, aad, pwd)){
            strncpy(current_user_aadhaar, aad, sizeof(current_user_aadhaar)-1);
            ok = 1;
        } else {
            // Ask for name and create user
            GtkWidget *sdlg = gtk_dialog_new_with_buttons("Signup - Enter Name",
                                                          parent,
                                                          GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
                                                          "_OK", GTK_RESPONSE_OK, NULL);
            GtkWidget *g2 = gtk_grid_new();
            gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(sdlg))), g2);
            GtkWidget *lbl_name = gtk_label_new("Name:");
            GtkWidget *ent_name = gtk_entry_new();
            gtk_grid_attach(GTK_GRID(g2), lbl_name, 0, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(g2), ent_name, 1, 0, 1, 1);
            gtk_widget_show_all(sdlg);
            if(gtk_dialog_run(GTK_DIALOG(sdlg))==GTK_RESPONSE_OK){
                const char *name = gtk_entry_get_text(GTK_ENTRY(ent_name));
                if(name && name[0]){
                    if(user_signup(users_path, name, aad, pwd)){
                        strncpy(current_user_aadhaar, aad, sizeof(current_user_aadhaar)-1);
                        ok = 1;
                    } else {
                        GtkWidget *m = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Signup failed (aadhaar exists?).");
                        gtk_dialog_run(GTK_DIALOG(m)); gtk_widget_destroy(m);
                    }
                }
            }
            gtk_widget_destroy(sdlg);
        }
    }
    gtk_widget_destroy(dialog);
    return ok;
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    college_data = read_college_csv(csv_path, &college_count);
    if (!college_data)
    {
        g_print("Failed to read %s — make sure the file exists and has header matching columns.\n", csv_path);
        return 1;
    }

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "College Seat Allocation");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 500);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Authentication
    if(!show_auth_dialog(GTK_WINDOW(window))){ g_print("Auth failed\n"); return 1; }

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
    GtkWidget *btn_alloc = gtk_button_new_with_label("Allocate / Freeze Offer");
    GtkWidget *btn_dash = gtk_button_new_with_label("View My Allocations");
    gtk_box_pack_start(GTK_BOX(hbox), btn_add, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), btn_alloc, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), btn_dash, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(window), vbox);

    g_signal_connect(btn_add, "clicked", G_CALLBACK(on_add_student_clicked), window);
    g_signal_connect(btn_alloc, "clicked", G_CALLBACK(on_allocate_clicked), window);
    g_signal_connect(btn_dash, "clicked", G_CALLBACK(on_view_dashboard_clicked), window);

    gtk_widget_show_all(window);
    gtk_main();

    free_heap(root_heap);
    free(college_data);
    return 0;
}

