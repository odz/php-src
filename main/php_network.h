/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2001 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Stig Venaas <venaas@uninett.no>                             |
   +----------------------------------------------------------------------+
 */
/* $Id: php_network.h,v 1.5 2001/02/26 06:07:31 andi Exp $ */

#ifndef _PHP_NETWORK_H
#define _PHP_NETWORK_H

int php_hostconnect(char *host, unsigned short port, int socktype, int timeout);

#endif /* _PHP_NETWORK_H */

/*
 * Local variables:
 * tab-width: 8
 * c-basic-offset: 8
 * End:
 */
