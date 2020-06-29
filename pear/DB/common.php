<?php
//
// +----------------------------------------------------------------------+
// | PHP version 4.0                                                      |
// +----------------------------------------------------------------------+
// | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group                   |
// +----------------------------------------------------------------------+
// | This source file is subject to version 2.0 of the PHP license,       |
// | that is bundled with this package in the file LICENSE, and is        |
// | available at through the world-wide-web at                           |
// | http://www.php.net/license/2_0.txt.                                  |
// | If you did not receive a copy of the PHP license and are unable to   |
// | obtain it through the world-wide-web, please send a note to          |
// | license@php.net so we can mail you a copy immediately.               |
// +----------------------------------------------------------------------+
// | Authors: Stig Bakken <ssb@fast.no>                                   |
// |                                                                      |
// +----------------------------------------------------------------------+
//
// Base class for DB implementations.
//

//
// XXX legend:
//
// XXX ERRORMSG: Needs a better way of dealing with errors.
// XXX ADDREF:	 As soon as Zend/PHP gets support for returning
//				 references, this return value should be made into
//				 a reference.
//

if (!empty($GLOBALS['USED_PACKAGES']['DB/common'])) return;
$GLOBALS['USED_PACKAGES']['DB/common'] = true;

/**
 * DB_common is a base class for DB implementations, and should be
 * inherited by all such.
 */
class DB_common {
    // {{{ properties

	var $features;		// assoc of capabilities for this DB implementation
	var $errorcode_map;	// assoc mapping native error codes to DB ones
	var $type;			// DB type (mysql, oci8, odbc etc.)

    // }}}
    // {{{ constructor

	function DB_common() {
		$this->features = array();
		$this->errorcode_map = array();
	}

    // }}}
    // {{{ quoteString()

	/**
	 * Quotes a string so it can be safely used within string delimiters
	 * in a query.
	 *
	 * @param $string the input string to quote
	 *
	 * @return string the quoted string
	 */
	function quoteString($string) {
		return str_replace("'", "\'", $string);
	}

	// }}}
	// {{{ provides()

	/**
	 * Tell whether a DB implementation or its backend extension
	 * supports a given feature.
	 *
	 * @param $feature name of the feature (see the DB class doc)
	 *
	 * @return bool whether this DB implementation supports $feature
	 */
	function provides($feature) {
		return $this->features[$feature];
	}

	// }}}
	// {{{ errorCode()

	/**
	 * Map native error codes to DB's portable ones.  Requires that
	 * the DB implementation's constructor fills in the $errorcode_map
	 * property.
	 *
	 * @param $nativecode the native error code, as returned by the backend
	 * database extension (string or integer)
	 *
	 * @return int a portable DB error code, or FALSE if this DB
	 * implementation has no mapping for the given error code.
	 */
	function errorCode($nativecode) {
		if ($this->errorcode_map[$nativecode]) {
			return $this->errorcode_map[$nativecode];
		}
		//php_error(E_WARNING, get_class($this)."::errorCode: no mapping for $nativecode");
		// Fall back to DB_ERROR if there was no mapping.  Ideally,
		// this should never happen.
		return DB_ERROR;
	}

	// }}}
	// {{{ errorMessage()

	/**
	 * Map a DB error code to a textual message.  This is actually
	 * just a wrapper for DB::errorMessage().
	 *
	 * @param $dbcode the DB error code
	 *
	 * @return string the corresponding error message, of FALSE
	 * if the error code was unknown
	 */
	function errorMessage($dbcode) {
		return DB::errorMessage($this->errorcode_map[$dbcode]);
	}

	// }}}

    // {{{ prepare()

	/**
	 * Prepares a query for multiple execution with execute().  With
	 * PostgreSQL, this is emulated.
	 */
	function prepare($query) {
		$tokens = split('[\&\?]', $query);
		$token = 0;
		$types = array();
		for ($i = 0; $i < strlen($query); $i++) {
			switch ($query[$i]) {
				case '?':
					$types[$token++] = DB_PARAM_SCALAR;
					break;
				case '&':
					$types[$token++] = DB_PARAM_OPAQUE;
					break;
			}
		}
		$this->prepare_tokens[] = &$tokens;
		end($this->prepare_tokens);
		$k = key($this->prepare_tokens);
		$this->prepare_types[$k] = $types;
		return $k;
	}

    // }}}
    // {{{ execute_emulate_query()

	/**
	 * @return a string containing the real query run when emulating
	 * prepare/execute.  A DB error code is returned on failure.
	 */
	function execute_emulate_query($stmt, $data = false) {
		$p = &$this->prepare_tokens;
		$stmt = 0; // XXX HORRIBLE HACK
		if (!isset($this->prepare_tokens[$stmt]) ||
			!is_array($this->prepare_tokens[$stmt]) ||
			!sizeof($this->prepare_tokens[$stmt])) {
			return DB_ERROR_INVALID;
		}
		$qq = &$this->prepare_tokens[$stmt];
		$qp = sizeof($qq) - 1;
		if ((!$data && $qp > 0) ||
			(!is_array($data) && $qp > 1) ||
			(is_array($data) && $qp > sizeof($data))) {
			return DB_ERROR_NEED_MORE_DATA;
		}
		$realquery = $qq[0];
		for ($i = 0; $i < $qp; $i++) {
			if ($this->prepare_types[$stmt][$i] == DB_PARAM_OPAQUE) {
				if (is_array($data)) {
					$fp = fopen($data[$i], "r");
				} else {
					$fp = fopen($data, "r");
				}
				$pdata = '';
				if ($fp) {
					while (($buf = fread($fp, 4096)) != false) {
						$pdata .= $buf;
					}
				}
			} else {
				if (is_array($data)) {
					$pdata = &$data[$i];
				} else {
					$pdata = &$data;
				}
			}
			$realquery .= "'" . $this->quoteString($pdata) . "'";
			$realquery .= $qq[$i + 1];
		}
		return $realquery;
	}

