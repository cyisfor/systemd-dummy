prefix?=/usr/lib
export prefix

LIBS:=gudev-1.0 systemd

all: $(patsubst %, lib%.so.0.0, $(LIBS))

MAKESUCKS:=$(patsubst %, $(prefix)/lib%.so, $(LIBS))
install : $(MAKESUCKS) $(patsubst %, %.0, $(MAKESUCKS))

.PHONY: all

$(prefix)/lib%.so: $(prefix)/lib%.so.0.0
	ln -s lib$*.so.0.0 $@

$(prefix)/lib%.so.0: $(prefix)/lib%.so.0.0
	ln -s lib$*.so.0.0 $@

$(prefix)/lib%.so.0.0: lib%.so.0.0
	install $< $@

name:=derp
export CC
lib%.so.0.0: %.c
	exec python ./link.py $@ $^

.PRECIOUS: lib%.so.0.0 $(prefix)/lib%.so.0.0
