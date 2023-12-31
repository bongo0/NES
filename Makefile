CC=gcc
CFLAGS = -std=c11 -Wall -pedantic -Wextra -Wmaybe-uninitialized -Wno-unused-parameter -g

INCLUDES := $(shell pkg-config sdl2 glew --cflags --libs) -lSDL2_ttf -lm
HEADERS := $(wildcard src/*.h src/*/*.h)
SOURCES := $(wildcard src/*.c src/*/*.c)
OBJS := $(SOURCES:src/%.c=obj/%.o)

DEPS := $(wildcard deps/*.c deps/*/*.c)
DEPS_OBJS := $(DEPS:deps/%.c=deps/%.o)

LIB_OBJS := $(filter-out obj/main.o, $(SOURCES:src/%.c=obj/%.o) )

TESTS := $(wildcard test/*.c test/*/*.c)
TESTBINS := $(patsubst test/%.c, test/bin/%, $(TESTS))

.DEFAULT_GOAL:=
bin/nes: $(OBJS) $(DEPS_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDES)

$(OBJS): obj/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $< 

test/bin/%: test/%.c $(LIB_OBJS) $(DEPS_OBJS)
	$(CC) $(CFLAGS) $< $(LIB_OBJS) $(DEPS_OBJS) -o $@ $(INCLUDES) -lcriterion

test/bin:
	mkdir $@

test: $(LIB_OBJS) $(TESTBINS)
	for test in $(TESTBINS); do ./$$test --verbose=; done

$(DEPS_OBJS): deps/%.o: deps/%.c
	$(CC) $(CFLAGS) -c -o $@ $< $(INCLUDES)

clean:
	$(RM) -r bin/* obj/*.o obj/*/*.o test/bin/*