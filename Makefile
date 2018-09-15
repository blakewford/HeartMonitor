CC=g++

sample: main.cpp
	$(CC) -I/usr/include/SDL2 -gdwarf-4 -std=c++11 -DPROGMEM= main.cpp -o $@ -I. -include port.h -Wno-narrowing -fpermissive -lSDL2

clean:
	rm sample
