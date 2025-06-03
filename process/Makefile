CXX = g++
CXXFLAGS = -Wall -std=c++17
BUILDDIR = build
TARGET = ec1 ec2 ec3 ec4 ec5

all: $(TARGET)

ec1: $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -DEC1 main.cpp -o $(BUILDDIR)/ec1

ec2: $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -DEC2 main.cpp -o $(BUILDDIR)/ec2

ec3: $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -DEC3 main.cpp -o $(BUILDDIR)/ec3

ec4: $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -DEC4 main.cpp -o $(BUILDDIR)/ec4

ec5: $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -DEC5 main.cpp -o $(BUILDDIR)/ec5

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)/*