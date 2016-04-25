UNAME_S := $(shell uname -s)

CC = g++
EXEC = exec
CCFLAGS = -std=c++11 -DGL_GLEXT_PROTOTYPES -DGLEW_STATIC -DGLM_FORCE_RADIANS
INC =
LDFLAGS = -lSDL2 -lSDL2_image -lassimp
MAX_LIGHTS=100

ifeq ($(UNAME_S),Linux)
	LDFLAGS += -lGLEW -lGL
endif

ifeq ($(UNAME_S),Darwin)
	INC += -F/Library/Frameworks/ -I/usr/local/include -L/usr/local/lib -I/opt/local/include
	LDFLAGS += -framework OpenGL -framework Cocoa
endif

# ------------------------
# Files we need 

SRC_FILES = $(wildcard src/*.cpp)
H_FILES = $(wildcard src/*.hpp)
OBJS = $(SRC_FILES:.cpp=.o)
SHADER_SETUP_SCRIPT="$(shell pwd)/src/shader_setup.bash"

# ------------------------

all: $(EXEC) setup_shaders

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(INC) -o $(EXEC) $(LDFLAGS)

setup_shaders: $(SHADER_FILES)
	$(SHADER_SETUP_SCRIPT) $(MAX_LIGHTS)

%.o: %.cpp %.hpp
	$(CC) $(CCFLAGS) $(INC) -c -o $@ $<

clean:
	rm -f $(EXEC) $(OBJS) *~ src/*~ 
	rm -rf build
