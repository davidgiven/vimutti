vimutti: main.o
	gcc -Os -g -o $@ $<

%.o: %.c
	gcc -Os -g -c -o $@ $<

