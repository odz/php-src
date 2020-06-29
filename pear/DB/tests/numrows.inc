<?php

function errhandler($obj)
{
    $msg = $obj->getMessage();
    print "$msg";
    if (substr($msg, -1) != "\n") {
        print "\n";
    }
}

if (isset($test_error_mode)) {
    $dbh->setErrorHandling($test_error_mode);
} else {
    $dbh->setErrorHandling(PEAR_ERROR_DIE);
}
$sth = $dbh->query("SELECT a FROM phptest");
printf("%d\n", $sth->numRows());
for ($i = 0; $i < 5; $i++) {
    $sth = $dbh->query("INSERT INTO phptest (a) VALUES($i)");
    $sth = $dbh->query("SELECT a FROM phptest");
    printf("%d\n", $sth->numRows());
}
$dbh->query("DELETE FROM phptest WHERE a < 4");
$sth = $dbh->query("SELECT a FROM phptest");
printf("%d\n", $sth->numRows());
$sth = $dbh->query("SELECT a FROM phptest where a < 0");
printf("%d\n", $sth->numRows());
$dbh->setErrorHandling(PEAR_ERROR_CALLBACK, "errhandler");
$test = $dbh->numRows(false);

?>
