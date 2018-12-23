# Define required macros here
SHELL = /bin/sh
all : spd 
spd: spdServer.o 
	gcc -o spd spdServer.o 
spd.o : spdServer.o
	gcc -c spdServer.c
install :
	cp spd /usr/bin
clean:
	rm -f spdServer.o spd
uninstall: 
	rm -f /usr/bin/spd
