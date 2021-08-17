AC_PREREQ([2.71])

AC_INIT
AC_CONFIG_SRCDIR([imcanna.c])

PACKAGE=im-canna3
VERSION=0.9

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)

AC_CONFIG_HEADERS([config.h:config.h.in])

AC_PROG_CC
AC_DISABLE_STATIC([])
LT_INIT

ALL_LINGUAS="ja"
AC_SUBST(ALL_LINGUAS)

#GETTEXT_PACKAGE=im-canna
#AC_SUBST(GETTEXT_PACKAGE)
#AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE")
#AM_GLIB_GNU_GETTEXT

GTK_DEP_CFLAGS=`pkg-config gtk+-3.0 --cflags`
GTK_DEP_LIBS=`pkg-config gtk+-3.0 --libs`
GTK_VERSION=`pkg-config gtk+-3.0 --variable=gtk_binary_version`
AC_SUBST(GTK_DEP_CFLAGS)
AC_SUBST(GTK_DEP_LIBS)
AC_SUBST(GTK_VERSION)

LT_RELEASE=$GTK_MAJOR_VERSION.$GTK_MINOR_VERSION
LT_CURRENT=2
LT_REVISION=$GTK_INTERFACE_AGE
LT_AGE=2
LT_CURRENT_MINUS_AGE=0
AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)
AC_SUBST(LT_CURRENT_MINUS_AGE)

dnl Canna check
AC_CHECK_HEADER(canna/jrkanji.h,,AC_MSG_ERROR(No Canna header))
AC_CHECK_LIB(canna, jrKanjiString,, AC_MSG_ERROR(No Canna library))

dnl
AC_ARG_ENABLE([hack-for-firefox52],
	[AS_HELP_STRING([--enable-hack-for-firefox52],
	 [enable to an action which firefox52 (or its variant) to completely able to handle backspace-key event when preedit is empty. (default=no)])],
	 [],
	 [enable_hack_for_firefox52=no])

AS_IF([test "x$enable_hack_for_firefox52" != xno],
    [AC_DEFINE([USE_HACK_FOR_FIREFOX52], [1],[for firefox52 hack])]
    )

AC_ARG_ENABLE([keysnooper],
	[AS_HELP_STRING([--enable-keysnooper],
	 [enable Keysnooper function that makes key input more smooth in fence mode and candinate mode. (default=yes)])],
	 [],
	 [enable_keysnooper=yes])

AS_IF([test "x$enable_keysnooper" != xno],
    [AC_DEFINE([USE_KEYSNOOPER], [1],[enable keysnooper])]
    )

AC_CONFIG_FILES([
Makefile
im-canna.spec
])
AC_OUTPUT
