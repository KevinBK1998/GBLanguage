copy: TEMP.gb
	cp TEMP.gb ~/GBemu || echo `echo "\nFAILED to copy\n" && rm ~/GBemu/TEMP.gb`
TEMP.gb: GBAssembler.py TEMP.gsm library.gb
	python3 GBAssembler.py
TEMP.gsm: compiler.exe GBPL/work.gbpl
	./compiler.exe GBPL/work.gbpl
library.gb: GBAssembler.py library.gsm
	python3 GBAssembler.py library
compiler.exe: y.tab.c lex.yy.c GBCompiler.cpp ASTree.cpp GSTable.cpp
	g++ y.tab.c lex.yy.c -o compiler.exe
lex.yy.c: tokenizer.l ASTree.h
	lex $<
y.tab.c: parser.y ASTree.h GSTable.h DS.h GBCompiler.h
	yacc -d $<
clean:
	rm -f TEMP.gb library.gb TEMP.gsm
.PHONY: reload
reload:
	make