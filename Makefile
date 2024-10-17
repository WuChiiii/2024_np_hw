CC = gcc
CFLAGS = -Wno-implicit-function-declaration -g
INC = -Iinclude

SRCS = $(wildcard src/*.c)

OBJS = $(patsubst src/%.c, obj/%.o, ${SRCS})

.PHONY: clean all

all: shell

shell: $(OBJS)
	$(CC) $^ -o ./bin/$@

obj/%.o: src/%.c
	$(CC) ${CFLAGS} ${INC} -c $^ -o $@

# or as following
# obj/%.o: $(SRCS)
#     $(CC) ${CFLAGS} ${INC} -c $(filter %/$(*.f).c,$^) -o $@

clean: 
	rm -f bin/* obj/*.o

# CC = gcc
# CFLAGS = -Wno-implicit-function-declaration -g
# INC = -Iinclude

# SRCS = $(wildcard test/*.c)

# OBJS = $(patsubst test/%.c, test_obj/%.o, ${SRCS})

# .PHONY: clean all

# all: shell

# shell: $(OBJS)
# 	$(CC) $^ -o ./bin/$@

# test_obj/%.o: test/%.c
# 	$(CC) ${CFLAGS} ${INC} -c $^ -o $@

# # or as following
# # obj/%.o: $(SRCS)
# #     $(CC) ${CFLAGS} ${INC} -c $(filter %/$(*.f).c,$^) -o $@

# clean: 
# 	rm -f bin/* test_obj/*.o