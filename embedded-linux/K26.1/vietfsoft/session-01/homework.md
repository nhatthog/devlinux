# Lab Exercises — File I/O · lseek · File Locking (fcntl & flock)

> **Language:** C &nbsp;|&nbsp; **OS:** Linux &nbsp;|&nbsp; **Syscalls only** — no stdio file functions

---

## Exercise 1 — Student Records with Basic File I/O

### Skills Practiced

`open` · `read` · `write` · `close` · binary file · struct serialization

### Overview

Build a program that manages a list of students stored in a **binary file**. All file operations must go through Linux system calls — the standard `fopen` / `fread` / `fwrite` family is not allowed.

### Record Format

Each student is represented by a fixed-size C struct:

```c
typedef struct {
    int   id;
    char  name[64];
    int   age;
    float gpa;
} Student;
```

### Menu

```
1. Add student
2. List all students
3. Find student by ID
4. Exit
```

### Requirements

| # | Behaviour |
|---|-----------|
| 1 | **Add** — collect fields from stdin, append one `Student` record to the file |
| 2 | **List** — read every record sequentially and print all fields |
| 3 | **Find** — scan records one by one; print the first whose `id` matches; report not found otherwise |
| 4 | **Exit** — close the file descriptor and terminate |

### Constraints

- Use **only**: `open`, `read`, `write`, `close`
- Do **not** use: `fopen`, `fread`, `fwrite`, `fprintf` (to the data file)
- Data file: **`students.dat`**
- Records must be stored in **binary format** (not text / CSV)
- The file must **persist** between runs — data added in one session must be readable in the next

### Acceptance Criteria

```bash
make
./main          # add 3 students, list all, find one by ID, exit
./main          # re-run — all 3 students must still be present
file students.dat   # must report: data (not ASCII text)
```

---

## Exercise 2 — Product Records with Random Access (`lseek`)

### Skills Practiced

`lseek` · random access · in-place field update · fixed-size records

### Overview

Build a product management program where any record can be **read or updated directly by index** using `lseek` — without loading the entire file into memory.

### Record Format

```c
typedef struct {
    int    id;
    char   name[64];
    int    quantity;
    double price;
} Product;
```

### Menu

```
1. Add product
2. Show product by index
3. Update quantity by index
4. List all products
5. Exit
```

### Requirements

| # | Behaviour |
|---|-----------|
| 1 | **Add** — append a new `Product` at the end of the file |
| 2 | **Show by index** — `lseek` directly to `index × sizeof(Product)`, read one record, print it |
| 3 | **Update quantity** — `lseek` to the exact byte offset of the `quantity` field inside the target record; write only that field — do not rewrite the whole record |
| 4 | **List all** — read and print every record sequentially |
| 5 | **Exit** |

### Constraints

- Use `lseek`, `read`, `write` for all file operations
- Updating a record must **seek directly** to the target offset — reading all records into memory and rewriting the whole file is not acceptable
- Data file: **`products.dat`**
- Every record must have a **fixed size** (`sizeof(Product)` must be constant across all records)
- The file persists between runs

### Key Formula

```c
// Byte offset of record at position [index]
off_t offset = (off_t)index * sizeof(Product);

// Byte offset of a specific field within a record
off_t field_offset = offset + offsetof(Product, quantity);
```

### Acceptance Criteria

```bash
make
./main      # add 4 products, show index 2, update qty of index 0, list all
./main      # re-run — updated quantity must still be there
```

Verify binary layout:

```bash
stat products.dat              # size must equal N × sizeof(Product)
od -A x -t x1z products.dat   # raw binary dump
```

---

## Exercise 3 — Multi-Process Log Writer with File Locking

### Skills Practiced

`flock` · `fcntl` write locks · `fork` · process synchronization · race condition prevention

### Overview

Simulate a multi-process logging system where several processes write to the **same log file concurrently**. Without locking, writes from different processes interleave and corrupt each other. Implement and compare two locking strategies: `flock` and `fcntl`.

### Log Line Format

Every entry written to **`system.log`** must follow this format:

```
[PID:12345] [2025-05-21 14:02:33] [INFO] your message here
```

### Program: `writer`

```bash
./writer_flock  "message text"
./writer_fcntl  "message text"
```

Each binary must:

1. Open `system.log` with `O_WRONLY | O_APPEND | O_CREAT`
2. Acquire an exclusive lock on the file
3. Format and write one log line (PID + timestamp + message)
4. Release the lock
5. Close the file and exit

---

### Part 1 — Locking with `flock`

```c
flock(fd, LOCK_EX);   // acquire exclusive lock — blocks until available
// ... write log line ...
flock(fd, LOCK_UN);   // release
```

Build target: **`writer_flock`**

---

### Part 2 — Locking with `fcntl`

```c
struct flock fl = {
    .l_type   = F_WRLCK,
    .l_whence = SEEK_SET,
    .l_start  = 0,
    .l_len    = 0,      // 0 = lock entire file
};
fcntl(fd, F_SETLKW, &fl);    // F_SETLKW = blocking acquire
// ... write log line ...
fl.l_type = F_UNLCK;
fcntl(fd, F_SETLK, &fl);     // release
```

Build target: **`writer_fcntl`**

---

### Makefile Targets

| Target        | Description                                        |
|---------------|----------------------------------------------------|
| `all`         | Build both `writer_flock` and `writer_fcntl`       |
| `test_flock`  | Launch 10 background `writer_flock` processes      |
| `test_fcntl`  | Launch 10 background `writer_fcntl` processes      |
| `clean`       | Remove all binaries and `system.log`               |

### Constraints

- Standard C + POSIX only — no third-party libraries
- Always use `O_APPEND` — never `lseek` to EOF before writing (not atomic)
- The lock must wrap the entire **format + write** sequence as one critical section
- Use `F_SETLKW` (blocking) in the `fcntl` version — `F_SETLK` (non-blocking) is not acceptable
- Both binaries must produce correct output when run **simultaneously from separate terminals**

### Acceptance Criteria

```bash
make

# --- flock version ---
rm -f system.log
for i in $(seq 1 10); do ./writer_flock "message $i" & done
wait
wc -l system.log            # must print: 10
grep -c "^\[PID" system.log # must print: 10

# --- fcntl version ---
rm -f system.log
for i in $(seq 1 10); do ./writer_fcntl "message $i" & done
wait
wc -l system.log            # must print: 10
grep -c "^\[PID" system.log # must print: 10
```

### Comparison Table

After completing both parts, fill in the following table as a comment block at the top of each source file:

| Property | `flock` | `fcntl` |
|---|---|---|
| Lock granularity | Whole file only | Byte range supported |
| Works over NFS | No | Yes |
| Inherited across `fork` | Yes (child gets a copy) | No (per open-file-description) |
| Automatically released on crash | Yes | Yes |
| Best used when | Simple local file locking | Network FS or byte-range locking |
