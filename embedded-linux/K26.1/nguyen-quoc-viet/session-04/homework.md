# Đề Bài — session-04

# Linux Process Exercises — fork, execve, wait

> Topics: Fork · Execve · Zombie Process · Wait / Waitpid
> Language: C | Level: Intermediate

---

## Exercise 1 — Multi-process Order Processing System

### Background

You are tasked with building an order processing system for a small store. Each order is handled by a separate worker running concurrently. The manager process (parent) tracks the result of each worker (child) after it finishes.

### Problem Statement

Write a single file `manager.c` that does the following:

1. Initialize an array of **3 hardcoded orders** inside `main()`.
2. Use `fork()` in a loop to spawn **3 child processes**, one per order.
3. The parent saves each child's PID into a `pids[]` array, then calls `waitpid()` to wait for **each child by its exact PID**.
4. After each `waitpid()`, the parent uses `WIFEXITED` and `WEXITSTATUS` to inspect the result.
5. At the end, the parent prints a **summary**: total orders, total revenue.

---

### Data Structures

```c
typedef struct {
    int   id;          /* Order ID                  */
    char  name[50];    /* Product name              */
    int   quantity;    /* Quantity ordered          */
    float unit_price;  /* Unit price (VND)          */
} Order;

/* Initialized in main() */
Order orders[3] = {
    {1, "Backpack", 2, 350000},
    {2, "Shoes",    1, 500000},
    {3, "Hat",      3, 120000}
};

pid_t pids[3];   /* Store child PIDs for ordered waitpid() calls */
```

**Required function (runs inside the child process):**

```c
void process_order(Order o) {
    float total = o.quantity * o.unit_price;
    printf("[CHILD-%d] PID: %d | PPID: %d\n", o.id, getpid(), getppid());
    printf("[CHILD-%d] %s x%d — Total: %.0f VND\n",
           o.id, o.name, o.quantity, total);
    printf("[CHILD-%d] Processing... (sleep 2s)\n\n", o.id);
    sleep(2);
}
```

---

### Execution Flow

```
main()
 │
 ├─ Loop 1: i = 0..2  (fork only — no waitpid here)
 │   └─ fflush(stdout)
 │   └─ fork()
 │       ├─ [child, pid == 0]
 │       │    process_order(orders[i])
 │       │    exit(0)               ← child terminates here
 │       │
 │       └─ [parent, pid > 0]
 │            pids[i] = pid         ← save for waitpid later
 │
 ├─ Loop 2: i = 0..2  (waitpid only — parent waits for each child)
 │   └─ waitpid(pids[i], &status, 0)
 │       ├─ WIFEXITED(status)       → check normal exit
 │       └─ WEXITSTATUS(status)     → get exit code (0 = success)
 │
 └─ Print summary (total orders, total revenue)
```

> **Key rule:** The two loops must be **separate**. If `fork()` and `waitpid()` are inside the same loop, the parent waits for each child before creating the next one — destroying concurrency and making the program effectively sequential.

> **stdout buffer note:** Call `fflush(stdout)` immediately before each `fork()`. When `fork()` duplicates the process, any data still in the stdio buffer is also copied. If the buffer is not flushed, the child will inherit and re-print unflushed output, causing duplicate lines.

---

### Expected Output

```
$ ./manager

===================================================
   ORDER PROCESSING SYSTEM — MANAGER (fork+wait)
===================================================
[MANAGER] PID: 1021 — spawning 3 child processes...

[MANAGER] fork() order #1 → child PID: 1022
[MANAGER] fork() order #2 → child PID: 1023
[MANAGER] fork() order #3 → child PID: 1024
[MANAGER] All 3 children spawned. Starting waitpid()...

--- [child output order may interleave — this is normal] ---

[CHILD-1] PID: 1022 | PPID: 1021
[CHILD-1] Backpack x2 — Total: 700000 VND
[CHILD-1] Processing... (sleep 2s)

[CHILD-2] PID: 1023 | PPID: 1021
[CHILD-2] Shoes x1 — Total: 500000 VND
[CHILD-2] Processing... (sleep 2s)

[CHILD-3] PID: 1024 | PPID: 1021
[CHILD-3] Hat x3 — Total: 360000 VND
[CHILD-3] Processing... (sleep 2s)

--- [~2 seconds later, all 3 children call exit(0)] ---

[MANAGER] waitpid(1022) — order #1: exit code=0 → SUCCESS
[MANAGER] waitpid(1023) — order #2: exit code=0 → SUCCESS
[MANAGER] waitpid(1024) — order #3: exit code=0 → SUCCESS

================= SUMMARY =================
  Total orders    : 3
  Successful      : 3
  Failed          : 0
  Total revenue   : 1,560,000 VND
===========================================
```

> **Observation:** The print order of the three children may vary on each run — this is *concurrent execution* in action, not a bug. The `waitpid()` section always prints in fixed order because the parent calls it with specific PIDs.

---
---

## Exercise 2 — Student Record Lookup via .txt File

### Background

The academic office needs a tool to look up student records from a data file. The system consists of **two separate programs**: a manager that receives search requests from the user, and a dedicated searcher that does the actual file lookup — launched via `fork` + `execve`.

### Problem Statement

Write **two files**: `manager.c` and `searcher.c`.

