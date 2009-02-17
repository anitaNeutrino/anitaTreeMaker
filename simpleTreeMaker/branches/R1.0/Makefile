#############################################################################
## Makefile -- New Version of my Makefile that works on both linux
##              and mac os x
## Ryan Nichol <rjn@hep.ucl.ac.uk>
##############################################################################
include Makefile.arch

#Site Specific  Flags
SYSINCLUDES	=
SYSLIBS         = 

ifdef ANITA_UTIL_INSTALL_DIR
ANITA_UTIL_LIB_DIR=${ANITA_UTIL_INSTALL_DIR}/lib
ANITA_UTIL_INC_DIR=${ANITA_UTIL_INSTALL_DIR}/include
LD_ANITA_UTIL=-L$(ANITA_UTIL_LIB_DIR)
INC_ANITA_UTIL=-I$(ANITA_UTIL_INC_DIR)
else
ANITA_UTIL_LIB_DIR=
ANITA_UTIL_INC_DIR=
ifdef EVENT_READER_DIR
LD_ANITA_UTIL=-L$(EVENT_READER_DIR)
INC_ANITA_UTIL=-I$(EVENT_READER_DIR)
endif
endif


#Generic and Site Specific Flags
CXXFLAGS     += $(ROOTCFLAGS) $(FFTFLAG) $(SYSINCLUDES) $(INC_ANITA_UTIL)
LDFLAGS      += -g $(ROOTLDFLAGS) 

LIBS          = $(ROOTLIBS)  -lMinuit $(SYSLIBS) $(LD_ANITA_UTIL) $(FFTLIBS) -lAnitaEvent
GLIBS         = $(ROOTGLIBS) $(SYSLIBS)


TREE_MAKER = makeRawHeadTree makeMonitorTree makeOtherTree makeHkTree makePrettyHkTree makeTurfRateTree makeSurfHkTree makeEventRunTree makeRawScalerTree makeSummedTurfRateTree makeAveragedSurfHkTree makeGpsTree makeAuxiliaryTree quickCheckForErrors makeRunSummaryFile


all : $(TREE_MAKER)

% :  %.$(SRCSUF)
	@echo "<**Linking**> "  
	$(LD)  $(CXXFLAGS) $(LDFLAGS) $<  $(LIBS) -o $@


clean:
	@rm -f *Dict*
	@rm -f *.${OBJSUF}
	@rm -f $(TREE_MAKER)
