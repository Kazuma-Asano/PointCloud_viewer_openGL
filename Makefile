CC = g++
LD = -L/usr/lib/x86_64-linux-gnu -lSDL2 -lGLEW -lGL
INC = -I/usr/include -D_REENTRANT

pc_viewer: pc_viewer.o
	$(CC) -o pc_viewer pc_viewer.o $(LD) $(INC)

pc_viewer.o: pc_viewer.cpp
	$(CC) -std=c++11 -c -o pc_viewer.o pc_viewer.cpp $(LD) $(INC)

clean:
	rm pc_viewer.o pc_viewer
