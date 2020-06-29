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
// | Authors: Sterling Hughes <sterling@designmultimedia.com>             |
// |                                                                      |
// +----------------------------------------------------------------------+
//
// $Id:
//
// Commonly needed functions searching directory trees
//

define("N_PATH", "Invalid Path");
define("N_OPEN", "Cannot Open File");
define("N_WRITE", "Cannot Write to file");
define("N_CLOSE", "Cannot Close file or directory");

class File_Find {

var $errmess; // Error message is stored here
var $err; // if there was an error this is set to 1, otherwise it is set to 0

/*
 *  array glob ( string pattern, string directory_path[, string pattern_type]) -- will search the current directory to find matches
 *  for the specified pattern.  It only searches the current directory and directory names are included in the search.  If you are looking to
 *  search a directory tree then see the search method.
 */

function glob ( $pattern, $dirpath, $pattern_type="PHP" ) {
    $dh = @opendir($dirpath) or File_Find::_RaiseException(N_PATH);
    if ( strtolower($pattern_type) == "perl" ) {
        while ($entry = readdir($dh)) {
            if ( preg_match($pattern,$entry) && $entry != '.' && $entry != '..' ) {
                $matches[] = $entry;
            }
        }
    } else {
        while ($entry = readdir($dh)) {
            if ( strtolower(substr($pattern, -2)) == "/i") {
                if (eregi(substr($pattern,0,-2), $entry)) {
                    $matches[] = $entry;
                }
            } else {
                if (ereg($pattern, $entry)) {
                    $matches[] = $entry;
                }
            }
        }
    }
    @closedir($dh);
    return $matches;
}

var $_dirs;
var $files = array();
var $directories = array();

/*
 *  array maptree ( string directory_path ) -- Will return two arrays the first a listing of all the sub directories under the 
 *  specified directory and the second listing all the files under the specified directory.
 */
function maptree ( $directory ) {
    $this->directories = array();
    $this->_dirs = array($directory);

    while (count($this->_dirs)) {
        $t = array_pop($this->_dirs); 
        File_Find::_build( $t );
        array_push($this->directories, $t);
    }
    return array($this->directories, $this->files);
}


/* internal function to build singular directory trees */

function _build ( $directory ) {
    $dh = @opendir($directory) or File_Find::_RaiseException(N_PATH);
    while ( $entry = readdir($dh) ) {
        if ($entry != '.' && $entry != '..') {
            $entry = $directory . $entry;
            if ( is_dir($entry) ) {
                $ent_name = $entry . '/';
                array_push($this->_dirs, $ent_name);
            } else {
                array_push($this->files, $entry);
            }
        }
    }
    @closedir($dh);
}

/*
 *   array search ( string pattern, string directory [, string regex_type] ) -- Will search an entire directory tree for a pattern specified 
 *   by the 'pattern' argument, in a directory specified by the 'directory' argument, of a pattern type specified by 'regex_type'.  If 
 *   regex_type is left out than PHP type regular expressions will be used.  If you want to specify the use of eregi instead of ereg attach a 
 *  '/i' onto the very end of your regular expression.
 */

function search ( $pattern, $directory, $type="PHP" ) {
    File_Find::maptree($directory);
    if ( strtolower($type) == "perl" ) {
        while (list(,$entry) = each($this->files)) {
            if ( preg_match($pattern,$entry) && $entry != '.' && $entry != '..' ) {
                $matches[] = $entry;
            }
        }
    } else {
        while (list(,$entry) = each($this->files)) {
            if ( strtolower(substr($pattern, -2)) == "/i") {
                if (eregi(substr($pattern,0,-2), $entry)) {
                    $matches[] = $entry;
                }
            } else {
                if (ereg($pattern, $entry)) {
                    $matches[] = $entry;
                }
            }
        }
    }
    return $matches;

}

function _RaiseException ( $error_message ) {
    $this->errmess = $error_message;
    $this->err = 1;
}

/*
 *   double File_Find_version(void) -- Returns the current version of File_Find
 */
function File_Find_version() {
    return 1.0;
}

//End Class
}
?>

<?php
/* EXAMPLES FOR THIS CLASS

Mapping an entire directory structure:

require('File/Find.php');
$searcher = new File_Find;
list ($directories,$files) = $searcher->maptree('/pear');
print implode("\n<br>\n", $directories);

(Yes That's it)

Searching an entire directory structure:

require('File/Find.php');
$fs = new File_Find;
$files = $fs->search('/\.c$/','/cvs/php4', 'perl');
print implode("\n<br>\n", $files);

Searching a singular directory:

require('File/Find.php');
$fs = new File_Find;
$files = $fs->glob('/.*/', '/cvs/', 'perl');
print implode("\n<br>\n", $files);

Usages:
Filecrawler (I built a complete crawler in 20 lines)
Removing a directory tree.
...

END EXAMPLES

*/
?>