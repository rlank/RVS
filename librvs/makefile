CC=gcc
CP=g++
CPflags=-g -std=c++11 -Wno-write-strings -pedantic -c
SHAREFLAGS=-fPIC

libver=0.3

biopath:=Biomass
dmpath:=DataManagement
fuelpath:=Fuels
buildpath:=build

INCLUDES:=-I/usr/include/boost 

##########
## MAIN ##
##########

sources := $(wildcard $(biopath)/*.cpp $(dmpath)/*.cpp $(fuelpath)/*.cpp)
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
	$(CP) $(CPflags) $(INCLUDES) $(SHAREFLAGS) $(@:.o=.cpp) -o $(buildpath)/$(notdir $@) -lboost_system
	

###########
## CLEAN ##
###########

clean: 
	rm -rfv $(buildpath)/*.o $(buildpath)/*.so* $(buildpath)/rvs $(buildpath)/librvs

cleangit:
	rm -rfv $(buildpath)
	