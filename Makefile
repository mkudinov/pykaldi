PYTHONKALDILIBDIR=lib
SUBDIRS = common asr_model fst readers segmentation
CLEANSUBDIRS=$(addsuffix .clean,$(SUBDIRS))
COPYSUBDIRS=$(addsuffix .copy,$(SUBDIRS))

.DEFAULT_GOAL :=
all: $(SUBDIRS)
	-echo Done

.PHONY: $(SUBDIRS)
$(SUBDIRS) : 
	$(MAKE) -C $@

.PHONY: clean
clean: $(CLEANSUBDIRS)
	-echo Cleaned

.PHONY: $(CLEANSUBDIRS) 
$(CLEANSUBDIRS) :
	$(MAKE) -C $(basename $@) clean

.PHONY: install
install: mkinstalldir $(COPYSUBDIRS)
	-echo Installed

.PHONY: $(COPYSUBDIRS) 
$(COPYSUBDIRS) :
	mv $(basename $@)/*.so $(PYTHONKALDILIBDIR) 

.PHONY: mkinstalldir
mkinstalldir:
	mkdir -p $(PYTHONKALDILIBDIR)
