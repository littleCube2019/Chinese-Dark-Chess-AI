# object file folder
OBJ_FOLDER := bin
Src := src
# check OS
ifeq ($(OS), Windows_NT) # windows
	G_PLUS_PLUS := g++ -std=c++11 -static -O3 -Wall -g
	EXEC := hw1.exe
	DELETE := del /f $(OBJ_FOLDER)\*.o $(EXEC)
	MAKE_FOLDER := if not exist $(OBJ_FOLDER) mkdir $(OBJ_FOLDER)
	OS_DEFINE := -DWINDOWS
else
	ifeq ($(shell uname), Darwin) # macOS
		G_PLUS_PLUS := g++ -std=c++11 -O3 -Wall -g
		OS_DEFINE := -DMACOS
	else # linux
		G_PLUS_PLUS := g++ -std=c++11 -static -O3 -Wall -g
		OS_DEFINE := -DLINUX
	endif
	EXEC := exec.hw1
	DELETE := rm -f $(OBJ_FOLDER)/*.o $(EXEC)
	MAKE_FOLDER := mkdir -p $(OBJ_FOLDER)
endif

# specify files
MAIN := main
MAIN_FILE := $(Src)/$(MAIN).cpp
MAIN_OBJ := $(OBJ_FOLDER)/$(MAIN).o

AI := MyAI
AI_FILE := $(Src)/$(AI).cpp
AI_HEADER := $(Src)/$(AI).h
AI_OBJ := $(OBJ_FOLDER)/$(AI).o

# command
.PHONY: all
all: hw1

$(AI_OBJ) : $(AI_FILE) $(AI_HEADER) 
	$(MAKE_FOLDER)
	$(G_PLUS_PLUS) -c $(AI_FILE) $(OS_DEFINE) -o $(AI_OBJ)

$(EXEC) : $(AI_OBJ) $(MAIN_FILE)
	$(MAKE_FOLDER)
	$(G_PLUS_PLUS) -c $(MAIN_FILE) $(OS_DEFINE) -o $(MAIN_OBJ)
	$(G_PLUS_PLUS) $(AI_OBJ) $(MAIN_OBJ) -o $(EXEC)

# target
hw1 : $(EXEC)

# clean object files and executable file
.PHONY: clean
clean: 
	$(DELETE)
