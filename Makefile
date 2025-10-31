.PHONY: default
default: all

SOURCES += from_map16.cpp
SOURCES += to_map16.cpp
SOURCES += human_readable_map16.cpp

ifeq ($(PLATFORM),)
  ifeq ($(OS),Windows_NT)
    PLATFORM = windows
  else
    UNAME = $(shell uname -s)
    ifeq ($(UNAME),Darwin)
      PLATFORM = macos
    else ifeq ($(UNAME),Linux)
      PLATFORM = linux
    else
      PLATFORM = unknown
    endif
  endif
endif

ARCH ?= $(shell uname -m)

ifeq ($(PLATFORM),unknown)
  $(error unknown platform, please specify PLATFORM= windows, macos, or linux)
else
  $(info ==========  building for $(PLATFORM)-$(ARCH) ============)
endif

ifeq ($(PLATFORM),windows)
  LIBEXT = dll
endif

ifeq ($(PLATFORM),macos)
  LIBEXT = dylib
endif

ifeq ($(PLATFORM),linux)
  LIBEXT = so
endif

LIB = human_readable_map16.$(LIBEXT)

.PHONY: all
all: $(LIB)

FLAGS += $(CPPFLAGS)
FLAGS += -Wall
FLAGS += --std=c++17

$(LIB): $(SOURCES)
	$(CXX) -dynamiclib $(FLAGS) $(SOURCES) -o $@

.PHONY: clean
clean:
	rm -f $(LIB)

