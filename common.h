//common.h
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
    char aadhaar[16];
    char email[64];
    char phone[16];
    char **priorities; // preferred institutes
    int pref_count;
} Student;

typedef enum { OFFER_PENDING, OFFER_FLOATED, OFFER_FROZEN, OFFER_SLID } OfferStatus;

typedef struct {
    int idx; // index in college array
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

/* CSV input/output */
CollegeRow* read_college_csv(const char *path, int *out_count);
int write_college_csv(const char *path, CollegeRow *rows, int count);

/* Heap operations */
HeapNode* heap_insert(HeapNode *root, Student s, Offer *offers, int offer_count);
HeapNode* heap_pop(HeapNode **root);
void free_heap(HeapNode *root);

/* Allocation helpers */
int is_eligible(const Student *s, const CollegeRow *r);
int compute_prefscore(const Student *s, const CollegeRow *r);
Offer* build_offers(const Student *s, CollegeRow *rows, int nrows, int *out_offers);
void free_student(Student *s);

#endif
