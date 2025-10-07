#include "input.h"

void readStudentData(const char *filename, Student students[], int *count) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: Could not open student file %s\n", filename);
        return;
    }

    fscanf(fp, "%d", count);
    for (int i = 0; i < *count; i++) {
        fscanf(fp, "%s %d %s %d",
               students[i].name,
               &students[i].rank,
               students[i].category,
               &students[i].prefCount);

        for (int j = 0; j < students[i].prefCount; j++) {
            fscanf(fp, "%d", &students[i].preferences[j]);
        }
        students[i].allotted = 0;
    }

    fclose(fp);
}


void readCollegeData(const char *filename, College colleges[], int *count) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: Could not open college file %s\n", filename);
        return;
    }

    fscanf(fp, "%d", count);
    for (int i = 0; i < *count; i++) {
        fscanf(fp, "%s %d %d",
               colleges[i].name,
               &colleges[i].totalSeats,
               &colleges[i].numCategories);

        for (int j = 0; j < colleges[i].numCategories; j++) {
            fscanf(fp, "%s %d",
                   colleges[i].categories[j],
                   &colleges[i].reservedSeats[j]);
            colleges[i].seatsFilled[j] = 0;
        }
    }

    fclose(fp);
}


void initializeSeatMatrix(College colleges[], int count) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < colleges[i].numCategories; j++) {
            colleges[i].seatsFilled[j] = 0;
        }
    }
}


void validatePreferences(Student students[], int studentCount, int collegeCount) {
    for (int i = 0; i < studentCount; i++) {
        Student *s = &students[i];
        int validPrefs[MAX_PREFS];
        int validCount = 0;

        for (int j = 0; j < s->prefCount; j++) {
            if (s->preferences[j] >= 0 && s->preferences[j] < collegeCount) {
                validPrefs[validCount++] = s->preferences[j];
            } else {
                printf("Warning: Invalid college preference for %s (index %d)\n",
                       s->name, s->preferences[j]);
            }
        }

        for (int k = 0; k < validCount; k++) {
            s->preferences[k] = validPrefs[k];
        }
        s->prefCount = validCount;
    }
}
