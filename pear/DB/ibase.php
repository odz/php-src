<?php
//
// +----------------------------------------------------------------------+
// | PHP version 4.0                                                      |
// +----------------------------------------------------------------------+
// | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group                   |
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
// $Id: ibase.php,v 1.5 2000/08/24 01:34:16 sterling Exp $
//
// Database independent query interface definition for PHP's Interbase
// extension.
//

include_once 'DB/common.php';

class DB_ibase extends DB_common {

	var $connection;
	var $phptype, $dbsyntax;
	var $prepare_tokens = array();
	var $prepare_types = array();

	function DB_ibase() {
		$this->phptype = 'ibase';
		$this->dbsyntax = 'ibase';
		$this->features = array(
			'prepare' => true,
			'pconnect' => true,
			'transactions' => true
		);
	}

	function connect (&$dsn, $persistant=false) {
		if(is_array($dsn)) {
			$dsninfo = &$dsn;
		} else {
			$dsninfo = DB::parseDSN($dsn);
		}
		if (!$dsninfo || !$dsninfo['phptype']) {
			return new DB_Error(); 
		}
		$user = $dsninfo['username'];
		$pw = $dsninfo['password'];
		$dbhost = $dsninfo['hostspec'] ? $dsninfo['hostspec'] : 'localhost';
		$connect_function = $persistent ? 'ibase_pconnect' : 'ibase_connect';
		if ($dbhost && $user && $pw) {
			$conn = $connect_function($dbhost, $user, $pw);
		} elseif ($dbhost && $user) {
			$conn = $connect_function($dbhost, $user);
		} elseif ($dbhost) {
			$conn = $connect_function($dbhost);
		} else {
			return new DB_Error();
		}
		$this->connection = $conn;
		return DB_OK;
	}

	function disconnect() {
		return @ibase_close($this->connection);
	}

	function &query( $stmt ) {
		$result = @ibase_query($this->connection, $stmt);
		if (!$result) {
			return new DB_Error();
		}
		// Determine which queries that should return data, and which
		// should return an error code only.
		if (preg_match('/(SELECT|SHOW|LIST|DESCRIBE)/i', $stmt)) {
			$resultObj = new DB_result($this, $result);
			return $resultObj;
		} else {
			return DB_OK;
		}
	}

	function simpleQuery($stmt) {
		$result = @ibase_query($this->connection, $stmt);
		if (!$result) {
			return new DB_Error();
		}
		// Determine which queries that should return data, and which
		// should return an error code only.
		return preg_match('/(SELECT|SHOW|LIST|DESCRIBE)/i', $stmt) ? $result : DB_OK;
	}

	function &fetchRow($result, $getmode=DB_GETMODE_DEFAULT) {
		if ($getmode & DB_GETMODE_ASSOC) {
			$row = (array)ibase_fetch_object($result);
		} else {
			$row = ibase_fetch_row($result);
		}
		if (!$row) {
			return new DB_Error();
		}
		return $row;
	}

	function fetchInto($result, &$ar, $getmode=DB_GETMODE_DEFAULT) {
		if ($getmode & DB_GETMODE_ASSOC) {
			return new DB_Error(DB_ERROR_NOT_CAPABLE);
		} else {
			$ar = ibase_fetch_row($result);
		}
		if (!$ar) {
			return new DB_Error();
		}
		return DB_OK;
	}

	function freeResult() {
		return new DB_Error(DB_ERROR_NOT_CAPABLE);
	}

	function freeQuery($query) {
		ibase_free_query($query);
		return true;
	} 

	function numCols($result) {
		$cols = ibase_num_fields($result);
		if (!$cols) {
			return new DB_Error();
		}
		return $cols;
	}

	function prepare($query) {
		return ibase_prepare($query);
	}

	function execute($stmt, $data = false) {
		$result = ibase_execute($stmt, $data);
		if (!$result) {
			return new DB_Error();
		}
		return preg_match('/(SELECT|SHOW|LIST|DESCRIBE)/i', $stmt) ? $result : DB_OK;
	}

	function autoCommit($onoff=false) {
		return new DB_Error(DB_ERROR_NOT_CAPABLE);
	}

	function commit() {
		return ibase_commit($this->connection);
	}

	function rollback($trans_number) {
		return ibase_rollback($this->connection,$trans_number);
	}

	function transactionInit($trans_args=0) {
		return $trans_args ? ibase_trans($trans_args, $this->connection) : ibase_trans();
	}
}
?>
