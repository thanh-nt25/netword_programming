#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

typedef struct student{
    char name[20];
    int eng;
    int math;
    int phys;
}STUDENT;

STUDENT data[]=
{
    {"Tuan", 82, 72, 58},
    {"Nam", 77, 82, 79},
    {"Khanh",52, 62, 39},
    {"Phuong", 61, 82, 88},
};
STUDENT *p;

int main(){
    //  count number of student
    int n = sizeof(data) / sizeof(data[0]); 
    
    // point to first element of data
    p = data;  

    for (int i = 0; i < n; i++) {
        printf("Student: %s, English: %d, Math: %d, Physics: %d\n", 
               (p + i)->name, (p + i)->eng, (p + i)->math, (p + i)->phys);
    }
    
    return 0;
}