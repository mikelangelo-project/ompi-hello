
GCC=gcc -ggdb
GCCSO=gcc -fPIC -shared -ggdb
MPICC=mpicc -ggdb
MPICCSO=mpicc -fPIC -shared -ggdb

EXEC=mpi_hello mpi_hello.so

all: $(EXEC)

clean:
	/bin/rm -f $(EXEC)

mpi_hello: mpi_hello.c
	$(MPICC) -o $@ $<

mpi_hello.so: mpi_hello.c
	$(MPICCSO) -o $@ $<
