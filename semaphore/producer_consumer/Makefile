CXX := g++
CXXFLAGS := -Wall -std=c++17 -g
BUILDDIR := build

# EC_LIST := $(shell grep -o 'EC[0-9]\+' main.cpp | sort -u)
# TARGET := $(addprefix $(BUILDDIR)/, $(EC_LIST))
# TARGET += $(BUILDDIR)/producer $(BUILDDIR)/consumer
TARGET := app
SOURCE := main.cpp RingBuffer.cpp

all: $(TARGET)

app: $(SOURCE) $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(BUILDDIR)/$@

$%: %.cpp $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILDDIR):
	mkdir -p $@

clean:
	rm -rf $(BUILDDIR)