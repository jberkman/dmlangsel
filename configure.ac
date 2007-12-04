#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([dmlangsel], [0.4], [jberkman@novell.com])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([gdmlanguages.c])
AC_CONFIG_HEADER([config.h])

GETTEXT_PACKAGE=$PACKAGE
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], [$GETTEXT_PACKAGE], [Package name for gettext])
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.35.0])

# Honor aclocal flags
ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

# Checks for packages
GTK_REQVER="2.8.0"
FONTCONFIG_REQVER="2.3"

PKG_CHECK_MODULES(DMLANGSEL,
	gtk+-2.0 >= $GTK_REQVER
	fontconfig >= $FONTCONFIG_REQVER)
AC_SUBST(DMLANGSEL_CFLAGS)
AC_SUBST(DMLANGSEL_LIBS)

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_LIBTOOL

# Make libtool use --silent when --silent is passed to make
changequote(,)dnl
LIBTOOL="${LIBTOOL} \$(shell echo \"\$(MFLAGS)\" | awk '/^[^ ]*s/ { print \"--silent\" }')"
changequote([,])dnl

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([locale.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_STRUCT_TM

# Checks for library functions.
AC_CHECK_FUNCS([strchr strrchr])

AC_CONFIG_FILES([
Makefile
dmlangsel.spec
po/Makefile.in
])

AC_OUTPUT
