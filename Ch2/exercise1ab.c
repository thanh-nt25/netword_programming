#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

struct student{
    char name[20];
    int eng;
    int math;
    int phys;
    double mean;
    char grade[2];
};

static struct student data[]=
{
    {"Tuan", 82, 72, 58, 0.0, ""},
    {"Nam", 77, 82, 79, 0.0, ""},
    {"Khanh",52, 62, 39, 0.0, ""},
    {"Phuong", 61, 82, 88, 0.0, ""},
    // {"Thanh", 100, 80, 90, 0.0, ""}
};


int main(){
    // Number of students
    int n = sizeof(data) / sizeof(data[0]); 

    // ============= 1a ============= //
    // Calculate the average for each student
    for (int i = 0; i < n; i++) {
        data[i].mean = (data[i].eng + data[i].math + data[i].phys) / 3.0;
    }

    // ============= 1b ============= //
    for(int i = 0; i < n; ++i){
        if (data[i].mean >= 90 && data[i].mean <= 100) {
            strcpy(data[i].grade, "S");
        } else if (data[i].mean >= 80 && data[i].mean < 90) {
            strcpy(data[i].grade, "A");
        } else if (data[i].mean >= 70 && data[i].mean < 80) {
            strcpy(data[i].grade, "B");
        } else if (data[i].mean >= 60 && data[i].mean < 70) {
            strcpy(data[i].grade, "C");
        } else if (data[i].mean >= 50 && data[i].mean < 60) {
            strcpy(data[i].grade, "D");
        } else {
            strcpy(data[i].grade, "F");
        }
    }

    // Output the results
    for (int i = 0; i < n; i++) {
        printf("Student: %s, Mean Score: %.2f, Grade: %s\n", data[i].name, data[i].mean, data[i].grade);
    }
    
    return 0;
}