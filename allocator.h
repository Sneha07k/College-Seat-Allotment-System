#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_NAME 50
#define MAX_PREF 10
#define MAX_COLLEGES 50
#define MAX_STUDENTS 100


typedef struct {
    char name[MAX_NAME];
    int totalSeats;
    int generalSeats;
    int obcSeats;
    int scSeats;
    int stSeats;
} College;

typedef struct {
    char name[MAX_NAME];
    int rank;
    char category[10];
    char preferences[MAX_PREF][MAX_NAME];
    int prefCount;
    char allottedCollege[MAX_NAME];
    bool isAllotted;
} Student;


void buildHeap(Student students[], int n);
void heapify(Student students[], int n, int i);
Student extractTop(Student students[], int *n);




#endif

