CC := gcc
CFLAGS := `pkg-config --libs --cflags gtk+-2.0` -g -lws2_32 -mwindows

client.exe: client.o network.o
	$(CC) -o $@ $^ $(CFLAGS) 

client.o: client.c
	$(CC) -o $@ $^ $(CFLAGS) -c

network.o: network.c
	$(CC) -o $@ $^ $(CFLAGS) -c

clean:
	-rm client.exe client.o network.o

all: client.exe
