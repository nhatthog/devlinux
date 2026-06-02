CC     = gcc
CFLAGS = -Wall -Wextra

all: writer_flock writer_fcntl

writer_flock: writer_flock.c
	$(CC) $(CFLAGS) -o writer_flock writer_flock.c

writer_fcntl: writer_fcntl.c
	$(CC) $(CFLAGS) -o writer_fcntl writer_fcntl.c

test_flock: writer_flock
	rm -f system.log
	for i in $$(seq 1 10); do ./writer_flock "message $$i" & done; wait
	@echo "--- wc -l ---"
	@wc -l system.log
	@echo "--- grep count ---"
	@grep -c "^\[PID" system.log

test_fcntl: writer_fcntl
	rm -f system.log
	for i in $$(seq 1 10); do ./writer_fcntl "message $$i" & done; wait
	@echo "--- wc -l ---"
	@wc -l system.log
	@echo "--- grep count ---"
	@grep -c "^\[PID" system.log

clean:
	rm -f writer_flock writer_fcntl system.log
