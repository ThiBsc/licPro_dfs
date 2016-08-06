CCOPTS=-Wall

dfs: conf.o dfs.o main.c
	gcc $(CCOPTS) -o dfs main.c conf.o dfs.o header/variables.h -lssl -lcrypto

conf.o: conf.c header/conf.h
	gcc $(CCOPTS) -c conf.c header/conf.h

dfs.o: dfs.c header/dfs.h
	gcc $(CCOPTS) -c dfs.c header/dfs.h