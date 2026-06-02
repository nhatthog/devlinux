#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    int   id;
    char  name[64];
    int   age;
    float gpa;
} Student;

static int fd = -1;

void add_student() {
    Student s;
    printf("Enter ID: ");
    scanf("%d", &s.id);
    printf("Enter name: ");
    scanf("%63s", s.name);
    printf("Enter age: ");
    scanf("%d", &s.age);
    printf("Enter GPA: ");
    scanf("%f", &s.gpa);

    if (lseek(fd, 0, SEEK_END) < 0) { perror("lseek"); return; }
    if (write(fd, &s, sizeof(Student)) != sizeof(Student))
        perror("write");
    else
        printf("Student added.\n");
}

void list_students() {
    Student s;
    int count = 0;
    if (lseek(fd, 0, SEEK_SET) < 0) { perror("lseek"); return; }
    printf("\n%-6s %-20s %-5s %-5s\n", "ID", "Name", "Age", "GPA");
    printf("--------------------------------------\n");
    while (read(fd, &s, sizeof(Student)) == sizeof(Student)) {
        printf("%-6d %-20s %-5d %.2f\n", s.id, s.name, s.age, s.gpa);
        count++;
    }
    if (count == 0) printf("No students found.\n");
}

void find_student() {
    int target_id;
    printf("Enter ID to find: ");
    scanf("%d", &target_id);

    Student s;
    if (lseek(fd, 0, SEEK_SET) < 0) { perror("lseek"); return; }
    while (read(fd, &s, sizeof(Student)) == sizeof(Student)) {
        if (s.id == target_id) {
            printf("Found: ID=%d Name=%s Age=%d GPA=%.2f\n",
                   s.id, s.name, s.age, s.gpa);
            return;
        }
    }
    printf("Student with ID %d not found.\n", target_id);
}

int main() {
    fd = open("students.dat", O_RDWR | O_CREAT, 0644);
    if (fd < 0) { perror("open"); exit(1); }

    int choice;
    while (1) {
        printf("\n1. Add student\n2. List all students\n3. Find student by ID\n4. Exit\n> ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: add_student();  break;
            case 2: list_students(); break;
            case 3: find_student();  break;
            case 4:
                close(fd);
                printf("Goodbye.\n");
                exit(0);
            default:
                printf("Invalid choice.\n");
        }
    }
}
