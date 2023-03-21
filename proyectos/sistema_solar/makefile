# ················
# · DEFINICIONES ·
# ················

# Compilador y opciones
CC=g++ -g
CFLAGS=--std=c++17
LDFLAGS=-L/usr/local/Cellar/glfw/3.3.8/lib -lglfw
EXECUTABLE_NAME=main

# Carpetas de salida
BIN=bin
OBJ=$(BIN)/obj
LIB=../../Librerias

# Archivos cabecera (.h)
HEADER_DIR=.
INCLUDES= \
	-I$(HEADER_DIR) \
	-I$(LIB)/tofu \
	-I$(LIB)/glad \
	-I$(LIB)/imgui \
	-I$(LIB)/imgui/backends \
	-I$(LIB)/imgui_zmo/imGuIZMO.quat \
	-I/usr/local/include \
	-I/usr/local/include/GLFW
HEADER_FILES=$(wildcard $(HEADER_DIR)/*.h) $(wildcard lib/tofu/*.h)

# Archivos fuente (.cpp)
SRC=.
SOURCE_FILES=$(SRC)/main.cpp

# Archivos objecto (.o)
OBJECT_FILES=$(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SOURCE_FILES))

# Ejecutable
EXECUTABLE_FILES=$(BIN)/$(EXECUTABLE_NAME)

# Librerías (.a)
GLAD_LIB=$(LIB)/glad/libglad.a
LDFLAGS:=$(LDFLAGS) -L$(LIB)/glad -lglad

IMGUI_LIB=$(LIB)/imgui/libimgui.a
IMGUI_SOURCES=$(wildcard $(LIB)/imgui/*.cpp)
IMGUI_OBJECTS=$(patsubst $(LIB)/imgui/%.cpp, $(OBJ)/%.o, $(IMGUI_SOURCES))
IMGUI_BACKENDS=$(OBJ)/imgui_impl_glfw.o $(OBJ)/imgui_impl_opengl3.o
IMGUI_ZMO=$(OBJ)/imGuIZMOquat.o
LDFLAGS:=$(LDFLAGS) -L$(LIB)/imgui -limgui

# ··········
# · REGLAS ·
# ··········

# Build
build: $(EXECUTABLE_FILES)

# Clean
clean:
	@rm -rf $(OBJ)
clean-all: clean
	@rm -rf $(BIN)

# Construir ejecutable a partir de archivos de objecto (.o)
$(EXECUTABLE_FILES): $(OBJECT_FILES) $(GLAD_LIB) $(IMGUI_LIB)
	@echo "Generando ejecutable"
	@mkdir -p $(BIN)
	@$(CC) $(LDFLAGS) $(INCLUDES) $^ -o $@
	@echo "Ejecutable generado en $@"

# Librería Glad
$(GLAD_LIB) : $(OBJ)/glad.o
	@echo "Generando librería $@"
	@mkdir -p $(LIB)
	@ar rcs $@ $^
$(OBJ)/glad.o : $(LIB)/glad/glad.c
	@echo "Compilando $<"
	@mkdir -p $(OBJ)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Librería ImGui
$(IMGUI_LIB) : $(IMGUI_OBJECTS) $(IMGUI_BACKENDS) $(IMGUI_ZMO)
	@echo "Generando librería $@"
	@mkdir -p $(LIB)
	@ar rcs $@ $^
$(IMGUI_OBJECTS) : $(OBJ)/%.o : $(LIB)/imgui/%.cpp
	@echo "Compilando $<"
	@mkdir -p $(OBJ)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
$(IMGUI_BACKENDS): $(OBJ)/%.o : $(LIB)/imgui/backends/%.cpp
	@echo "Compilando $<"
	@mkdir -p $(OBJ)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
$(IMGUI_ZMO): $(OBJ)/%.o : $(LIB)/imgui_zmo/imGuIZMO.quat/%.cpp
	@echo "Compilando $<"
	@mkdir -p $(OBJ)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ -DVGIZMO_USES_GLM -DIMGUIZMO_IMGUI_FOLDER=""

# Construir archivos de objecto (.o) usando archivos fuente (.cpp)
$(OBJECT_FILES): $(OBJ)/%.o : $(SRC)/%.cpp $(HEADER_FILES)
	@echo "Compilando $<"
	@mkdir -p $(OBJ)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Para asegurarnos de que se pueden ejecutar aunque haya otro archivo con este nombre
.PHONY: build clean clean-all
