#include "common.h"
#include <stdlib.h>
#include <string.h>


int is_eligible(const Student *s, const CollegeRow *r) {
    if(!r) return 0;
    if(r->Opening_R==0 && r->Closing_R==0) return 0;
    if(s->jee_rank >= r->Opening_R && s->jee_rank <= r->Closing_R){
        if(strcmp(r->Seat_Type,"OPEN")==0) return 1;
        if(strcmp(r->Seat_Type,s->gender)==0) return 1;
        if(strncmp(r->Seat_Type,"OPEN",4)==0 && strcmp(s->gender,"OPEN")==0) return 1;
    }
    return 0;
}


int compute_prefscore(const Student *s,const CollegeRow *r){
    if(s->pref_count>0 && s->priorities){
        for(int i=0;i<s->pref_count;i++)
            if(strcmp(s->priorities[i],r->Institute)==0) return i;
    }
    return 1000 + r->Closing_R;
}


Offer* build_offers(const Student *s, CollegeRow *rows,int nrows,int *out_offers){
    *out_offers=0;
    Offer *buf=malloc(sizeof(Offer)*nrows);
    int c=0;
    for(int i=0;i<nrows;i++){
        if(rows[i].Seats_left<=0) continue;
        if(is_eligible(s,&rows[i])){
            Offer o;
            o.idx=i;
            o.row=rows[i];
            o.prefScore=compute_prefscore(s,&rows[i]);
            o.status=OFFER_PENDING;
            buf[c++]=o;
        }
    }
    if(c==0){ free(buf); *out_offers=0; return NULL; }
    for(int i=0;i<c-1;i++)
        for(int j=i+1;j<c;j++)
            if((buf[i].prefScore>buf[j].prefScore) || (buf[i].prefScore==buf[j].prefScore && buf[i].row.Closing_R>buf[j].row.Closing_R)){
                Offer tmp=buf[i]; buf[i]=buf[j]; buf[j]=tmp;
            }
    *out_offers=c;
    return buf;
}


HeapNode* heap_insert(HeapNode *root, Student s, Offer *offers, int offer_count){
    HeapNode *node=malloc(sizeof(HeapNode));
    node->student=s;
    node->offers=offers;
    node->offer_count=offer_count;
    node->left=node->right=NULL;
    if(!root) return node;
    HeapNode **curr=&root;
    while(*curr){
        if(s.jee_rank<(*curr)->student.jee_rank) curr=&((*curr)->left);
        else curr=&((*curr)->right);
    }
    *curr=node;
    return root;
}


HeapNode* heap_pop(HeapNode **root){
    if(!root || !*root) return NULL;
    HeapNode *top=*root;
    if(top->right) *root=top->right;
    else *root=top->left;
    top->left=top->right=NULL;
    return top;
}

void free_heap(HeapNode *root){
    if(!root) return;
    free_heap(root->left);
    free_heap(root->right);
    if(root->offers) free(root->offers);
    free(root);
}


void free_student(Student *s){
    if(!s) return;
    if(s->priorities){
        for(int i=0;i<s->pref_count;i++)
            if(s->priorities[i]) free(s->priorities[i]);
        free(s->priorities);
        s->priorities=NULL;
    }
}
