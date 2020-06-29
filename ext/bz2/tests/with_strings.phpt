--TEST--
BZ2 with strings
--SKIPIF--
<?php if (!extension_loaded("bz2")) print "skip"; ?>
--POST--
--GET--
--FILE--
<?php // $Id: with_strings.phpt,v 1.3 2002/03/20 02:16:23 sniper Exp $

error_reporting(E_ALL);

# This FAILS
$blaat = <<<HEREDOC
This is some random data
HEREDOC;

# This Works: (so, is heredoc related)
#$blaat= 'This is some random data';

$blaat2 = bzdecompress(bzcompress($blaat));

$tests = <<<TESTS
 \$blaat === \$blaat2
TESTS;

 include('tests/quicktester.inc');

--EXPECT--
OK
