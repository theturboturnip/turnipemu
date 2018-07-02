all: prog

PROJECT_NAME = turnipemu

BUILD_FOLDER = ./build
DEPENDS_FOLDER = $(BUILD_FOLDER)/depends
OBJECT_FOLDER = $(BUILD_FOLDER)/objects
HEADER_OBJECT_FOLDER = $(BUILD_FOLDER)/header_objects
SOURCE_FOLDER = ./src
SOURCE_HEADER_FOLDER = ./include
EXTERNAL_HEADER_FOLDER = ./external

LINK      = clang++
LINKFLAGS = -g -lGL -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
CPPFLAGS  = -g -Wall -O3 -I/usr/include/SDL2 -std=c++1z -I$(EXTERNAL_HEADER_FOLDER) -I$(SOURCE_HEADER_FOLDER) -I$(SOURCE_HEADER_FOLDER)/$(PROJECT_NAME)
CPP_HEADER_FLAGS = -Wno-pragma-once-outside-header

HEADER_FILES = $(shell find $(SOURCE_HEADER_FOLDER)/ -name "*.h")
HEADER_COMPILATION_OBJS = $(patsubst $(SOURCE_HEADER_FOLDER)/%.h, $(HEADER_OBJECT_FOLDER)/%.o, $(HEADER_FILES))
HEADER_DEPENDS = $(patsubst $(SOURCE_HEADER_FOLDER)/%.h, $(DEPENDS_FOLDER)/%_header.d, $(HEADER_FILES))

CPP_FILES = $(shell find $(SOURCE_FOLDER)/ -name "*.cpp")
OBJS = $(patsubst $(SOURCE_FOLDER)/%.cpp, $(OBJECT_FOLDER)/%.o, $(CPP_FILES))
DEPENDS = $(patsubst $(SOURCE_FOLDER)/%.cpp, $(DEPENDS_FOLDER)/%.d, $(CPP_FILES))

EXEC = run

CXX = clang++

-include $(DEPENDS)
-include $(HEADER_DEPENDS)

$(DEPENDS_FOLDER)/%_header.d : $(SOURCE_HEADER_FOLDER)/%.h
$(HEADER_OBJECT_FOLDER)/%.o : $(SOURCE_HEADER_FOLDER)/%.h
	@mkdir -p $(dir $(DEPENDS_FOLDER)/$*.d) $(dir $(HEADER_OBJECT_FOLDER)/$*.o)
	$(CXX) -MD -MF $(DEPENDS_FOLDER)/$*_header.d -c $(CPPFLAGS) $(CPP_HEADER_FLAGS) $(SOURCE_HEADER_FOLDER)/$*.h -o $(HEADER_OBJECT_FOLDER)/$*.o

$(DEPENDS_FOLDER)/%.d : $(SOURCE_FOLDER)/%.cpp
$(OBJECT_FOLDER)/%.o : $(SOURCE_FOLDER)/%.cpp
	@mkdir -p $(dir $(DEPENDS_FOLDER)/$*.d) $(dir $(OBJECT_FOLDER)/$*.o)
	$(CXX) -MD -MF $(DEPENDS_FOLDER)/$*.d -c $(CPPFLAGS) $(SOURCE_FOLDER)/$*.cpp -o $(OBJECT_FOLDER)/$*.o

prog: $(OBJS)
	$(LINK) $(OBJS) $(LINKFLAGS) -o $(EXEC)

rebuild: clean prog

run: prog
	./$(EXEC)
gdb: prog
	gdb ./$(EXEC) --eval-command=run
valgrind:
	valgrind ./$(EXEC)

test-individual: prog
	./run ./data/bios.gb ./data/cpu_instrs_test/individual/01-special.gb --no-limit
	./run ./data/bios.gb ./data/cpu_instrs_test/individual/02-interrupts.gb --no-limit
	./run ./data/bios.gb ./data/cpu_instrs_test/individual/03-op\ sp\,hl.gb --no-limit
	./run ./data/bios.gb ./data/cpu_instrs_test/individual/04-op\ r\,imm.gb --no-limit
	./run ./data/bios.gb ./data/cpu_instrs_test/individual/05-op\ rp.gb --no-limit
	./run ./data/bios.gb ./data/cpu_instrs_test/individual/06-ld\ r\,r.gb --no-limit
	./run ./data/bios.gb ./data/cpu_instrs_test/individual/07-jr,jp,call,ret,rst.gb --no-limit
	./run ./data/bios.gb ./data/cpu_instrs_test/individual/08-misc\ instrs.gb --no-limit
	./run ./data/bios.gb ./data/cpu_instrs_test/individual/09-op\ r\,r.gb --no-limit
	./run ./data/bios.gb ./data/cpu_instrs_test/individual/10-bit\ ops.gb --no-limit
	./run ./data/bios.gb ./data/cpu_instrs_test/individual/11-op\ a\,\(hl\).gb --no-limit
test-collated: prog
	./run ./data/bios.gb ./data/cpu_instrs_test/cpu_instrs.gb --no-limit

headers: $(HEADER_COMPILATION_OBJS)

clean:
	rm -rf *.o *.d $(BUILD_FOLDER) $(EXEC) 
