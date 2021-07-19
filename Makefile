CC = clang
CFLAGS = -Wall -Wextra -O3 -I./src

.PHONY: all
all: bin/javap \
     bin/test_classfile

bin/javap: bin/classfile.o bin/file.o bin/verifier.o bin/javap.o
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
