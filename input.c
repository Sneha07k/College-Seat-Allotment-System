#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void trim(char *s) {
    char *p = s;
    while(*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p)+1);
    int l = strlen(s);
    while (l>0 && isspace((unsigned char)s[l-1])) { s[l-1]=0; l--; }
}

static char* my_strdup(const char *src) {
    char *dst = malloc(strlen(src)+1);
    if(dst) strcpy(dst, src);
    return dst;
}

static int find_index(char **cols, int colc, const char *name) {
    for(int i=0;i<colc;i++)
        if(strcmp(cols[i], name)==0) return i;
    return -1;
}

CollegeRow* read_college_csv(const char *path, int *out_count) {
    *out_count=0;
    FILE *f = fopen(path,"r");
    if(!f){ perror("open csv"); return NULL; }
    char line[4096];
    if(!fgets(line,sizeof(line),f)){ fclose(f); return NULL; }

    char *cols[64]; int colc=0;
    char *tok=strtok(line,",\n\r");
    while(tok && colc<64){ trim(tok); cols[colc++]=my_strdup(tok); tok=strtok(NULL,",\n\r"); }

    int idx_Institute = find_index(cols,colc,"Institute");
    int idx_Quota = find_index(cols,colc,"Quota");
    int idx_Gender = find_index(cols,colc,"Gender");
    int idx_Year = find_index(cols,colc,"Year");
    int idx_Academic = find_index(cols,colc,"Academic_Program_Name");
    int idx_Closing = find_index(cols,colc,"Closing_Rank");
    int idx_Opening = find_index(cols,colc,"Opening_Rank");
    int idx_SeatType = find_index(cols,colc,"Seat_Type");
    int idx_Seats = find_index(cols,colc,"Seats_left");

    int cap=256;
    CollegeRow *rows=malloc(sizeof(CollegeRow)*cap);
    int rc=0;

    while(fgets(line,sizeof(line),f)){
        if(strlen(line)<=1) continue;
        char *cells[64]; int cc=0;
        char *p=strtok(line,",\n\r");
        while(p && cc<64){ trim(p); cells[cc++]=p; p=strtok(NULL,",\n\r"); }

        CollegeRow r; memset(&r,0,sizeof(r));
        if(idx_Institute<cc) strncpy(r.Institute,cells[idx_Institute],MAX_STR-1);
        if(idx_Quota<cc) strncpy(r.Quota,cells[idx_Quota],MAX_STR-1);
        if(idx_Gender<cc) strncpy(r.Gender,cells[idx_Gender],MAX_STR-1);
        if(idx_Year<cc) r.Year=atoi(cells[idx_Year]);
        if(idx_Academic<cc) strncpy(r.Academic,cells[idx_Academic],MAX_STR-1);
        if(idx_Closing<cc) r.Closing_R=atoi(cells[idx_Closing]);
        if(idx_Opening<cc) r.Opening_R=atoi(cells[idx_Opening]);
        if(idx_SeatType<cc) strncpy(r.Seat_Type,cells[idx_SeatType],MAX_STR-1);
        if(idx_Seats<cc) r.Seats_left=atoi(cells[idx_Seats]);
        if(rc>=cap){ cap*=2; rows=realloc(rows,sizeof(CollegeRow)*cap); }
        rows[rc++]=r;
    }
    fclose(f);
    for(int i=0;i<colc;i++) free(cols[i]);
    *out_count=rc;
    return rows;
}

int write_college_csv(const char *path, CollegeRow *rows, int count) {
    FILE *f=fopen(path,"w");
    if(!f) return 0;
    fprintf(f,"Institute,Quota,Gender,Year,Academic_Program_Name,Closing_Rank,Opening_Rank,Seat_Type,Seats_left\n");
    for(int i=0;i<count;i++)
        fprintf(f,"%s,%s,%s,%d,%s,%d,%d,%s,%d\n",
            rows[i].Institute, rows[i].Quota, rows[i].Gender, rows[i].Year,
            rows[i].Academic, rows[i].Closing_R, rows[i].Opening_R,
            rows[i].Seat_Type, rows[i].Seats_left);
    fclose(f);
    return 1;
}
