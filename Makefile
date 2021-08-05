CC=gcc
CFLAGS = -std=c11 -Wall -pedantic -g

INCLUDES := $(shell pkg-config sdl2 --cflags --libs)
HEADERS := $(wildcard src/*.h src/*/*.h)
SOURCES := $(wildcard src/*.c src/*/*.c)
OBJS := $(SOURCES:src/%.c=obj/%.o)

LIB_OBJS := $(filter-out obj/main.o, $(SOURCES:src/%.c=obj/%.o) )

TESTS := $(wildcard test/*.c test/*/*.c)
TESTBINS := $(patsubst test/%.c, test/bin/%, $(TESTS))

.DEFAULT_GOAL:=
bin/nes: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDES)

$(OBJS): obj/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $< $(INCLUDES)

test/bin/%: test/%.c $(LIB_OBJS)
	$(CC) $(CFLAGS) $< $(LIB_OBJS) -o $@ -lcriterion

test/bin:
	mkdir $@

test: $(LIB_OBJS) $(TESTBINS)
	for test in $(TESTBINS); do ./$$test --verbose=; done


clean:
	$(RM) -r bin/* obj/* test/bin/*