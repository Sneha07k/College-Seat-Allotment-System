#ifndef OUTPUT_H
#define OUTPUT_H
#include <stdio.h>
typedef struct {
    int id;
    char name[50];
    char collegeName[50];
    char category[20];
} Student;

typedef struct {
    char name[50];
    int totalSeats;
    int filledSeats;
} College;
void displayStudentAllocations(Student *students, int count);
void displayCollegeAllocations(College *colleges, int count);
void displayCategoryWise(College *colleges, int count);
void exportResultsToFile(Student *students, int count, char *filename);

#endif 
