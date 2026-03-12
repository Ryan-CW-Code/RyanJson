
# 编译器设置
CC = gcc
C_FLAGS = -std=gnu99 -O2 -Wall -Wextra -Wno-unused-parameter
C_FLAGS += -DRyanJsonProjectRootPath=\"$(shell pwd)\"

# 头文件包含目录
CFLAGS_INC = -I ./RyanJson
CFLAGS_INC += -I ./example
CFLAGS_INC += -I ./test
CFLAGS_INC += -I ./test/baseTest
CFLAGS_INC += -I ./test/externalModule/valloc
CFLAGS_INC += -I ./test/externalModule/tlsf
CFLAGS_INC += -I ./test/externalModule/cJSON
CFLAGS_INC += -I ./test/externalModule/yyjson

# 源文件扫描 (排除 fuzzer)
src = $(wildcard ./RyanJson/*.c)
src += $(wildcard ./example/*.c)
src += $(wildcard ./test/*.c)
src += $(wildcard ./test/baseTest/*.c)
src += $(wildcard ./test/externalModule/valloc/*.c)
src += $(wildcard ./test/externalModule/tlsf/*.c)
src += $(wildcard ./test/externalModule/cJSON/*.c)
src += $(wildcard ./test/externalModule/yyjson/*.c)

# 中间对象
obj = $(src:.c=.o)

# 目标程序 - 修改名字避免与源码文件夹 RyanJson 重名
target = app

# 默认规则
all: $(target)

$(target): $(obj)
	$(CC) $(obj) $(C_FLAGS) -o $(target) -lm

# 编译模式规则
%.o: %.c
	$(CC) $(CFLAGS_INC) $(C_FLAGS) -c $< -o $@

# 清理规则
.PHONY: clean
clean:
	rm -f $(obj) $(target)
