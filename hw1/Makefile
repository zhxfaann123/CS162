EXECUTABLES=pthread words lwords pwords
CC=gcc
CFLAGS=-g -pthread -Wall -std=gnu99
LDFLAGS=-pthread

.PHONY: all clean

all: $(EXECUTABLES)

pthread: pthread.o
words: words.o word_helpers.o word_count.o
lwords: lwords.o word_count_l.o word_helpers.o list.o debug.o
pwords: pwords.o word_count_p.o word_helpers.o list.o debug.o

$(EXECUTABLES):
	$(CC) $(LDFLAGS) $^ -o $@

lwords.o: words.c
word_count_l.o: word_count_l.c
pwords.o: pwords.c
word_count_p.o: word_count_p.c

lwords.o word_count_l.o:
	$(CC) $(CFLAGS) -DPINTOS_LIST -c $< -o $@

pwords.o word_count_p.o:
	$(CC) $(CFLAGS) -DPINTOS_LIST -DPTHREADS -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXECUTABLES) *.o
