CC=mpicc
ODIR=obj

_HELLO_OBJ = hello.o
HELLO_OBJ = $(patsubst %,$(ODIR)/%,$(_HELLO_OBJ))

_PROFILE_OBJ = mpi-profile.o
PROFILE_OBJ = $(patsubst %,$(ODIR)/%,$(_PROFILE_OBJ))

_TEST_OBJ = test.o
TEST_OBJ = $(patsubst %,$(ODIR)/%,$(_TEST_OBJ))

obj:
	mkdir -p $@

$(ODIR)/%.o: %.c | obj
	$(CC) -c -o $@ $<

hello: $(HELLO_OBJ)
	$(CC) -o $@ $^

profile: $(PROFILE_OBJ)
	$(CC) -o $@ $^

test: $(TEST_OBJ)
	$(CC) -o $@ $^

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o hello profile test
