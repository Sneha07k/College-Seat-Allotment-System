#include "output.h"
#include <stdio.h>
#include <stdlib.h>
#include //Name of final list

void displayStudentAllocations(StudentNode *head, int count)
 {
    StudentNode *p = head;
    int i = 0;
    printf("ID\tName\tCollege\tCategory\n");
    while (p != NULL && i < count) 
    {
        printf("%d\t%s\t%s\t%s\n",
               p->data.id,
               p->data.name,
               p->data.collegeName,
               p->data.category);
        p = p->next;
        i++;
    }
}
void displayCollegeAllocations(CollegeNode *head) 
{
    printf("College\tTotalSeats\tFilledSeats\n");
    CollegeNode *p = head;
    while (p != NULL) 
    {
        printf("%s\t%d\t%d\n",
               p->data.name,
               p->data.totalSeats,
               p->data.filledSeats);
        p = p->next;
    }
}
void displayCategoryWise(CollegeNode *head) 
{
    printf("Category-wise allocation (example)\n");
    CollegeNode *p = head;
    while (p != NULL) {
        printf("%s: Filled Seats = %d\n",
               p->data.name,
               p->data.filledSeats);
        p = p->next;
    }
}
void exportResultsToFile(StudentNode *head, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) 
    {
        perror("File open failed");
        return;
    }
    fprintf(fp, "ID,Name,College,Category\n");
    StudentNode *p = head;
    while (p != NULL) 
    {
        fprintf(fp, "%d,%s,%s,%s\n",
                p->data.id,
                p->data.name,
                p->data.collegeName,
                p->data.category);
        p = p->next;
    }

    fclose(fp);

}
