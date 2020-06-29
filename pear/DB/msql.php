<?php
//
// +----------------------------------------------------------------------+
// | PHP version 4.0                                                      |
// +----------------------------------------------------------------------+
// | Copyright (c) 1997-2001 The PHP Group                                |
// +----------------------------------------------------------------------+
// | This source file is subject to version 2.02 of the PHP license,      |
// | that is bundled with this package in the file LICENSE, and is        |
// | available at through the world-wide-web at                           |
// | http://www.php.net/license/2_02.txt.                                 |
// | If you did not receive a copy of the PHP license and are unable to   |
// | obtain it through the world-wide-web, please send a note to          |
// | license@php.net so we can mail you a copy immediately.               |
// +----------------------------------------------------------------------+
// | Authors: Sterling Hughes <sterling@php.net>                          |
// +----------------------------------------------------------------------+
//
// $Id: msql.php,v 1.16 2001/02/19 12:22:26 ssb Exp $
//
// Database independent query interface definition for PHP's Mini-SQL
// extension.
//

require_once 'DB/common.php';

class DB_msql extends DB_common
{
    var $connection;
    var $phptype, $dbsyntax;
    var $prepare_tokens = array();
    var $prepare_types = array();

    function DB_msql()
    {
        $this->DB_common();
        $this->phptype = 'msql';
        $this->dbsyntax = 'msql';
        $this->features = array(
            'prepare' => false,
            'pconnect' => true,
            'transactions' => false
        );
    }

    function connect($dsn, $persistent = false)
    {
        if(is_array($dsn)) {
            $dsninfo = &$dsn;
        } else {
            $dsninfo = DB::parseDSN($dsn);
        }
        if (!$dsninfo || !$dsninfo['phptype']) {
            return $this->raiseError(); 
        }
        $this->dsn = $dsninfo;
        $user = $dsninfo['username'];
        $pw = $dsninfo['password'];
        $dbhost = $dsninfo['hostspec'] ? $dsninfo['hostspec'] : 'localhost';
        $connect_function = $persistent ? 'msql_pconnect' : 'msql_connect';
        if ($dbhost && $user && $pw) {
            $conn = $connect_function($dbhost, $user, $pw);
        } elseif ($dbhost && $user) {
            $conn = $connect_function($dbhost,$user);
        } else {
            $conn = $connect_function($dbhost);
        }
        if ($dsninfo['database']) {
            @msql_select_db($dsninfo['database'], $conn);
        } else {
            return $this->raiseError();
        }
        $this->connection = $conn;
        return DB_OK;
    }

    function disconnect()
    {
        return @msql_close($this->connection);
    }

    function simpleQuery($query)
    {
	$this->last_query = $query;
        $query = $this->modifyQuery($query);
        $result = @msql_query($query, $this->connection);
        if (!$result) {
            return $this->raiseError();
        }
        // Determine which queries that should return data, and which
        // should return an error code only.
        return DB::isManip($query) ? DB_OK : $result;
    }

    function &fetchRow($result, $fetchmode=DB_FETCHMODE_DEFAULT)
    {
	if ($fetchmode == DB_FETCHMODE_DEFAULT) {
	    $fetchmode = $this->fetchmode;
	}
        if ($fetchmode & DB_FETCHMODE_ASSOC) {
            $row = @msql_fetch_array($result, MSQL_ASSOC);
        } else {
            $row = @msql_fetch_row($result);
        }
        if (!$row) {
	    if ($error = msql_error()) {
		return $this->raiseError($error);
	    } else {
		return null;
	    }
        }
	
        return $row;
    }

    function fetchInto($result, &$ar, $fetchmode=DB_FETCHMODE_DEFAULT)
    {
	if ($fetchmode == DB_FETCHMODE_DEFAULT) {
	    $fetchmode = $this->fetchmode;
	}
        if ($fetchmode & DB_FETCHMODE_ASSOC) {
            $ar = @msql_fetch_array($result, MSQL_ASSOC);
        } else {
            $ar = @msql_fetch_row($result);
        }
        if (!$ar) {
            return $this->raiseError();
        }
        return DB_OK;
    }

    function freeResult($result)
    {
        if (is_resource($result)) {
            return @msql_free_result($result);
        }
        if (!isset($this->prepare_tokens[$result])) {
            return false;
        }
        unset($this->prepare_tokens[$result]);
        unset($this->prepare_types[$result]);
        return true; 
    }

    function numCols($result)
    {
        $cols = @msql_num_fields($result);
        if (!$cols) {
            return $this->raiseError();
        }
        return $cols;
    }

    function numRows($result)
    {
        $rows = @msql_num_rows($result);
        if (!$rows) {
            return $this->raiseError();
        }
        return $rows;
    }
}

?>
