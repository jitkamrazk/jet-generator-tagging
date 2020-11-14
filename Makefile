TARGDIR    := src
EXECDIR    := bin
EXECUTABLE := src/bin/jet-generator
EXTERNALS  := simulation

all: $(EXTERNALS)

simulation:
	@echo "Building $@"
	@$(MAKE) -C $(TARGDIR)
	@mkdir $(EXECDIR)
	@cp $(EXECUTABLE) $(EXECDIR)

.PHONY : clean purge $(EXTERNALS)

clean:
	@$(MAKE) $@ -C $(TARGDIR)

purge:
	@rm -rf $(EXECDIR)
	@$(MAKE) $@ -C $(TARGDIR)