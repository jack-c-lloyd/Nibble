CC = gcc
CFLAGS = -std=c23

SRC = $(wildcard *.c)
OUT = nib

.PHONY: all delete help test

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC)

delete:
	rm -f $(OUT) $(wildcard *.exe)

help:
	@echo "usage: make [targets]"
	@echo "\ttargets:"
	@echo "\t\tall: build the program"
	@echo "\t\tdelete: remove the program"
	@echo "\t\thelp: display this usage message"
	@echo "\t\ttest: run the test cases"

test: $(OUT)
	echo 12 34 | ./$(OUT) -d examples/addition.nib
	echo  5  6 | ./$(OUT) -d examples/multiplication.nib
