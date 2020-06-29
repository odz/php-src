--TEST--
DB_oci8::errorNative test
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
require_once "DB.php";
include("mktable.inc");
$dbh->query("syntax error please");
print $dbh->errorNative() . "\n";
?>
--EXPECT--
XXX fill me in
