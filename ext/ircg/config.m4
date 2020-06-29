dnl
dnl $Id: config.m4,v 1.9 2002/01/24 12:55:08 sas Exp $
dnl

PHP_ARG_WITH(ircg, for IRCG support,
[  --with-ircg             Include IRCG support])

AC_ARG_WITH(ircg-config,
[  --with-ircg-config        IRCG: Path to the ircg-config script],
[ IRCG_CONFIG=$withval ],
[ IRCG_CONFIG=ircg-config ])

if test "$PHP_IRCG" != "no"; then
  $IRCG_CONFIG --ldflags
  if test "$?" != "0"; then
    AC_MSG_ERROR([I cannot run the ircg-config script which should have been installed by IRCG. Please ensure that the script is in your PATH or point --with-ircg-config to the path of the script.])
  fi
  
  PHP_EVAL_LIBLINE(`$IRCG_CONFIG --ldflags`)
  PHP_EVAL_INCLINE(`$IRCG_CONFIG --cppflags`)
  PHP_ADD_LIBRARY_WITH_PATH(ircg, $PHP_IRCG/lib)
  PHP_ADD_INCLUDE($PHP_IRCG/include)
  if test "$PHP_SAPI" = "thttpd"; then
    AC_DEFINE(IRCG_WITH_THTTPD, 1, [Whether thttpd is available])
    PHP_DISABLE_CLI
  fi
  AC_DEFINE(HAVE_IRCG, 1, [Whether you want IRCG support])
  PHP_EXTENSION(ircg, $ext_shared)
fi
