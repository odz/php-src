--TEST--
DB_oci8::numRows test
--SKIPIF--
<?php require "skipif.inc"; ?>
--FILE--
<?php
require "connect.inc";
require "mktable.inc";
$test_error_mode = PEAR_ERROR_PRINT;
include "../numrows.inc";
?>
--EXPECT--
DB Error: DB backend not capable
DB Error: DB backend not capable
DB Error: DB backend not capable
DB Error: DB backend not capable
DB Error: DB backend not capable
DB Error: DB backend not capable
DB Error: DB backend not capable
DB Error: DB backend not capable
DB Error: unknown error
