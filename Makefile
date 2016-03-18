ifeq ($(OS),Windows_NT) # If windows
	LIB_ROOT = C:/dev/lib/x86
	SDL_LFLAGS = -L$(LIB_ROOT) -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image
	CC = gcc
	STD = c99
	POST_BUILD = .\copy_dll_32.bat
	LFLAGS =
	INCLUDE_FLAGS = -IC:/dev/include
else
	SDL_LFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf
	CC = clang++
	STD = c++14
	POST_BUILD =
	INCLUDE_FLAGS =
    SYSTEM_FLAGS =
    PLATFORM_DIR = Posix
endif

MODE = DEBUG
#MODE = RELEASE

ifeq ($(MODE),DEBUG)
    DEBUG_FLAGS = -g -DDEBUG
	OPT_LEVEL = 0
else
    DEBUG_FLAGS =
	OPT_LEVEL = 3
endif

EXPLICIT_OPTIMIZATIONS =

TARGET		= soft
CFLAGS		= -std=$(STD) -O$(OPT_LEVEL) $(EXPLICIT_OPTIMIZATIONS) -Wall -I. $(INCLUDE_FLAGS) $(DEBUG_FLAGS) -Wno-unknown-pragmas

LINKER		= $(CC) -o
LFLAGS		= -Wall -I.

SRCDIR		= SoftRenderer
OBJDIR		= obj
BINDIR		= .

SOURCES		:= $(wildcard $(SRCDIR)/*.cpp $(SRCDIR)/$(PLATFORM_DIR)/*.cpp)
INCLUDES	:= $(wildcard $(SRCDIR)/*.h) External/Include
OBJECTS		:= $(SOURCES:$(SRCDIR)%.cpp=$(OBJDIR)/%.o)$($(SRCDIR)/$(PLATFORM_DIR)%.cpp=$(OBJDIR)/$(PLATFORM_DIR)/%.o)

all: $(BINDIR)/$(TARGET) post-build
	@echo Done!

$(BINDIR)/$(TARGET): $(OBJECTS)
	@echo @$(LINKER) $@ $(LFLAGS) $(OBJECTS) $(SDL_LFLAGS) $(LFLAGS) $(SYSTEM_FLAGS)
	@$(LINKER) $@ $(ALFLAGS) $(OBJECTS) $(SDL_LFLAGS) $(LFLAGS) $(SYSTEM_FLAGS)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@echo @$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(CFLAGS) -I $(INCLUDES) -c $< -o $@

.PHONEY: clean
clean:
	@$(RM) $(OBJECTS)

.PHONEY: remove
remove: clean
	@$(RM) $(BINDIR)/$(TARGET)

.PHONEY: run
run: all
	./$(TARGET)

post-build:
	$(POST_BUILD)
