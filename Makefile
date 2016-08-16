prefix?=/usr/lib
export prefix

LIBS:=gudev-1.0 systemd

all : $(patsubst %, install-%, $(LIBS))

.PHONY: all

define derp =
include "$(lib).mk"
endef
$(foreach lib,$(LIBS),$(eval $(derp)))

%.mk:
	perl makemake.pl > temp
	mv temp $@
