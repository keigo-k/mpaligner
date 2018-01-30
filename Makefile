CC = gcc
CFLAGS = -O2
LOADLIBES = -lm
OBJS = mpaligner.o HASH_func.o mpAlign.o toolOptions.o

mpaligner: $(OBJS)

clean:
	rm -f $(OBJS) mpaligner
