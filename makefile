UNAME_S := $(shell uname -s)

CC = g++
EXEC = exec
CCFLAGS = -std=c++11 -DGL_GLEXT_PROTOTYPES -DGLEW_STATIC -DGLM_FORCE_RADIANS -D_DEFAULT_SCENE_FILE_="\"scene_file.txt\""
INC =
LDFLAGS = -lSDL2 -lSDL2_image -lassimp

ifeq ($(UNAME_S),Linux)
	LDFLAGS += -lGLEW -lGL
endif

ifeq ($(UNAME_S),Darwin)
	INC += -F/Library/Frameworks/ -I/usr/local/include -L/usr/local/lib -I/opt/local/include
	LDFLAGS += -framework OpenGL -framework Cocoa
endif

SRC_FILES = $(wildcard src/*.cpp)
H_FILES = $(wildcard src/*.hpp)
OBJS = $(SRC_FILES:.cpp=.o)

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(INC) -o $(EXEC) $(LDFLAGS)

%.o: %.cpp %.hpp
	$(CC) $(CCFLAGS) $(INC) -c -o $@ $<

clean:
	rm -f $(EXEC) $(OBJS) *~ src/*~
