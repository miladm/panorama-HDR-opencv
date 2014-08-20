INCDIR = ../include
OBJDIR = ../obj
COMMONDIR = ../common
BINDIR = ../bin

HEADERS = $(wildcard $(INCDIR)/*.h)
HEADERS += $(wildcard *.h)

OBJNAMES :=
OBJS = $(patsubst %, $(OBJDIR)/%.o, $(OBJNAMES))
DEBUGOBJS = $(patsubst %, $(OBJDIR)/%.o, $(OBJNAMES))

TARGETNAMES := Panorama
TARGETS := $(patsubst %, $(BINDIR)/%, $(TARGETNAMES))
DEBUGTARGETS := $(patsubst %, $(BINDIR)/%.debug, $(TARGETNAMES))

CXXFLAGS += -I/usr/local/include
LDFLAGS += -L/usr/local/lib

OPENCV_MODULES := core flann calib3d contrib features2d gpu highgui imgproc legacy ml nonfree objdetect ocl photo stitching superres ts video videostab
LIBS += $(patsubst %, -lopencv_%, $(OPENCV_MODULES))

.PHONY: all

all: $(TARGETS) $(DEBUGTARGETS)

$(TARGETS) : $(BINDIR)/%: $(OBJDIR)/%.o $(OBJS)
		/usr/bin/g++ -std=c++0x -O3 -mtune=native -march=native $(LDFLAGS) -g -o $@ $^ $(LIBS)

$(DEBUGTARGETS): $(BINDIR)/%.debug: $(OBJDIR)/%.debug.o $(DEBUGOBJS)
		/usr/bin/g++ -std=c++0x $(LDFLAGS) -g -o $@ $^ $(LIBS)

$(OBJDIR)/%.o: %.cpp $(HEADERS)
		/usr/bin/g++ -std=c++0x -c -O3 -mtune=native -march=native $(CXXFLAGS) -g -I $(INCDIR) -I ./ -o $@ $<

$(OBJDIR)/%.debug.o: %.cpp $(HEADERS)
		/usr/bin/g++ -std=c++0x -c $(CXXFLAGS) -g -I $(INCDIR) -I ./ -o $@ $<
