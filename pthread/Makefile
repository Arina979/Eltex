BIN := main
SOURCES := *.c
HEADERS := 
CC := gcc
CFLAGS := -lpthread
LDFLAGS :=

$(BIN):	$(SOURCES)
		clear && $(CC) $(SOURCES) -o $(BIN) $(CFLAGS) $(LDFLAGS)
clean:
		rm -rf $(BIN)