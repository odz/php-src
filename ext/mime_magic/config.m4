dnl $Id: config.m4,v 1.6 2002/09/24 06:37:17 sas Exp $
dnl config.m4 for extension mime_magic

PHP_ARG_ENABLE(mime-magic, whether to enable mime_magic support,
[  --enable-mime-magic     Enable mime_magic support])

if test "$PHP_MIME_MAGIC" = "yes"; then
  dnl PHP_SUBST(MIME_MAGIC_SHARED_LIBADD)

  PHP_NEW_EXTENSION(mime_magic, mime_magic.c, $ext_shared)

  # Try to see if we can find the path of the magic file in its 
  # default locations.
  if test -f /usr/share/magic.mime ; then
	PHP_MIME_MAGIC_FILE_PATH=/usr/share/magic.mime
  elif test -f /usr/share/magic ; then
	PHP_MIME_MAGIC_FILE_PATH=/usr/share/magic	
  elif test -f /usr/share/misc/magic.mime ; then
 	PHP_MIME_MAGIC_FILE_PATH=/usr/share/misc/magic.mime
  elif test -f /etc/magic ; then
  	PHP_MIME_MAGIC_FILE_PATH=/etc/magic
  else
  	PHP_MIME_MAGIC_FILE_PATH=""	
  fi

  AC_DEFINE_UNQUOTED(PHP_MIME_MAGIC_FILE_PATH,"$PHP_MIME_MAGIC_FILE_PATH",[magic file path])

fi