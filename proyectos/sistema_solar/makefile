# ················
# · DEFINICIONES ·
# ················

# Compilador y opciones
CC=g++ -g
CFLAGS=--std=c++17
LDFLAGS=
EXECUTABLE_NAME=main

# Carpetas de salida
BIN=bin
OBJ=$(BIN)/obj
LIB_OUT=$(BIN)/lib
LDFLAGS:=$(LDFLAGS) -L$(LIB_OUT)

# Archivos cabecera (.h)
ROOT_DIR=../..
LIB=$(ROOT_DIR)/lib
INCLUDES= \
	-I. \
	-I$(ROOT_DIR) \
	-I$(LIB)/glad \
	-I$(LIB)/glm \
	-I$(LIB)/glfw/include \
	-I$(LIB)/glfw/include/GLFW \
	-I$(LIB)/imgui \
	-I$(LIB)/imgui/backends \
	-I$(LIB)/imguizmo/imGuIZMO.quat
HEADER_FILES=$(wildcard ./*.h) $(wildcard $(ROOT_DIR)/*.h)

# Archivos fuente (.cpp)
SRC=.
SOURCE_FILES=$(SRC)/main.cpp

# Archivos objecto (.o)
OBJECT_FILES=$(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SOURCE_FILES))

# Ejecutable
EXECUTABLE_FILES=$(BIN)/$(EXECUTABLE_NAME)

# Librerías (.a)
GLAD_LIB=$(LIB_OUT)/libglad.a
LDFLAGS:=$(LDFLAGS) -lglad

GLFW_LIB=$(LIB_OUT)/glfw/build/src/libglfw3.a
GLFW_CMAKE_FLAGS=
LDFLAGS:=$(LDFLAGS) -L$(LIB_OUT)/glfw/build/src -lglfw3
ifeq ($(shell uname), Darwin)
	LDFLAGS:=$(LDFLAGS) -framework Cocoa -framework IOKit -framework CoreVideo
else ifeq ($(shell uname), Linux)
	
else
	LDFLAGS:=$(LDFLAGS) -lgdi32 -lopengl32 -static
	GLFW_CMAKE_FLAGS:=$(GLFW_CMAKE_FLAGS) -G Ninja
endif

IMGUI_LIB=$(LIB_OUT)/libimgui.a
IMGUI_SOURCES=$(wildcard $(LIB)/imgui/*.cpp)
IMGUI_OBJECTS=$(patsubst $(LIB)/imgui/%.cpp, $(OBJ)/%.o, $(IMGUI_SOURCES))
IMGUI_BACKENDS=$(OBJ)/imgui_impl_glfw.o $(OBJ)/imgui_impl_opengl3.o
IMGUI_ZMO=$(OBJ)/imGuIZMOquat.o
LDFLAGS:=$(LDFLAGS) -limgui

# ··········
# · REGLAS ·
# ··········

# TODO: Copiar recursos (shaders, texturas, etc...)

# Build
build: $(EXECUTABLE_FILES)

# Clean
clean:
	@rm -rf $(OBJ)
clean-all: clean
	@rm -rf $(BIN)

# Construir ejecutable a partir de archivos de objecto (.o)
$(EXECUTABLE_FILES): $(OBJECT_FILES) $(GLAD_LIB) $(GLFW_LIB) $(IMGUI_LIB)
	@echo "generando ejecutable"
	@mkdir -p $(BIN)
	$(CC) $(INCLUDES) $^ -o $@ $(LDFLAGS)
	@echo "ejecutable generado en $@"

# Librería Glad
$(GLAD_LIB) : $(OBJ)/glad.o
	@echo "generando librería glad"
	@mkdir -p $(LIB_OUT)
	@ar rcs $@ $^
$(OBJ)/glad.o : $(LIB)/glad/glad.c
	@echo "compilando $<"
	@mkdir -p $(OBJ)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Librería Glfw (usa CMake)
$(GLFW_LIB):
	@echo "generando librería glfw"
	@mkdir -p $(LIB_OUT)/glfw/build
	@cmake -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF -S $(LIB)/glfw -B $(LIB_OUT)/glfw/build $(GLFW_CMAKE_FLAGS)
	@cmake --build $(LIB_OUT)/glfw/build

# Librería ImGui
$(IMGUI_LIB) : $(IMGUI_OBJECTS) $(IMGUI_BACKENDS) $(IMGUI_ZMO)
	@echo "generando librería imgui"
	@mkdir -p $(LIB_OUT)
	@ar rcs $@ $^
$(IMGUI_OBJECTS) : $(OBJ)/%.o : $(LIB)/imgui/%.cpp
	@echo "compilando $<"
	@mkdir -p $(OBJ)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
$(IMGUI_BACKENDS): $(OBJ)/%.o : $(LIB)/imgui/backends/%.cpp
	@echo "compilando $<"
	@mkdir -p $(OBJ)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
$(IMGUI_ZMO): $(OBJ)/%.o : $(LIB)/imguizmo/imGuIZMO.quat/%.cpp
	@echo "compilando $<"
	@mkdir -p $(OBJ)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ -DVGIZMO_USES_GLM -DIMGUIZMO_IMGUI_FOLDER=""

# Construir archivos de objecto (.o) usando archivos fuente (.cpp)
$(OBJECT_FILES): $(OBJ)/%.o : $(SRC)/%.cpp $(HEADER_FILES)
	@echo "compilando $<"
	@mkdir -p $(OBJ)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Para asegurarnos de que se pueden ejecutar aunque haya otro archivo con este nombre
.PHONY: build clean clean-all
