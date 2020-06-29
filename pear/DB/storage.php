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
// DB_storage: a class that lets you return SQL data as objects that
// can be manipulated and that updates the database accordingly.
//

use "DB";

function DB_storage_destructor() {
    global $DB_storage_object_list;

    if (is_array($DB_storage_object_list)) {
	reset($DB_storage_object_list);
	while (list($ind, $obj) = each($DB_storage_object_list)) {
	    $obj->destroy();
	}
    }
}

class DB_storage {
    /** the name of the table (or view, if the backend database supports
        updates in views) we hold data from */
    var $_table = null;
    /** which column in the table contains primary keys */
    var $_keycolumn = null;
    /** DB connection handle used for all transactions */
    var $_dbh = null;
    /** an assoc with the names of database fields stored as properties
	in this object */
    var $_properties = array();
    /** an assoc with the names of the properties in this object that
	have been changed since they were fetched from the database */
    var $_changes = array();
    /** flag that decides if data in this object can be changed.
	objects that don't have their table's key column in their
	property lists will be flagged as read-only. */
    var $_readonly = false;

    /**
     * Constructor, adds itself to the DB_storage class's list of
     * objects that should have their "destroy" method called when
     * PHP shuts down (poor man's destructors).
     */
    function DB_storage($table, $keycolumn, &$dbh) {
	global $DB_storage_object_list;
	if (!is_array($DB_storage_object_list)) {
	    $DB_storage_object_list = array(&$this);
	}
	$DB_storage_object_list[] = &$this;
	$this->_table = $table;
	$this->_keycolumn = $keycolumn;
	$this->_dbh = $dbh;
	$this->_readonly = false;
    }

    /**
     * Static method used to create new DB storage objects.
     * @param $data assoc. array where the keys are the names
     *              of properties/columns
     * @return object a new instance of DB_storage (or inheriting class)
     */
    function &create($table, &$data) {
	$classname = get_class(&$this);
	$obj = new $classname($table);
	reset($data);
	while (list($name, $value) = each($data)) {
	    $obj->_properties[$name] = true;
	    $obj->$name = &$value;
	}
	return $obj;
    }

    /**
     * Loads data into this object from the given query.  If this
     * object already contains table data, changes will be saved and
     * the object re-initialized first.
     *
     * @param $query SQL query
     *
     * @param $params parameter list in case you want to use
     * prepare/execute mode
     *
     * @return int DB_OK on success, DB_WARNING_READ_ONLY if the
     * returned object is read-only (because the object's specified
     * key column was not found among the columns returned by $query),
     * or another DB error code in case of errors.
     */
    function loadFromQuery($query, $params = false) {
	if (sizeof($this->_properties)) {
	    if (sizeof($this->_changes)) {
		$this->store();
		$this->_changes = array();
	    }
	    $this->_properties = array();
	}
	$rowdata = $this->_dbh->getRow($query, DB_GETMODE_ASSOC, $params);
	if (DB::isError($rowdata)) {
	    return $rowdata;
	}
	reset($rowdata);
	$found_keycolumn = false;
	while (list($key, $value) = each($rowdata)) {
	    if ($key == $this->_keycolumn) {
		$found_keycolumn = true;
	    }
	    $this->_properties[$key] = true;
	    $this->$key = &$value;
	    unset($value); // have to unset, or all properties will
	    		   // refer to the same value
	}
	if (!$found_keycolumn) {
	    $this->_readonly = true;
	    return DB_WARNING_READ_ONLY;
	}
	return DB_OK;
    }

    function set($property, &$newvalue) {
	// only change if $property is known and object is not
	// read-only
	if (!$this->_readonly && isset($this->_properties[$property])) {
	    $this->$property = $newvalue;
	    $this->_changes[$property]++;
	    return true;
	}
	return false;
    }

    function &get($property) {
	// only return if $property is known
	if (isset($this->_properties[$property])) {
	    return $this->$property;
	}
	return null;
    }

    function destroy($discard = false) {
	if (!$discard && sizeof($this->_changes)) {
	    $this->store();
	}
	$this->_properties = array();
	$this->_changes = array();
	$this->_table = null;
    }

    function store() {
	while (list($name, $changed) = each($this->_changes)) {
	    $params[] = &$this->$name;
	    $vars[] = $name . ' = ?';
	}
	if ($vars) {
	    $query = 'UPDATE ' . $this->_table . ' SET ' .
		implode(', ', $vars) . ' WHERE id = ?';
	    $params[] = $this->id;
	    $stmt = $this->_dbh->prepare($query);
	    $res = $this->_dbh->execute($stmt, &$params);
	    if (DB::isError($res)) {
		return $res;
	    }
	    $this->_changes = array();
	}
	return DB_OK;
    }
}

?>
