
CXXFLAGS	+= --std=c++1z
CXXFLAGS	+= -MMD -MP
CXXFLAGS	+= $(shell cat .cxxflags 2> /dev/null | xargs)
CXXFLAGS	+= -I./include
LDFLAGS		+= $(shell cat .ldflags 2> /dev/null | xargs)
ifneq ($(strip $(WEB)),)
LIBDIR	:= ./lib/web
CXXFLAGS	+= -s USE_SDL=2
LDFLAGS		+= -s USE_SDL=2
TOOLS_CXX	:= $(CXX)
CXX			:= emcc
else
LIBDIR	:= ./lib
LDLIBS	+= -lSDL2main -lSDL2 -lpthread -lGL -lGLEW
endif
LDFLAGS	+= -L$(LIBDIR)


SOURCES	:= $(shell echo *.cpp)

TEMPDIR	:= temp
DISTDIR	:= out
ifneq ($(strip $(WEB)),)
TEMPDIR	:= $(TEMPDIR)/web
DISTDIR	:= $(DISTDIR)/web
endif

OBJECTS	:= $(SOURCES:%.cpp=$(TEMPDIR)/%.o)
LOCALIB	:= $(wildcard $(LIBDIR)/*.a)
DEPENDS	:= $(OBJECTS:.o=.d)


ifneq ($(strip $(WEB)),)
BINEXT			:= /main.html
SHELL_TEMPLATE	:= ./common/emscripten_shell.template
SHELLS			:= $(OBJECTS:%.o=%.shell)
TITLIZE			:= ./tools/titlize
endif

TARGETS	:= $(SOURCES:%.cpp=$(DISTDIR)/%$(BINEXT))
ifneq ($(strip $(WEB)),)
JS		:= $(TARGETS:%.html=%.js)
WASM	:= $(TARGETS:%.html=%.wasm)
OUTDIRS	:= $(shell dirname $(TARGETS))
endif

GIT_HEAD_FILE	:= .git/HEAD
GIT_HEAD_SHA	:= $(shell git rev-parse HEAD)

build: $(TARGETS)

ifneq ($(strip $(WEB)),)
$(DISTDIR)/%$(BINEXT): $(TEMPDIR)/%.o $(TEMPDIR)/%.shell $(LOCALIB) | $(DISTDIR)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $< $(LOCALIB) $(LDLIBS) -o $@ --shell-file $(TEMPDIR)/$*.shell

$(TEMPDIR)/%.shell: $(SHELL_TEMPLATE) $(GIT_HEAD_FILE) $(TITLIZE)
	@sed -e 's/__TITLE__/$(shell echo $* | $(TITLIZE))/g' -e 's/__COMMIT_SHA_FILE__/$(GIT_HEAD_SHA)\/$*.cpp/g' $(SHELL_TEMPLATE) > $@

$(TITLIZE): $(TITLIZE).cpp
	make WEB= CXX=$(TOOLS_CXX) $(TITLIZE)
else
$(DISTDIR)/%$(BINEXT): $(TEMPDIR)/%.o $(LOCALIB) | $(DISTDIR)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $< $(LOCALIB) $(LDLIBS) -o $@
endif

$(TEMPDIR)/%.o: %.cpp | $(TEMPDIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TEMPDIR):
	@mkdir -p $@

$(DISTDIR):
	@mkdir -p $@

clean:
	@rm $(DEPENDS) 2> /dev/null || true
	@rm $(OBJECTS) 2> /dev/null || true
	@rmdir -p $(TEMPDIR) 2> /dev/null || true
	@echo Temporaries cleaned!

distclean: clean
	@rm $(TARGETS) 2> /dev/null || true
	@rm $(JS) 2> /dev/null || true
	@rm $(WASM) 2> /dev/null || true
	@rm $(SHELLS) 2> /dev/null || true
	@rmdir -p $(OUTDIRS) 2> /dev/null || true
	@rmdir -p $(DISTDIR) 2> /dev/null || true
	@echo All clean!

-include $(DEPENDS)

.PRECIOUS : $(OBJECTS)
.PRECIOUS : $(SHELLS)
.PHONY : clean distclean
