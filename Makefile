BINARIES = test_psqlreader
OBJECTS = PsqlReader.o

BOOST_ROOT = /usr/local/boost_1_56_0
BOOST_LIBS = $(BOOST_ROOT)/stage/lib

LIBPQ_ROOT = /usr/local/pgsql
LIBPQ_INCS = $(LIBPQ_ROOT)/include
LIBPQ_LIBS = $(LIBPQ_ROOT)/lib

INCFLAGS = -I$(BOOST_ROOT) -I$(LIBPQ_INCS)
LDFLAGS = -L$(BOOST_LIBS) -L$(LIBPQ_LIBS) \
					-Wl,-rpath,$(BOOST_LIBS) -lboost_program_options \
					-Wl,-rpath,$(BOOST_LIBS) -lpq

CXX := g++
CXXFLAGS = -Wall -pthread -std=c++11 -O2

SRCS = $(wildcard *.cc)
BUILDDIR = build

DEPDIR = .d
$(shell mkdir -p $(DEPDIR) > /dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

all : $(BINARIES)

test_pq : $(addprefix $(BUILDDIR)/, test_pq.o $(OBJECTS))
	$(CXX) $(LDFLAGS) $^ -o $@

test_psqlreader : $(addprefix $(BUILDDIR)/, test_psqlreader.o $(OBJECTS))
	$(CXX) $(LDFLAGS) $^ -o $@

$(BUILDDIR)/%.o : %.cc
$(BUILDDIR)/%.o : %.cc $(DEPDIR)/%.d
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(INCFLAGS) -c $< -o $@
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;

.PRECIOUS: $(DEPDIR)/%.d

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))

clean : 
	@rm -f *~ $(BINARIES) $(BUILDDIR)/* *.pdf *.gif *.png *.gv *.ps

cleanall : clean
	@rm -f $(DEPDIR)/*
