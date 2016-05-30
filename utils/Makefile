OBJECTS = PsqlReader.o

LIBNAME = libbdtaunu_graphutils.so

BOOST_ROOT = /usr/local/boost_1_59_0
BOOST_LIBS = $(BOOST_ROOT)/stage/lib

LIBPQ_ROOT = /usr/pgsql-9.4
LIBPQ_INCS = $(LIBPQ_ROOT)/include
LIBPQ_LIBS = $(LIBPQ_ROOT)/lib

INCFLAGS = -I$(BOOST_ROOT) -I$(LIBPQ_INCS)
LDFLAGS = -L$(BOOST_LIBS) -L$(LIBPQ_LIBS) \
					-Wl,-rpath,$(BOOST_LIBS) -lboost_program_options \
					-Wl,-rpath,$(BOOST_LIBS) -lpq

CXX := g++
CXXFLAGS = -Wall -fPIC -pthread -std=c++11 -O2 -Wno-unused-local-typedef -Wno-redeclared-class-member

SRCS = $(wildcard *.cc)
BUILDDIR = build

DEPDIR = .d
$(shell mkdir -p $(DEPDIR) > /dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

lib : $(LIBNAME)

$(LIBNAME) : $(addprefix $(BUILDDIR)/, $(OBJECTS))
	if [ "$(shell uname)" = "Darwin" ]; then \
    SHARED_LIB_FLAG="-dynamiclib -Wl,-install_name,@rpath/$(LIBNAME)"; \
  else \
    SHARED_LIB_FLAG="-shared -Wl,-soname,$(LIBNAME)"; \
  fi; \
	$(CXX) $${SHARED_LIB_FLAG} $(LDFLAGS) $^ -o $(LIBNAME)


$(BUILDDIR)/%.o : %.cc
$(BUILDDIR)/%.o : %.cc $(DEPDIR)/%.d
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(INCFLAGS) -c $< -o $@
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;

.PRECIOUS: $(DEPDIR)/%.d

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))

clean : 
	@rm -f *~ $(BUILDDIR)/* $(LIBNAME)

cleanall : clean
	@rm -f $(DEPDIR)/*
