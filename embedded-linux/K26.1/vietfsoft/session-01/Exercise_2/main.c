#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

typedef struct {
    int    id;
    char   name[64];
    int    quantity;
    double price;
} Product;

static int fd = -1;

int total_records() {
    off_t size = lseek(fd, 0, SEEK_END);
    if (size < 0) { perror("lseek"); return -1; }
    return (int)(size / sizeof(Product));
}

void add_product() {
    Product p;
    printf("Enter ID: ");
    scanf("%d", &p.id);
    printf("Enter name: ");
    scanf("%63s", p.name);
    printf("Enter quantity: ");
    scanf("%d", &p.quantity);
    printf("Enter price: ");
    scanf("%lf", &p.price);

    if (lseek(fd, 0, SEEK_END) < 0) { perror("lseek"); return; }
    if (write(fd, &p, sizeof(Product)) != sizeof(Product))
        perror("write");
    else
        printf("Product added.\n");
}

void show_by_index() {
    int n = total_records();
    printf("Enter index (0 to %d): ", n - 1);
    int idx;
    scanf("%d", &idx);
    if (idx < 0 || idx >= n) { printf("Index out of range.\n"); return; }

    off_t offset = (off_t)idx * sizeof(Product);
    if (lseek(fd, offset, SEEK_SET) < 0) { perror("lseek"); return; }

    Product p;
    if (read(fd, &p, sizeof(Product)) != sizeof(Product)) { perror("read"); return; }
    printf("Index %d: ID=%d Name=%s Quantity=%d Price=%.2f\n",
           idx, p.id, p.name, p.quantity, p.price);
}

void update_quantity() {
    int n = total_records();
    printf("Enter index (0 to %d): ", n - 1);
    int idx;
    scanf("%d", &idx);
    if (idx < 0 || idx >= n) { printf("Index out of range.\n"); return; }

    int new_qty;
    printf("Enter new quantity: ");
    scanf("%d", &new_qty);

    // Seek directly to the quantity field of that record
    off_t field_offset = (off_t)idx * sizeof(Product) + offsetof(Product, quantity);
    if (lseek(fd, field_offset, SEEK_SET) < 0) { perror("lseek"); return; }
    if (write(fd, &new_qty, sizeof(int)) != sizeof(int))
        perror("write");
    else
        printf("Quantity updated.\n");
}

void list_all() {
    if (lseek(fd, 0, SEEK_SET) < 0) { perror("lseek"); return; }
    Product p;
    int idx = 0;
    printf("\n%-5s %-6s %-20s %-10s %-10s\n", "Idx", "ID", "Name", "Quantity", "Price");
    printf("--------------------------------------------------\n");
    while (read(fd, &p, sizeof(Product)) == sizeof(Product)) {
        printf("%-5d %-6d %-20s %-10d %.2f\n", idx++, p.id, p.name, p.quantity, p.price);
    }
    if (idx == 0) printf("No products found.\n");
}

int main() {
    fd = open("products.dat", O_RDWR | O_CREAT, 0644);
    if (fd < 0) { perror("open"); exit(1); }

    int choice;
    while (1) {
        printf("\n1. Add product\n2. Show product by index\n3. Update quantity by index\n4. List all products\n5. Exit\n> ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: add_product();    break;
            case 2: show_by_index();  break;
            case 3: update_quantity(); break;
            case 4: list_all();       break;
            case 5:
                close(fd);
                printf("Goodbye.\n");
                exit(0);
            default:
                printf("Invalid choice.\n");
        }
    }
}
