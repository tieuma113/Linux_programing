CXX := g++
CXXFLAGS := -Wall -std=c++17 -O0 -g
EC_lIST := $(shell grep -o 'EC[0-9]\+' main.cpp | sort -u) 
TARGETS := $(addprefix $(BUILDDIR)/, $(EC_LIST))
BUILDDIR := build

all: $(TARGETS)

$(BUILDDIR)/EC%: main.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -DEC$* $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)