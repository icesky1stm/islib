### 基本参数定义
BIN  := \
islib_test \

OBJ  += istime_test
OBJ  += isstr_test

LIBS := islib.so aaaa
CFLAGS  += -Wall -std=c99 -I../src/

LDFLAGS += ../islib.so

.SUFFIXES: .c .so .o

.PHONY: all
all: $(BIN)

.PHONY: $(BIN)
$(BIN): $(OBJ)

.PHONY: clean
clean:
	rm -f *_test

%.c:
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $<

