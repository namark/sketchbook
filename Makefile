
override CPPFLAGS	+= --std=c++1z
override CPPFLAGS	+= -MMD -MP
override CPPFLAGS	+= $(shell cat .cxxflags | xargs)
override CPPFLAGS	+= -I./include
override LDFLAGS	+= $(shell cat .ldflags | xargs)
override LDFLAGS	+= -L./lib
override LDLIBS		+= -lSDL2main -lSDL2 -lpthread -lGL -lGLEW

SOURCES	:= $(shell echo *.cpp)
TEMPDIR	:= temp
DISTDIR	:= out
TARGETS	:= $(SOURCES:%.cpp=$(DISTDIR)/%)
OBJECTS	:= $(SOURCES:%.cpp=$(TEMPDIR)/%.o)
LOCALIB	:= $(shell echo lib/*.a)
DEPENDS	:= $(OBJECTS:.o=.d)

build: $(TARGETS)

$(DISTDIR)/%: $(TEMPDIR)/%.o $(LOCALIB) | $(DISTDIR)
	$(CXX) $(LDFLAGS) $< $(LOCALIB) $(LDLIBS) -o $@

$(TEMPDIR)/%.o: %.cpp | $(TEMPDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

$(TEMPDIR):
	@mkdir $@

$(DISTDIR):
	@mkdir $@

clean:
	@rm $(DEPENDS) 2> /dev/null || true
	@rm $(OBJECTS) 2> /dev/null || true
	@rmdir $(TEMPDIR) 2> /dev/null || true
	@echo Temporaries cleaned!

distclean: clean
	@rm $(TARGET) 2> /dev/null || true
	@rmdir $(DISTDIR) 2> /dev/null || true
	@echo All clean!

-include $(DEPENDS)

.PRECIOUS : $(OBJECTS)
.PHONY : clean distclean
