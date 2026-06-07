#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FILE_NAME "students.dat"

typedef struct {
    int   id;
    char  name[64];
    int   age;
    float gpa;
} Student;

void flush_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void add_student(int fd) {
    Student s;
    memset(&s, 0, sizeof(Student));

    printf("Enter Student ID: ");
    if (scanf("%d", &s.id) != 1) return;
    flush_stdin();

    printf("Enter Name: ");
    if (fgets(s.name, sizeof(s.name), stdin)) {
        s.name[strcspn(s.name, "\n")] = 0; // Strip newline
    }

    printf("Enter Age: ");
    if (scanf("%d", &s.age) != 1) return;

    printf("Enter GPA: ");
    if (scanf("%f", &s.gpa) != 1) return;
    flush_stdin();

    // Di chuyển về cuối file để append
    lseek(fd, 0, SEEK_END);
    if (write(fd, &s, sizeof(Student)) != sizeof(Student)) {
        perror("Error writing to file");
    } else {
        printf("Student added successfully.\n");
    }
}

void list_students(int fd) {
    Student s;
    lseek(fd, 0, SEEK_SET); // Quay về đầu file

    printf("\n--- Student List ---\n");
    printf("%-5s %-20s %-5s %-5s\n", "ID", "Name", "Age", "GPA");
    
    while (read(fd, &s, sizeof(Student)) == sizeof(Student)) {
        printf("%-5d %-20s %-5d %-5.2f\n", s.id, s.name, s.age, s.gpa);
    }
}

void find_student(int fd) {
    int search_id;
    Student s;
    int found = 0;

    printf("Enter Student ID to find: ");
    if (scanf("%d", &search_id) != 1) {
        flush_stdin();
        return;
    }
    flush_stdin();

    lseek(fd, 0, SEEK_SET);
    while (read(fd, &s, sizeof(Student)) == sizeof(Student)) {
        if (s.id == search_id) {
            printf("\nStudent Found:\n");
            printf("ID: %d\nName: %s\nAge: %d\nGPA: %.2f\n", s.id, s.name, s.age, s.gpa);
            found = 1;
            break;
        }
    }
    if (!found) {
        printf("Student with ID %d not found.\n", search_id);
    }
}

int main() {
    int fd = open(FILE_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    int choice;
    do {
        printf("\n=== Student Management Menu ===\n");
        printf("1. Add student\n");
        printf("2. List all students\n");
        printf("3. Find student by ID\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input!\n");
            flush_stdin();
            continue;
        }
        flush_stdin();

        switch (choice) {
            case 1: add_student(fd); break;
            case 2: list_students(fd); break;
            case 3: find_student(fd); break;
            case 4: printf("Exiting...\n"); break;
            default: printf("Invalid choice, try again.\n");
        }
    } while (choice != 4);

    close(fd);
    return EXIT_SUCCESS;
}