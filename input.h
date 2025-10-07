#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LEN 50
#define MAX_CATEGORY_LEN 20
#define MAX_PREFS 20
#define MAX_STUDENTS 100
#define MAX_COLLEGES 50

typedef struct {
    char name[MAX_NAME_LEN];
    int rank;
    char category[MAX_CATEGORY_LEN];
    int preferences[MAX_PREFS];
    int prefCount;
    int allotted;
} Student;

typedef struct {
    char name[MAX_NAME_LEN];
    int totalSeats;
    int numCategories;
    char categories[MAX_CATEGORY_LEN][MAX_CATEGORY_LEN];
    int reservedSeats[MAX_CATEGORY_LEN];
    int seatsFilled[MAX_CATEGORY_LEN];
} College;


void readStudentData(const char *filename, Student students[], int *count);
void readCollegeData(const char *filename, College colleges[], int *count);
void initializeSeatMatrix(College colleges[], int count);
void validatePreferences(Student students[], int studentCount, int collegeCount);

#endif
