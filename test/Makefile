### 基本参数定义
BIN  := \
main_islib_test \

OBJ  += islib_test
OBJ  += istime_test
OBJ  += isstr_test
OBJ  += istcp_test
OBJ  += islog_test
OBJ  += isfile_test
OBJ  += isipc_test
OBJ  += isatom_test

LIBS := islib.so

CFLAGS  += -Wall -std=c99 -g -I../src/

LDFLAGS += ../islib.so

.SUFFIXES: .c .so .o

.PHONY: all
all: $(BIN)

.PHONY: $(BIN)
$(BIN): $(OBJ)

.PHONY: clean
clean:
	rm -f *_test *_tmp

%.c:
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $<

