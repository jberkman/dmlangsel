# -*- Makefile -*-

bin_PROGRAMS += dmlangsel

dmlangsel_SOURCES :=				\
	gdm-common-config.c			\
	gdm-common-config.h			\
	gdm-config.c				\
	gdm-config.h				\
	gdmlanguages.c				\
	gdmlanguages.h				\
	dmlangsel.c				\
	dmlangsel.h				\
	dmlangsel-compat-gdm.c			\
	dmlangsel-compat-gdm.h

dmlangsel_CPPFLAGS := -DLOCALEDIR=\"$(datadir)/locale\"
dmlangsel_CFLAGS := $(DMLANGSEL_CFLAGS)

dmlangsel_LDADD := $(DMLANGSEL_LIBS)

apps_DATA += dmlangsel.desktop

dist_noinst_DATA +=				\
	autogen.sh				\
	dmlangsel.desktop.in			\
	dmlangsel.spec				\
	intltool-extract.in			\
	intltool-merge.in			\
	intltool-update.in

DISTCLEANFILES =				\
	intltool-extract			\
	intltool-merge				\
	intltool-update

compat_headers := gdm.h gdmwm.h gdmcommon.h gdmconfig.h gdm-socket-protocol.h gdm-common.h
noinst_HEADERS += $(compat_headers)

gdmlanguages.c: $(compat_headers)

$(compat_headers):
	echo '#include "dmlangsel-compat-gdm.h"' > $@
