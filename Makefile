prefix?=/usr/lib
export prefix

LIBS:=gudev-1.0 systemd

all : $(patsubst %, install-%, $(LIBS))

.PHONY: all

install-%: $(prefix)/lib%.so $(prefix)/lib%.so.0
	echo huh?

$(prefix)/lib%.so: $(prefix)/lib%.so.0.0
	ln -s lib$*.so.0.0 $@

$(prefix)/lib%.so.0: $(prefix)/lib%.so.0.0
	ln -s lib$*.so.0.0 $@

$(prefix)/lib%.so.0.0: lib%.so.0.0
	install $< $@

define LINK =
	$(CC) -Wl,--version-script,systemdsux.version 
					 -ggdb	
					 -shared 
					 -Wl,-son ame, longname, 
					 -fpic
					 -o $@
				   $^
endef

define \n

endef

LINK:=$(subst ${\n}, ,$(LINK))

lib%.so.0.0: %.c
	$(LINK)
