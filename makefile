UNAME_S := $(shell uname -s)

# Engine constats
# ----------------
#

# Resolution defaults to display size
ifndef EMERALD_SCREEN_WIDTH
	EMERALD_SCREEN_WIDTH = "SCREEN_WIDTH=$(shell ./utils/screen_res.bash width)"
endif
ifndef EMERALD_SCREEN_HEIGHT
	EMERALD_SCREEN_HEIGHT = "SCREEN_HEIGHT=$(shell ./utils/screen_res.bash height)"
endif

FRUSTUM_NEAR="_NEAR_=0.1f"
FRUSTUM_FAR="_FAR_=1000.f"
SSAO_N_SAMPLES="_SSAO_N_SAMPLES_=16"
MAX_LIGHTS="_MAX_LIGHTS_=25"
MAX_MODELS="_MAX_MODELS_=150"
ATT_QUAD="_ATT_QUAD_=0.05"
ATT_LIN="_ATT_LIN_=0.0005"
ATT_CON="_ATT_CON_=1.0"
SSAO_SCALE="_SSAO_SCALE_= 0.5"
SHADOW_SCALE="_SHADOW_SCALE_=4"
MAX_TERRAIN_MESHES="_MAX_TERRAIN_MESHES_=20000"
# ----------------

CC = g++
EXEC = emerald
CCFLAGS = -std=c++11 -DGL_GLEXT_PROTOTYPES -DGLEW_STATIC -DGLM_FORCE_RADIANS -D_DEFAULT_SCENE_FILE_="\"res/scenes/scene_file.txt\"" -D$(FRUSTUM_NEAR) -D$(FRUSTUM_FAR) -D$(EMERALD_SCREEN_HEIGHT) -D$(EMERALD_SCREEN_WIDTH) -D$(SSAO_N_SAMPLES) -D$(ATT_QUAD) -D$(ATT_LIN) -D$(ATT_CON) -D$(MAX_LIGHTS) -D$(MAX_MODELS) -D$(SSAO_SCALE) -D$(SHADOW_SCALE) -D$(MAX_TERRAIN_MESHES) #-DPROFILING -D_DEBUG_LOADER_
INC =
LDFLAGS = -lSDL2 -lSDL2_image -lassimp
FRAGMENT_SHADER_PRECOMPILE= $(MAX_LIGHTS) $(FRUSTUM_NEAR) $(FRUSTUM_FAR) $(EMERALD_SCREEN_WIDTH) $(EMERALD_SCREEN_HEIGHT) $(SSAO_N_SAMPLES) $(ATT_QUAD) $(ATT_LIN) $(ATT_CON)
GEOMETRY_SHADER_PRECOMPILE= $(MAX_MODELS)
VERTEX_SHADER_PRECOMPILE= $(MAX_LIGHTS)

ifeq ($(UNAME_S),Linux)
	LDFLAGS += /usr/local/lib/libAntTweakBar.so -lGLEW -lGL
	INC += -I/usr/local/include -L/usr/local/lib
endif

ifeq ($(UNAME_S),Darwin)
	CCFLAGS += -Qunused-arguments
	INC += -F/Library/Frameworks/ -I/usr/local/include -L/usr/local/lib -I/opt/local/include
	LDFLAGS += /usr/local/lib/libAntTweakBar.dylib -framework OpenGL -framework Cocoa
endif

# ------------------------
# Files we need

OBJS_DIR=build/objs/
SRC_FILES = $(wildcard src/*.cpp)
H_FILES = $(wildcard src/*.hpp)
OBJS_TMP = $(notdir $(SRC_FILES:.cpp=.o))
OBJS = $(addprefix $(OBJS_DIR),$(OBJS_TMP))

SHADER_FILES=$(wildcard src/shaders/*.vert) $(wildcard src/shaders/*.geom) $(wildcard src/shaders/*.frag)
FRAG_SETUP_SCRIPT="$(shell pwd)/src/setup_fragment_shader.bash"
GEOM_SETUP_SCRIPT="$(shell pwd)/src/setup_geometry_shader.bash"
VERT_SETUP_SCRIPT="$(shell pwd)/src/setup_vertex_shader.bash"

# ------------------------

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(INC) -o $(EXEC) $(LDFLAGS)

shaders: $(SHADER_FILES)
	$(FRAG_SETUP_SCRIPT) "$(FRAGMENT_SHADER_PRECOMPILE)"
	$(GEOM_SETUP_SCRIPT) "$(GEOMETRY_SHADER_PRECOMPILE)"
	$(VERT_SETUP_SCRIPT) "$(VERTEX_SHADER_PRECOMPILE)"

$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)

$(OBJS_DIR)%.o: src/%.cpp src/%.hpp | $(OBJS_DIR) shaders
	$(CC) $(CCFLAGS) $(INC) -c -o $@ $<

clean:
	rm -f $(EXEC) $(OBJS) *~ src/*~
	rm -rf build
