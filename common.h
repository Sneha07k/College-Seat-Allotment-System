#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>

#define MAX_STR 128

typedef struct {
    char Institute[MAX_STR];
    char Quota[MAX_STR];
    char Gender[MAX_STR];
    int Year;
    char Academic[MAX_STR];
    int Closing_R;
    int Opening_R;
    char Seat_Type[MAX_STR];
    int Seats_left;
} CollegeRow;

typedef struct {
    char name[MAX_STR];
    float percentage12;
    int jee_rank;
    int age;
    char gender[16];
    char aadhaar[32];
    char email[64];
    char phone[16];
    char exam_number[32];
    char reservation[32];
    char **priorities; 
    int pref_count;
} Student;

typedef enum { OFFER_PENDING, OFFER_FLOATED, OFFER_FROZEN, OFFER_SLID } OfferStatus;

typedef struct {
    int idx; 
    CollegeRow row;
    int prefScore;
    OfferStatus status;
} Offer;

typedef struct HeapNode {
    Student student;
    Offer *offers;
    int offer_count;
    struct HeapNode *left, *right;
} HeapNode;


CollegeRow* read_college_csv(const char *path, int *out_count);
int write_college_csv(const char *path, CollegeRow *rows, int count);


int append_allocation_txt(const char *path, const char *aadhaar, const char *student_name, const Offer *o);

int user_signup(const char *users_txt, const char *name, const char *aadhaar, const char *password);
int user_login(const char *users_txt, const char *aadhaar, const char *password);

HeapNode* heap_insert(HeapNode *root, Student s, Offer *offers, int offer_count);
HeapNode* heap_pop(HeapNode **root);
void free_heap(HeapNode *root);

int is_eligible(const Student *s, const CollegeRow *r);
int compute_prefscore(const Student *s, const CollegeRow *r);
Offer* build_offers(const Student *s, CollegeRow *rows, int nrows, int *out_offers);
void free_student(Student *s);

#endif
