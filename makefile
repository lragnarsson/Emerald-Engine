UNAME_S := $(shell uname -s)

CC = g++
EXEC = exec
CCFLAGS = -std=c++11 -DGL_GLEXT_PROTOTYPES -DGLEW_STATIC -DGLM_FORCE_RADIANS -D_DEFAULT_SCENE_FILE_="\"scene_file.txt\""
INC =
LDFLAGS = -lSDL2 -lSDL2_image -lassimp
FRAGMENT_SHADER_PRECOMPILE= "MAX_LIGHTS=100"
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
FRAG_SETUP_SCRIPT="$(shell pwd)/src/setup_fragment_shader.bash"
VERT_SETUP_SCRIPT="$(shell pwd)/src/setup_vertex_shader.bash"

# ------------------------

all: $(EXEC) setup_shaders

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(INC) -o $(EXEC) $(LDFLAGS)

setup_shaders: $(SHADER_FILES)
	$(FRAG_SETUP_SCRIPT) "$(FRAGMENT_SHADER_PRECOMPILE)"
	$(VERT_SETUP_SCRIPT) $(VERTEX_SHADER_PRECOMPILE)


$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)

$(OBJS_DIR)%.o: src/%.cpp src/%.hpp | $(OBJS_DIR)
	$(CC) $(CCFLAGS) $(INC) -c -o $@ $<

clean:
	rm -f $(EXEC) $(OBJS) *~ src/*~
	rm -rf build
