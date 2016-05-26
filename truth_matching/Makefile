BINARIES = test4
OBJECTS = TruthMatcher.o

UTILS_ROOT = ../utils

BOOST_ROOT = /usr/local/boost_1_56_0
BOOST_LIBS = $(BOOST_ROOT)/stage/lib

LIBPQ_ROOT = /usr/local/pgsql
LIBPQ_INCS = $(LIBPQ_ROOT)/include
LIBPQ_LIBS = $(LIBPQ_ROOT)/lib

INCFLAGS = -I$(UTILS_ROOT) -I$(BOOST_ROOT) -I$(LIBPQ_INCS)
LDFLAGS = -L $(UTILS_ROOT) -L$(BOOST_LIBS) -L$(LIBPQ_LIBS) \
					-Wl,-rpath,$(UTILS_ROOT) -lbdtaunu_graphutils \
					-Wl,-rpath,$(BOOST_LIBS) -lboost_program_options \
					-Wl,-rpath,$(LIBPQ_LIBS) -lpq

CXX := g++
CXXFLAGS = -Wall -fPIC -pthread -std=c++11 -O2 -Wno-unused-local-typedef -Wno-redeclared-class-member

SRCS = $(wildcard *.cc)
BUILDDIR = build

DEPDIR = .d
$(shell mkdir -p $(DEPDIR) > /dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

all : $(BINARIES)

test% : $(addprefix $(BUILDDIR)/, test%.o $(OBJECTS))
	$(CXX) $(LDFLAGS) $^ -o $@
	
$(BUILDDIR)/%.o : %.cc
$(BUILDDIR)/%.o : %.cc $(DEPDIR)/%.d
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(INCFLAGS) -c $< -o $@
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;

.PRECIOUS: $(DEPDIR)/%.d

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))

clean : 
	@rm -f *~ $(BINARIES) $(BUILDDIR)/* *.pdf *.gif *.png *.gv *.ps *.csv

cleanall : clean
	@rm -f $(DEPDIR)/*
