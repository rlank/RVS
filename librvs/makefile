CC=gcc
CP=g++
CPflags=-g -std=c++11 -Wno-write-strings -Wall -pedantic -c
SHAREFLAGS=-fPIC

libver=0.2

biopath:=Biomass
dmpath:=DataManagement
buildpath:=build

INCLUDES:=-I$(shell pwd)/Biomass/ -I$(shell pwd)/DataManagement/

##########
## MAIN ##
##########

sources := $(wildcard $(biopath)/*.cpp $(dmpath)/*.cpp)
objects := $(patsubst %.cpp,%.o, $(sources))

all: lib exe
	

lib: $(buildpath) $(objects)
	$(CP) -shared $(buildpath)/*.o -o $(buildpath)/librvs.so.$(libver)

exe: $(lib)
	$(CP) $(CPflags) main.cpp -o $(buildpath)/main.o
	ln -s $(shell pwd)/$(buildpath)/librvs.so.$(libver) $(shell pwd)/$(buildpath)/librvs.so
	$(CP) -o $(buildpath)/rvs $(buildpath)/main.o -lsqlite3 -L. $(shell pwd)/$(buildpath)/librvs.so

$(buildpath):
	mkdir -p $(buildpath)

%.o: 
	$(CP) $(CPflags) $(SHAREFLAGS) $(@:.o=.cpp) -o $(buildpath)/$(notdir $@)
	

###########
## CLEAN ##
###########

clean: 
	rm -rfv $(buildpath)/*.o $(buildpath)/*.so* $(buildpath)/rvs $(buildpath)/librvs

cleangit:
	rm -rfv $(buildpath)
	
