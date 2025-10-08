#include "allocator.h"


bool hasSeat(College *college, const char *category) {
    if (strcmp(category, "GEN") == 0) return college->generalSeats > 0;
    if (strcmp(category, "OBC") == 0) return college->obcSeats > 0;
    if (strcmp(category, "SC") == 0)  return college->scSeats > 0;
    if (strcmp(category, "ST") == 0)  return college->stSeats > 0;
    return false;
}



void heapify(Student students[], int n, int i) {
    int smallest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && students[left].rank < students[smallest].rank)
        smallest = left;
    if (right < n && students[right].rank < students[smallest].rank)
        smallest = right;

    if (smallest != i) {
        Student temp = students[i];
        students[i] = students[smallest];
        students[smallest] = temp;
        heapify(students, n, smallest);
    }
}


void buildHeap(Student students[], int n) {
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(students, n, i);
    }
}


Student extractTop(Student students[], int *n) {
    Student top = students[0];
    students[0] = students[*n - 1];
    (*n)--;
    heapify(students, *n, 0);
    return top;
}



bool allocateSeat(Student *student, College colleges[], int collegeCount) {
    for (int p = 0; p < student->prefCount; p++) {
        for (int c = 0; c < collegeCount; c++) {
            if (strcmp(colleges[c].name, student->preferences[p]) == 0 &&
                hasSeat(&colleges[c], student->category)) {

                strcpy(student->allottedCollege, colleges[c].name);
                student->isAllotted = true;
                updateSeatMatrix(student, colleges, collegeCount);
                return true;
            }
        }
    }
    return false;
}

void allocateSeats(Student students[], int studentCount, College colleges[], int collegeCount) {
    buildHeap(students, studentCount);

    Student result[MAX_STUDENTS];
    int resultCount = 0;

    int n = studentCount;
    while (n > 0) {
        Student top = extractTop(students, &n);
        allocateSeat(&top, colleges, collegeCount);
        result[resultCount++] = top;
    }

    // Copy back results
    for (int i = 0; i < resultCount; i++) {
        students[i] = result[i];
    }
}




}