    // }}}

    // {{{ executeMultiple()

	/**
	 * This function does several execute() calls on the same
	 * statement handle.  $data must be an array indexed numerically
	 * from 0, one execute call is done for every "row" in the array.
	 *
	 * If an error occurs during execute(), executeMultiple() does not
	 * execute the unfinished rows, but rather returns that error.
	 */
	function executeMultiple($stmt, &$data) {
		for ($i = 0; $i < sizeof($data); $i++) {
			$res = $this->execute($stmt, &$data[$i]);
			if (DB::isError($res)) {
				return $res;
			}
		}
		return DB_OK;
	}

    // }}}
    // {{{ getOne()

	/**
	 * Fetch the first column of the first row of data returned from
	 * a query.  Takes care of doing the query and freeing the results
	 * when finished.
	 *
	 * @param $query the SQL query
	 * @param $params is supplies, prepare/execute will be used
	 *        with this array as execute parameters
	 */
	function &getOne($query, $params = array()) {
		if (sizeof($params) > 0) {
			$sth = $this->prepare($query);
			if (DB::isError($sth)) {
				return $sth;
			}
			$res = $this->execute($sth, &$params);
		} else {
			$res = $this->simpleQuery($query);
		}
		if (DB::isError($res)) {
			return $res;
		}
		$row = $this->fetchRow($res, DB_GETMODE_ORDERED);
		if (DB::isError($row)) {
			return $row;
		}
		$ret = &$row[0];
		$this->freeResult($res);
		return $ret;
	}

    // }}}
    // {{{ getRow()

	/**
	 * Fetch the first row of data returned from a query.  Takes care
	 * of doing the query and freeing the results when finished.
	 *
	 * @param $query the SQL query
	 * @return array the first row of results as an array indexed from
	 * 0, or a DB error code.
	 */
	function &getRow($query, $getmode = DB_GETMODE_DEFAULT, $params = array()) {
		$res = $this->simpleQuery($query);
		if (DB::isError($res)) {
			return $res;
		}
		$row = $this->fetchRow($res, $getmode);
		if (DB::isError($row)) {
			return $row;
		}
		$this->freeResult($res);
		return $row;
	}

    // }}}
    // {{{ getAssoc()

	/**
	 * Fetch the entire result set of a query and return it as an
	 * associative array using the first column as the key.
	 *
	 * @param $query the SQL query
	 *
	 * @param $force_array (optional) used only when the query returns
	 * exactly two columns.  If true, the values of the returned array
	 * will be one-element arrays instead of scalars.
	 *
	 * @return array associative array with results from the query.
	 * If the result set contains more than two columns, the value
	 * will be an array of the values from column 2-n.  If the result
	 * set contains only two columns, the returned value will be a
	 * scalar with the value of the second column (unless forced to an
	 * array with the $force_array parameter).  A DB error code is
	 * returned on errors.  If the result set contains fewer than two
	 * columns, DB_ERROR_TRUNCATED is returned.
	 *
	 * For example, if the table "mytable" contains:
	 *
	 *  ID      TEXT       DATE
	 * --------------------------------
	 *  1       'one'      944679408
	 *  2       'two'      944679408
	 *  3       'three'    944679408
	 *
	 * Then the call getAssoc('SELECT id,text FROM mytable') returns:
	 *   array(
	 *     '1' => 'one',
	 *     '2' => 'two',
	 *     '3' => 'three',
	 *   )
	 *
	 * ...while the call getAssoc('SELECT id,text,date FROM mydate') returns:
	 *   array(
	 *     '1' => array('one', '944679408'),
	 *     '2' => array('two', '944679408'),
	 *     '3' => array('three', '944679408')
	 *   )
	 *
	 * Keep in mind that database functions in PHP usually return string
	 * values for results regardless of the database's internal type.
	 */
	function &getAssoc($query, $force_array = false, $params = array()) {
		$res = $this->simpleQuery($query);
		if (DB::isError($res)) {
			return $res;
		}
		$cols = $this->numCols($res);
		if ($cols < 2) {
			return DB_ERROR_TRUNCATED;
		}
		$results = array();
		if ($cols > 2 || $force_array) {
			// return array values
			// XXX this part can be optimized
			while (!DB::isError($row = $this->fetchRow($res))) {
				reset($row);
				// we copy the row of data into a new array
				// to get indices running from 0 again
				for ($i = 1; $i < $cols; $i++) {
					$results[$row[0]][$i-1] = $row[$i];
				}
			}
		} else {
			// return scalar values
			while (!DB::isError($row = $this->fetchRow($res))) {
				$results[$row[0]] = $row[1];
			}
		}
		return $results; // XXX ADDREF
	}

    // }}}
	// {{{ getAll()

	function &getAll($query, $getmode = DB_GETMODE_DEFAULT, $params = array()) {
				
	}

	// }}}
}

// Local variables:
// tab-width: 4
// c-basic-offset: 4
// End:
?>
