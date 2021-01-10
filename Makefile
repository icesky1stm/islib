### 基本参数定义
BIN  := islib.so

SRC  += islib.c
SRC  += istime.c
SRC  += isstr.c
SRC  += islog.c
SRC  += istcp.c
SRC  += isfile.c
SRC  += isipc.c
SRC  += isatom.c

# 定义基本路径
OBJDIR := obj
# makefile 查找路径
vpath %.c   src
vpath %.h   src
RM = rm -i

#LIBS    := -lm -lpthread
CFLAGS  += -Wall -std=c99 -fPIC

#CFLAGS  += -I$(OBJDIR)/include
LDFLAGS += -shared

.SUFFIXES: .c .so .o
### 操作系统特殊定义
#find the OS
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

ifeq ($(uname_S), Linux)

else

endif

OBJ  := $(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

.PHONY: all
all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	cp islib.so test/islib.so

.PHONY:test
test:
	cd ./test && make  && cd -

.PHONY: clean
clean:
	$(RM) -f $(BIN) obj/* test/$(BIN)
	@cd ./test && make clean && cd -

$(OBJDIR):
	@mkdir -p $@

$(OBJDIR)/%.o : %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<


