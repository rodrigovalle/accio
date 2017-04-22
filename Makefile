CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
USERID=104494120
CLASSES=file.cpp socket.cpp

CHECKS=clang-analyzer-cplusplus*,cppcoreguidelines*,google*,llvm*,modernize*,readability*

all: server client

server: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp

client: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM server client *.tar.gz *.plist

tidy-%: %.cpp
	clang-tidy $< -checks=$(CHECKS) -- -std=c++11

dist: tarball
tarball: clean
	tar -cvf /tmp/$(USERID).tar.gz --exclude=./.vagrant . && mv /tmp/$(USERID).tar.gz .
