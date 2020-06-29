dnl
dnl $Id: config.m4,v 1.4 2002/03/20 02:07:11 sniper Exp $
dnl

if test "$PHP_OPENSSL" != "no"; then
  PHP_NEW_EXTENSION(openssl, openssl.c, $ext_openssl_shared)
  OPENSSL_SHARED_LIBADD="-lcrypto -lssl"
  PHP_SUBST(OPENSSL_SHARED_LIBADD)
  AC_DEFINE(HAVE_OPENSSL_EXT,1,[ ])
fi
