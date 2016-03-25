UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CCFLAGS = -lGLEW -lGL -lSDL2 -lassimp -lSOIL
endif

INC =
ifeq ($(UNAME_S),Darwin)
	INC += -F/Library/Frameworks/ -I/usr/local/include  $(sdl2-config --cflags --libs) -L/usr/local/lib -lassimp -I/opt/local/include -L/opt/local/lib -lSOIL
	CCFLAGS = -framework SDL2 -framework SDL2_image -framework OpenGL -framework	 Cocoa
endif

SRC_FILES = main.cpp Utils.cpp Model.cpp
H_FILES = Utils.hpp Model.hpp

all: main

main: $(SRC_FILES) $(H_FILES)
	g++ $(SRC_FILES) $(INC) $(CCFLAGS) -std=c++11 -o main


clean:
	rm main

