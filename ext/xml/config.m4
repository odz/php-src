# $Source: /repository/php4/ext/xml/config.m4,v $
# $Id: config.m4,v 1.24 2001/01/10 14:41:01 hirokawa Exp $

dnl Fallback for --with-xml[=DIR]
AC_ARG_WITH(xml,[],enable_xml=$withval)

AC_C_BIGENDIAN

if test "$ac_cv_c_bigendian" = "yes"; then
  order=21
else
  order=12
fi

PHP_ARG_ENABLE(xml,for XML support,
[  --disable-xml           Disable XML support using bundled expat lib], yes)

if test "$PHP_XML" != "no"; then

  AC_DEFINE(HAVE_LIBEXPAT, 1, [ ])

  if test "$PHP_XML" = "yes"; then
    CPPFLAGS="$CPPFLAGS -DXML_BYTE_ORDER=$order"
    EXPAT_INTERNAL_LIBADD="expat/libexpat.la"	    
    PHP_SUBST(EXPAT_INTERNAL_LIBADD)
    EXPAT_SUBDIRS="expat"	    
    PHP_SUBST(EXPAT_SUBDIRS)
    PHP_SUBST(EXPAT_SHARED_LIBADD)
    PHP_EXTENSION(xml, $ext_shared)
    LIB_BUILD($ext_builddir/expat,$ext_shared,yes)
    LIB_BUILD($ext_builddir/expat/xmlparse,$ext_shared,yes)
    LIB_BUILD($ext_builddir/expat/xmltok,$ext_shared,yes)
    AC_ADD_INCLUDE($ext_srcdir/expat/xmltok)
    AC_ADD_INCLUDE($ext_srcdir/expat/xmlparse)
    PHP_FAST_OUTPUT($ext_builddir/expat/Makefile $ext_builddir/expat/xmlparse/Makefile $ext_builddir/expat/xmltok/Makefile)

  else

    EXPAT_DIR="$withval"
    if test -f $EXPAT_DIR/lib/libexpat.a -o -f $EXPAT_DIR/lib/libexpat.so ; then
        AC_DEFINE(HAVE_LIBEXPAT2, 1, [ ])
        AC_ADD_INCLUDE($EXPAT_DIR/include)
    else
        AC_MSG_RESULT(not found)
        AC_MSG_ERROR(Please reinstall the expat distribution)
    fi

    PHP_SUBST(EXPAT_SHARED_LIBADD)
    AC_ADD_LIBRARY_WITH_PATH(expat, $EXPAT_DIR/lib, EXPAT_SHARED_LIBADD)
    PHP_EXTENSION(xml, $ext_shared)
  fi
fi
