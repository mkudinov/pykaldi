all: 

include ../../kaldi.mk
CXXFLAGS += -I$(dir $(KALDILIBDIR))
OBJFILES = kaldi-python-feat.o

LIBNAME = python-kaldi-feat

ADDLIBS = ../../base/kaldi-base.a ../../util/kaldi-util.a ../../matrix/kaldi-matrix.a ../../feat/kaldi-feat.a ../common/python-kaldi-common.so

LIBFILE = lib$(LIBNAME).so

LDFLAGS += -Wl,-rpath=$(shell readlink -f $(KALDILIBDIR)) -L.

LDFLAGS += $(foreach dep,$(ADDLIBS), -L$(dir $(dep)))
LDLIBS  += $(foreach dep,$(ADDLIBS), -l$(notdir $(basename $(dep))) )
XDEPENDS = $(foreach dep,$(ADDLIBS), $(dir $(dep))/lib$(notdir $(basename $(dep))).so)


all: depend $(LIBFILE)

$(LIBFILE): $(OBJFILES)
	$(CXX) -shared -fPIC -Wl,-soname=$@ -Wl,--no-undefined -Wl,--as-needed -o $@ $^ $(LDFLAGS) $(XDEPENDS) $(LDLIBS)

depend:
	-$(CXX) -M $(CXXFLAGS) *.cc > .depend.mk

.PHONY: clean

clean:
	rm -f *.o *.so .depend.mk