all: i86pc/rootpool amd64/rootpool

i86pc/rootpool: BITS=32
amd64/rootpool: BITS=64

i86pc/rootpool amd64/rootpool: rootpool.c
	mkdir -p $(@D)
	gcc -Wall -m$(BITS) -o $@ $^ -lzfs -lnvpair
