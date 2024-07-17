
CFLAGS_INC = -I RyanJson
CFLAGS_INC +=  -I cJSON
CFLAGS_INC +=  -I yyjson
CFLAGS_INC +=  -I valloc
CFLAGS_INC +=  -I RyanJsonExample


src = $(wildcard ./RyanJson/*.c)
src += $(wildcard ./cJSON/*.c)
src += $(wildcard ./yyjson/*.c)
src += $(wildcard ./valloc/*.c)
src += $(wildcard ./RyanJsonExample/*.c)

obj = $(patsubst %.c, %.o, $(src))
target = app.o
CC = gcc

$(target): $(obj)
	$(CC) $(CFLAGS_INC) $(obj) -o $(target) -lm

%.o: %.c
	$(CC) $(CFLAGS_INC) -c $< -o $@ -lm

.PHONY: clean
clean:
	rm -rf $(obj) $(target)
