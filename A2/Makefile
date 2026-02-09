# Makefile for sclp

CC      = cc
YACC    = yacc
LEX     = lex

TARGET  = sclp
YACC_C  = parser.tab.c
YACC_H  = parser.tab.h
LEX_C   = lex.yy.c

all: $(TARGET)

$(TARGET): $(LEX_C) $(YACC_C)
	$(CC) $(LEX_C) $(YACC_C) -o $(TARGET)

$(YACC_C) $(YACC_H): parser.y
	$(YACC) -d parser.y

$(LEX_C): scanner.l $(YACC_H)
	$(LEX) scanner.l

clean:
	rm -f $(TARGET) $(YACC_C) $(YACC_H) $(LEX_C)
	find . -type f -name "*.toks" -delete
	
