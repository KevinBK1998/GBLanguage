copy: TEMP.gb
	cp TEMP.gb ~/GBemu
TEMP.gb: GBAssembler.py TEMP.gsm library.gb
	python3 GBAssembler.py
TEMP.gsm: in2post.exe work.gbpl
	./in2post.exe work.gbpl
library.gb: GBAssembler.py library.gsm
	python3 GBAssembler.py library
in2post.exe: y.tab.c lex.yy.c GBCompiler.cpp exprtree.cpp
	g++ y.tab.c lex.yy.c -o in2post.exe
lex.yy.c: tokenizer.l
	lex $<
y.tab.c: parser.y
	yacc -d $<
clean:
	rm -f TEMP.gb library.gb TEMP.gsm
.PHONY: reload
reload:
	make