UNAME_S := $(shell uname -s)

CC = g++
EXEC = emerald
CCFLAGS = -std=c++11 -DGL_GLEXT_PROTOTYPES -DGLEW_STATIC -DGLM_FORCE_RADIANS -D_DEFAULT_SCENE_FILE_="\"scene_file.txt\""
INC =
LDFLAGS = -lSDL2 -lSDL2_image -lassimp
FRAGMENT_SHADER_PRECOMPILE= "_MAX_LIGHTS_=100" "_NEAR_=0.1" "_FAR_=100"
VERTEX_SHADER_PRECOMPILE=

ifeq ($(UNAME_S),Linux)
	LDFLAGS += -lGLEW -lGL
endif

ifeq ($(UNAME_S),Darwin)
	CCFLAGS += -Qunused-arguments
	INC += -F/Library/Frameworks/ -I/usr/local/include -L/usr/local/lib -I/opt/local/include
	LDFLAGS += -framework OpenGL -framework Cocoa
endif

# ------------------------
# Files we need

OBJS_DIR=build/objs/
SRC_FILES = $(wildcard src/*.cpp)
H_FILES = $(wildcard src/*.hpp)
OBJS_TMP = $(notdir $(SRC_FILES:.cpp=.o))
OBJS = $(addprefix $(OBJS_DIR),$(OBJS_TMP))

SHADER_FILES=$(wildcard src/shaders/*.vert) $(wildcard src/shaders/*.frag)
FRAG_SETUP_SCRIPT="$(shell pwd)/src/setup_fragment_shader.bash"
VERT_SETUP_SCRIPT="$(shell pwd)/src/setup_vertex_shader.bash"

# ------------------------

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(INC) -o $(EXEC) $(LDFLAGS)

shaders: $(SHADER_FILES)
	$(FRAG_SETUP_SCRIPT) "$(FRAGMENT_SHADER_PRECOMPILE)"
	$(VERT_SETUP_SCRIPT) $(VERTEX_SHADER_PRECOMPILE)

$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)

$(OBJS_DIR)%.o: src/%.cpp src/%.hpp | $(OBJS_DIR) shaders
	$(CC) $(CCFLAGS) $(INC) -c -o $@ $<

clean:
	rm -f $(EXEC) $(OBJS) *~ src/*~
	rm -rf build
