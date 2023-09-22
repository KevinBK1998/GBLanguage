run: TEMP.gb
	cp TEMP.gb ~/GBemu
TEMP.gb: TEMP.gsm
	python3 GBAssembler.py
TEMP.gsm: in2post.exe
	./in2post.exe work.gbpl
in2post.exe: y.tab.c lex.yy.c GBCompiler.c exprtree.c
	gcc y.tab.c lex.yy.c -o in2post.exe
lex.yy.c: tokenizer.l
	lex $<
y.tab.c: parser.y
	yacc -d $<
clean:
	rm -f TEMP.gb TEMP.gsm
.PHONY: reload
reload:
	make