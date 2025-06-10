CXX = g++
CXXFLAGS = -Wall -std=c++17 -g
BUILDDIR = build

EC_LIST := $(shell grep -o 'EC[0-9]\+' main.cpp | sort -u)
TARGET := $(addprefix $(BUILDDIR)/, $(EC_LIST))

all: $(TARGET) $(BUILDDIR)/read $(BUILDDIR)/write

$(BUILDDIR)/EC4: main.cpp $(BUILDDIR)/read $(BUILDDIR)/write | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -DEC4 $< -o $@

$(BUILDDIR)/EC%: main.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -DEC$* $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/read: read.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILDDIR)/write: write.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(BUILDDIR)