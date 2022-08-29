cc=g++

oj_server:oj_server.cc
	$(cc) -o $@ $^ -std=c++11 -lpthread -lctemplate -ljsoncpp
.PHONY:clean
clean:
	rm -rf oj_server