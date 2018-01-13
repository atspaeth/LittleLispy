LDFLAGS := 
CFLAGS := -std=c11

INCDIRS := inc
INCDIRS := $(foreach dir,$(INCDIRS),-I$(dir))

LIBPATHS :=
LIBS := 
LIBS := $(foreach lib,$(LIBS),-l$(lib))

CFLAGS := $(INCDIRS) $(CFLAGS)

TARGET:=$(shell basename $(CURDIR))

VPATH:=src
BLD:=bld

CFILES := $(wildcard $(VPATH)/*.c)
OFILES := $(foreach file,$(CFILES),$(BLD)/$(shell basename $(file)).o)
GENFILES := $(BLD) .gitignore
GENERATED := $(OFILES) $(GENFILES) $(TARGET)

.PHONY: clean all run

all : $(TARGET)
$(TARGET) : $(GENFILES) $(OFILES)
	@echo Linking.
	@clang $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

.gitignore :
	@[ -f .gitignore ] && rm .gitignore || true
	@touch .gitignore
	@for file in $(GENERATED) ; do echo $$file >> .gitignore ; done
	@echo '*~' >> .gitignore

run : all
	./$(TARGET)

clean: 
	@for file in $(GENERATED) ; do [ -f $$file ] && rm $$file || true ; done

$(BLD):
	@mkdir $(BLD)

$(BLD)/%.c.o: %.c
	@echo Compiling $(notdir $<)...
	@clang -MMD -MP -MF $(BLD)/$*.d $(CFLAGS) -c $< -o $@
