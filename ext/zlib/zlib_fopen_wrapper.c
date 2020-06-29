/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2002 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Wez Furlong <wez@thebrainroom.com>, based on work by:        |
   |         Hartmut Holzgraefe <hartmut@six.de>                          |
   +----------------------------------------------------------------------+
 */
/* $Id: zlib_fopen_wrapper.c,v 1.33.2.3 2002/11/19 21:11:03 wez Exp $ */
#define IS_EXT_MODULE
#define _GNU_SOURCE

#include "php.h"
#include "php_zlib.h"
#include "fopen_wrappers.h"

struct php_gz_stream_data_t	{
	gzFile gz_file;
};

static size_t php_gziop_read(php_stream *stream, char *buf, size_t count TSRMLS_DC)
{
	struct php_gz_stream_data_t *self = (struct php_gz_stream_data_t *)stream->abstract;
	int read;
	
	read = gzread(self->gz_file, buf, count);
	
	if (gzeof(self->gz_file))
		stream->eof = 1;
	
	return read < 0 ? 0 : read;
}

static size_t php_gziop_write(php_stream *stream, const char *buf, size_t count TSRMLS_DC)
{
	struct php_gz_stream_data_t *self = (struct php_gz_stream_data_t *)stream->abstract;
	int wrote;
	wrote = gzwrite(self->gz_file, (char*)buf, count);
	return wrote < 0 ? 0 : wrote;
}

static int php_gziop_seek(php_stream *stream, off_t offset, int whence, off_t *newoffs TSRMLS_DC)
{
	struct php_gz_stream_data_t *self = (struct php_gz_stream_data_t *)stream->abstract;
	int ret;
	
	assert(self != NULL);
	
	ret = gzseek(self->gz_file, offset, whence);
	*newoffs = gztell(self->gz_file);
	
	return ret < 0 ? -1 : 0;
}

static int php_gziop_close(php_stream *stream, int close_handle TSRMLS_DC)
{
	struct php_gz_stream_data_t *self = (struct php_gz_stream_data_t *)stream->abstract;
	int ret = EOF;
	
	if (close_handle) {
		if (self->gz_file) {
			ret = gzclose(self->gz_file);
			self->gz_file = NULL;
		}
	}
	efree(self);

	return ret;
}
static int php_gziop_flush(php_stream *stream TSRMLS_DC)
{
	struct php_gz_stream_data_t *self = (struct php_gz_stream_data_t *)stream->abstract;
	return gzflush(self->gz_file, Z_SYNC_FLUSH);
}


php_stream_ops php_stream_gzio_ops = {
	php_gziop_write, php_gziop_read,
	php_gziop_close, php_gziop_flush,
	"ZLIB",
	php_gziop_seek, 
	NULL, /* cast */
	NULL, /* stat */
	NULL  /* set_option */
};

php_stream *php_stream_gzopen(php_stream_wrapper *wrapper, char *path, char *mode,
		int options, char **opened_path, php_stream_context *context STREAMS_DC TSRMLS_DC)
{
	struct php_gz_stream_data_t *self;
	php_stream *stream = NULL, *innerstream = NULL;

	/* sanity check the stream: it can be either read-only or write-only */
	if (strchr(mode, '+')) {
		if (options & REPORT_ERRORS) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "cannot open a zlib stream for reading and writing at the same time!");
		}
		return NULL;
	}
	if (strchr(mode, 'a')) {
		if (options & REPORT_ERRORS) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "cannot append to a zlib stream!");
		}
		return NULL;
	}
	
	self = emalloc(sizeof(*self));

	if (strncasecmp("compress.zlib://", path, 16) == 0)
		path += 16;
	else if (strncasecmp("zlib:", path, 5) == 0)
		path += 5;
	
	innerstream = php_stream_open_wrapper(path, mode, STREAM_MUST_SEEK|options|STREAM_WILL_CAST, opened_path);
	
	if (innerstream) {
		int fd;
		if (SUCCESS == php_stream_cast(innerstream, PHP_STREAM_AS_FD | PHP_STREAM_CAST_RELEASE, (void**)&fd, REPORT_ERRORS)) {
			self->gz_file = gzdopen(fd, mode);
			if (self->gz_file)	{
				stream = php_stream_alloc_rel(&php_stream_gzio_ops, self, 0, mode);
				if (stream) {
					stream->flags |= PHP_STREAM_FLAG_NO_BUFFER;
					return stream;
				}
				gzclose(self->gz_file);
			}
			if (options & REPORT_ERRORS)
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "gzopen failed");
		} else if (innerstream) {
			php_stream_close(innerstream);
		}
	}
	if (stream)
		php_stream_close(stream);
	if (self)
		efree(self);

	return NULL;
}

static php_stream_wrapper_ops gzip_stream_wops = {
	php_stream_gzopen,
	NULL, /* close */
	NULL, /* stat */
	NULL, /* stat_url */
	NULL, /* opendir */
	"ZLIB"
};

php_stream_wrapper php_stream_gzip_wrapper =	{
	&gzip_stream_wops,
	NULL,
	0, /* is_url */
};




/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
