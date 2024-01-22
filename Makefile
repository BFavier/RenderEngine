#Name of the file to compile
OUT := bin/main.exe
#Path to parent makefiles
PARENT := 
#Compilation flags
CFLAGS := -Wall -Wextra -pedantic -std=c++17 -O0
#Linker flags
LFLAGS := 
#Libraries to link without the "lib" prefix and ".a" suffix (libExample.a = Example)
#LIB := assimp zlib irrXML glfw3 gdi32 opengl32 mingw32
LIB := glfw3 gdi32 vulkan-1
#DLLs to use (must be in the bin folder, ".dll" extension must be omitted)
DLL := 
#Directories in which to look for the headers
IDIR := include
#Directories in which to look for the libraries
LDIR := lib C:\VulkanSDK\1.2.189.2\Lib

#define a recursive wildcard function
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
#Querying cpp source files
SRC := $(call rwildcard,src,*.cpp *.c)
#Generating obj file names
OBJ := $(SRC:src/%.cpp=obj/%.o)
#Generating dependencies
DEP := $(SRC:src/%.cpp=obj/%.d)
#Searching for files "lib*.a" matching a LIB in the "lib" directory
LIB_FILES := $(foreach path,./lib,$(foreach file,$(LIB:%=lib%.a),$(wildcard $(path)/$(file))))
#Adding -l prefix
LIB := $(addprefix -l,$(LIB))
#Adding -I prefix
IDIR := $(addprefix -I,$(IDIR))
#Adding -L prefix
LDIR := $(addprefix -L,$(LDIR))
#Adding path and ".dll" extension to dlls
DLL := $(addprefix $(dir $(OUT)), $(DLL))
DLL := $(addsuffix .dll, $(DLL))

#Target that are not corresponding to real files
.PHONY: release debug clean makeParentsRelease makeParentsDebug cleanParents

#Release entry points
release: makeParentsRelease $(OUT)

#Debug entry points
debug: CFLAGS += -g
debug: makeParentsDebug $(OUT)

#Clean generated files
clean: cleanParents
	@del /S obj\*.d 2> nul
	@del /S obj\*.o 2> nul
	@if exist $(subst /,\,$(OUT)) del $(subst /,\,$(OUT)) 2> nul
	
#Call make for parent makefiles
makeParentsRelease:
	@set here=%cd% 
	$(foreach path,$(PARENT),cd $(path) && make)
	@cd %here%
	
#Call make debug for parent makefiles
makeParentsDebug:
	@set here=%cd% 
	$(foreach path,$(PARENT),cd $(path) && make debug)
	@cd %here%

#Call make clean for parent makefiles
cleanParents:
	@set here=%cd% 
	$(foreach path,$(PARENT),cd $(path) && make clean)
	@cd %here%

#Generate target
$(OUT): $(OBJ) $(LIB_FILES) $(DLL)
ifeq ($(suffix $(OUT)), .a)
	ar -rcs $(OUT) $(OBJ) $(DLL) 
else ifeq ($(suffix $(OUT)), .dll)
	g++ -shared -o $(OUT) $(OBJ) $(DLL) $(LDIR) $(LIB) $(LFLAGS) 
#-Wl,--out-implib,bin/lib$(notdir $(basename $(OUT))).a
else
	g++ -o $(OUT) $(OBJ) $(DLL) $(IDIR) $(LDIR) $(LIB) $(LFLAGS)
endif

#Generate object files and dependency files
obj/%.o: src/%.cpp
	-@(mkdir $(subst /,\,$(dir $@)) 2> nul) || (VER > nul)
	g++ -o $@ -c $< $(CFLAGS) $(IDIR) -MMD

#Code blocks alias
.PHONY: Release cleanRelease Debug cleanDebug
Release: release
cleanRelease: clean
Debug: debug
cleanDebug: clean

-include $(DEP)