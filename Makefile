SRC = my_route_lookup.c io.c io.h utils.c utils.h 
CFLAGS = -g -Wall -O3

all: my_route_lookup

my_route_lookup: $(SRC)
	gcc $(CFLAGS) $(SRC) -o my_route_lookup -lm

.PHONY: clean

clean:
	rm -f my_route_lookup

#RL Lab 2020 Switching UC3M
