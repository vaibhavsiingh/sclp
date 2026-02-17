CC := gcc
CFLAGS := -Wall -Wextra -g
LEX := lex
YACC := yacc

TARGET := compiler

SRCS := ast.c ast_print.c symbol_table.c main.c y.tab.c lex.yy.c
OBJS := $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lfl

y.tab.c y.tab.h: parser.y
	$(YACC) -d -y parser.y

lex.yy.c: scanner.l y.tab.h
	$(LEX) scanner.l

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) $(OBJS) y.tab.c y.tab.h lex.yy.c

.PHONY: all clean
