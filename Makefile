
CFLAGS_INC = -I RyanJson
CFLAGS_INC +=  -I cJSON
CFLAGS_INC +=  -I yyjson
CFLAGS_INC +=  -I RyanJsonExample/valloc
CFLAGS_INC +=  -I RyanJsonExample


src = $(wildcard ./RyanJson/*.c)
src += $(wildcard ./cJSON/*.c)
src += $(wildcard ./yyjson/*.c)
src += $(wildcard ./RyanJsonExample/valloc/*.c)
src += $(wildcard ./RyanJsonExample/*.c)

obj = $(patsubst %.c, %.o, $(src))
target = app.o
CC = gcc
C_FLAGS = -Wall -Wextra -Wno-unused-parameter -Wformat=2

$(target): $(obj)
	$(CC) $(CFLAGS_INC) $(obj) $(C_FLAGS) -o $(target) -lm

%.o: %.c
	$(CC) $(CFLAGS_INC)  $(C_FLAGS) -c $< -o $@ -lm

.PHONY: clean
clean:
	rm -rf $(obj) $(target)
