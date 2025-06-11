CXX = g++
CXXFLAGS = -Wall -std=c++17 -g -pthread
BUILDDIR = build

EC_LIST := $(shell grep -o 'EC[0-9]\+' main.cpp | sort -u)
TARGET = $(addprefix $(BUILDDIR)/, $(EC_LIST))

all: $(TARGET)

$(BUILDDIR)/EC%: main.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -DEC$* $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)/*