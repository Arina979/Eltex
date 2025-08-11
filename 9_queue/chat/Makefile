BIN1 := server
BIN2 := client

all: $(BIN1) $(BIN2)

$(BIN1): $(BIN1).c
		gcc $(BIN1).c -o $(BIN1) -lrt

$(BIN2): $(BIN2).c
		gcc $(BIN2).c -o $(BIN2) -lrt -pthread -lncurses

clean:
		rm $(BIN1) $(BIN2)