**`manager.c` (parent process):**
1. Read a student ID from `stdin` in a loop (enter `"quit"` to exit).
2. For each ID: `fork()` → child calls `execve("./searcher")` → parent calls `waitpid()`.
3. Read the child's exit code via `WIFEXITED` + `WEXITSTATUS` and print the appropriate message.
4. The line immediately after `execve()` must be `perror()` + `exit(2)`, with a comment explaining why this line is normally never reached.

**`searcher.c` (child process — launched by execve):**
1. Receive `argv[1]` = student ID to find, `argv[2]` = path to the data file.
2. Open `students.txt`, read line by line, use `strtok(line, "|")` to split fields.
3. If found: print full student info + grade classification, then `exit(0)`.
4. If not found: print a message, then `exit(1)`.
5. On file/argument error: call `perror()`, then `exit(2)`.

---

### Data Structures

**File `students.txt` — required format (pipe `|` as delimiter):**

```
SV001|Nguyen Van An|KTPM01|8.5
SV002|Tran Thi Bich|KTPM01|7.2
SV003|Le Van Cuong|HTTT02|9.1
SV004|Pham Thi Dung|HTTT02|6.8
SV005|Hoang Van Em|KHMT03|5.0
```

| Field  | Token index | Type    | Note                           |
|--------|-------------|---------|--------------------------------|
| ID     | 0           | `char[]`| Search key — compare with `argv[1]` |
| Name   | 1           | `char[]`|                                |
| Class  | 2           | `char[]`|                                |
| GPA    | 3           | `float` | Parse with `atof()`            |

**Grade classification:**

| GPA       | Grade       |
|-----------|-------------|
| ≥ 8.5     | Excellent   |
| ≥ 7.0     | Good        |
| ≥ 5.0     | Average     |
| < 5.0     | Poor        |

**`searcher` exit code contract (must be followed exactly):**

| Exit code | Meaning            | Action                              |
|-----------|--------------------|-------------------------------------|
| `0`       | Student found      | Print full record to `stdout`       |
| `1`       | Student not found  | Print "not found" message           |
| `2`       | Error (file/args)  | Print error to `stderr` via `perror()` |

---

### Execution Flow

```
manager: read student ID from stdin
 │
 └─ fork()
     ├─ [child, pid == 0]
     │    char *argv_exec[] = {"./searcher", student_id, "students.txt", NULL}
     │    execve("./searcher", argv_exec, environ)
     │    /* Only reached if execve() FAILS */
     │    perror("execve failed")
     │    exit(2)
     │
     └─ [parent, pid > 0]
          waitpid(pid, &status, 0)
          WEXITSTATUS == 0  →  "Found"
          WEXITSTATUS == 1  →  "Not found"
          WEXITSTATUS == 2  →  "Error"
          └─ repeat until user enters "quit"


searcher: launched by execve()
 │
 ├─ Print getpid(), getppid()     ← shows parent-child relationship
 ├─ Open students.txt
 ├─ Loop: fgets() each line
 │    strtok(line, "|") → extract 4 fields
 │    Compare field[0] with argv[1]
 │    Match found → print record → exit(0)
 └─ End of file with no match → exit(1)
```

> **Key point about `execve()`:** On success, the entire memory image of the child process — stack, heap, and text segment — is **completely replaced** by the `searcher` binary. Any code from `manager.c` that existed in the child is gone. This is why `searcher` must be a separate compiled executable.
> The process PID and PPID remain unchanged after `execve()` — only the image is replaced, not the process identity.

---

### Expected Output

```
$ ./manager

=============================================
   STUDENT LOOKUP SYSTEM — MANAGER
   (fork + execve | file: students.txt)
=============================================
[MANAGER] PID: 2040
Enter student ID ('quit' to exit).

---------------------------------------------
Student ID: SV003

[MANAGER] fork() → child PID: 2041
[MANAGER] Waiting for child (waitpid)...

[SEARCHER] PID: 2041 | PPID: 2040
[SEARCHER] execve() succeeded — old image has been replaced
[SEARCHER] Searching for "SV003" in students.txt...

========== SEARCH RESULT ==========
  ID      : SV003
  Name    : Le Van Cuong
  Class   : HTTT02
  GPA     : 9.1
  Grade   : Excellent
====================================

[MANAGER] Child (PID 2041) exited. code=0 → Found

---------------------------------------------
Student ID: SV999

[MANAGER] fork() → child PID: 2042
[MANAGER] Waiting for child (waitpid)...

[SEARCHER] PID: 2042 | PPID: 2040
[SEARCHER] Searching for "SV999" in students.txt...
[SEARCHER] No student found with ID: SV999

[MANAGER] Child (PID 2042) exited. code=1 → Not found

---------------------------------------------
Student ID: SV001

[MANAGER] fork() → child PID: 2043
[MANAGER] Waiting for child (waitpid)...

[SEARCHER] PID: 2043 | PPID: 2040
[SEARCHER] Searching for "SV001" in students.txt...

========== SEARCH RESULT ==========
  ID      : SV001
  Name    : Nguyen Van An
  Class   : KTPM01
  GPA     : 8.5
  Grade   : Excellent
====================================

[MANAGER] Child (PID 2043) exited. code=0 → Found

---------------------------------------------
Student ID: quit
[MANAGER] Exiting. Goodbye!
$
```

> **Observations:**
> - Child PID increments each lookup (2041 → 2042 → 2043): every search spawns a **brand new process**.
> - The PPID of `searcher` is always 2040: `execve()` replaces the image, not the parent-child relationship.
> - The manager distinguishes outcomes via `WEXITSTATUS` — not by parsing the child's stdout.
