# programs to use
ASM			:= nasm
CC			:= gcc
LEX			:= flex
YACC		:= bison
# gcc flags
CFLAGS		:= -Wall
CPPFLAGS 	:= -DNDEBUG
# naming stuff
EXEC		:= bin/tpcc
ASM_NAME	:= _anonymous
LEX_NAME	:= lexer
YACC_NAME	:= parser
TAR_NAME	:= tpcc-source-code
# other
SOURCES		:= $(wildcard src/*.c)
OBJ     	:= obj/$(LEX_NAME).o obj/$(YACC_NAME).o $(SOURCES:src/%.c=obj/%.o)

######

.PHONY: all
all:
	mkdir -p bin/ obj/
	make $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

# Lexical analyser and parser

obj/$(LEX_NAME).o: obj/$(LEX_NAME).c obj/$(YACC_NAME).h inc/tree.h
	$(CC) -c $(CFLAGS) $< -o $@

obj/$(YACC_NAME).o: obj/$(YACC_NAME).c obj/$(YACC_NAME).h inc/tree.h
	$(CC) -c $(CFLAGS) $< -o $@

obj/$(LEX_NAME).c: src/$(LEX_NAME).l
	$(LEX) -o $@ $<

obj/$(YACC_NAME).c obj/$(YACC_NAME).h: src/$(YACC_NAME).y
	$(YACC) -d -Wcounterexamples $< -o obj/$(YACC_NAME).c

# C modules

obj/comp_check_decl.o: src/comp_check_decl.c inc/comp_check_decl.h inc/macros.h inc/symtable.h
obj/comp_check_types.o: src/comp_check_types.c inc/comp_check_types.h inc/macros.h inc/symfunc.h inc/types.h
obj/comp_init_symbols.o: src/comp_init_symbols.c inc/comp_init_symbols.h inc/comp_check_decl.h inc/hashtable.h inc/macros.h inc/symbol.h inc/symfunc.h inc/types.h
obj/comp_translate.o: src/comp_translate.c inc/comp_translate.h inc/macros.h inc/symbol.h inc/symfunc.h inc/symtable.h inc/types.h
obj/hashtable.o: src/hashtable.c inc/hashtable.h
obj/linkedlist.o: src/linkedlist.c inc/linkedlist.h
obj/main.o: src/main.c inc/comp_check_decl.h inc/comp_check_types.h inc/comp_init_symbols.h inc/comp_translate.h inc/macros.h inc/symfunc.h inc/symtable.h inc/tree.h obj/$(YACC_NAME).h
obj/symbol.o: src/symbol.c inc/symbol.h
obj/symfunc.o: src/symfunc.c inc/symfunc.h
obj/symtable.o: src/symtable.c inc/symtable.h inc/types.h
obj/tree.o: src/tree.c inc/tree.h
obj/types.o: src/types.c inc/types.h

obj/%.o: src/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

######

.PHONY: clean
clean:
	rm -rf bin/ obj/
	rm -f ./$(ASM_NAME).asm
	rm -f test/results.log

.PHONY: test
test: all
	chmod u+x test/run_tests.sh
	test/run_tests.sh

# tpcc --> asm --> executable
# .PHONY: asm
# asm: all
# 	$(EXEC) < test/good/gcd_rec_get.tpc
# 	$(ASM) -f elf64 -o obj/$(ASM_NAME).o ./$(ASM_NAME).asm
# 	$(CC) -o bin/$(ASM_NAME).out obj/$(ASM_NAME).o -nostartfiles -no-pie

.PHONY: tar
tar: clean
	tar -czvf ../$(TAR_NAME).tar.gz ./* --transform='s,./,$(TAR_NAME)/,'

# .PHONY: zip
# zip:
# 	rm -f ../export.zip
# 	mkdir -p ./export/
# 	cp -r ./inc ./export/inc/
# 	cp -r ./src ./export/src/
# 	cp makefile ./export/makefile
# 	zip -r --test --move export.zip ./export/
# 	mv ./export.zip ../export.zip