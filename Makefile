CC = clang
CFLAGS = -Wall -Wextra -O3 -I./src

.PHONY: all
all: bin/java \
     bin/test_classfile

bin/java: bin/classfile.o bin/file.o bin/java.o
	${CC} $(LDFLAGS) -o $@ $^

bin/test_classfile: bin/classfile.o bin/test_classfile.o
	${CC} $(LDFLAGS) -o $@ $^

bin/%.o: src/%.c
	${CC} ${CFLAGS} -c $< -o $@

bin/%.o: test/%.c
	${CC} ${CFLAGS} -c $< -o $@

.PHONY: clean
clean:
	rm -f ./bin/*
