BIN := main
SHARED_LIB := strerror
CC := gcc

all: $(SHARED_LIB) $(BIN)

$(SHARED_LIB): $(SHARED_LIB).c
		$(CC) -fPIC $(SHARED_LIB).c -c
		$(CC) -shared $(SHARED_LIB).o -o lib$(SHARED_LIB).so

$(BIN): $(BIN).c
		$(CC) $(BIN).c -o $(BIN) -L. -l$(SHARED_LIB) -Wl,-rpath,.

clean:
		rm $(BIN) $(SHARED_LIB).o lib$(SHARED_LIB).so