all:

include ../kaldi.mk

OBJFILES = kaldi-python-common.o read-kaldi-data.o kaldi-segmentation.o

LIBNAME = python-kaldi

ADDLIBS = ../base/kaldi-base.a ../util/kaldi-util.a  ../matrix/kaldi-matrix.a ../hmm/kaldi-hmm.a ../fstext/kaldi-fstext.a ../tree/kaldi-tree.a ../decoder/kaldi-decoder.a ../matrix/kaldi-matrix.o ../gmm/kaldi-gmm.a

LIBFILE = lib$(LIBNAME).so

LDFLAGS += -Wl,-rpath=$(shell readlink -f $(KALDILIBDIR)) -L.

LDFLAGS += $(foreach dep,$(ADDLIBS), -L$(dir $(dep)))
LDLIBS  += $(foreach dep,$(ADDLIBS), -l$(notdir $(basename $(dep))) )
XDEPENDS = $(foreach dep,$(ADDLIBS), $(dir $(dep))/lib$(notdir $(basename $(dep))).so)

all: depend  $(LIBFILE) $(BINFILES)

$(LIBFILE): $(OBJFILES)
	$(CXX) -shared -fPIC -Wl,-soname=$@ -Wl,--no-undefined -Wl,--as-needed -o $@ $^ $(LDFLAGS) $(XDEPENDS) $(LDLIBS)

depend: 
	-$(CXX) -M $(CXXFLAGS) *.cc > .depend.mk

