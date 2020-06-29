<?php

/* vim: set expandtab tabstop=4 shiftwidth=4; */
// +---------------------------------------------------------------------+
// |  PHP version 4.0                                                    |
// +---------------------------------------------------------------------+
// |  Copyright (c) 1997-2001 The PHP Group                              |
// +---------------------------------------------------------------------+
// |  This source file is subject to version 2.0 of the PHP license,     |
// |  that is bundled with this package in the file LICENSE, and is      |
// |  available through the world-wide-web at                            |
// |  http://www.php.net/license/2_02.txt.                               |
// |  If you did not receive a copy of the PHP license and are unable to |
// |  obtain it through the world-wide-web, please send a note to        |
// |  license@php.net so we can mail you a copy immediately.             |
// +---------------------------------------------------------------------+
// |  Authors:  Sean Grimes <metallic@noworlater.net>                    |
// +---------------------------------------------------------------------+
// 
// $Id: Render.php,v 1.2 2001/01/16 00:27:34 metallic Exp $

/**
* Render class for rendering from XML. 
*
* This class should render documents from xml.
* The intended rendering modes will be HTML and
* the Adobe PDF format. Maybe at some point I 
* will make it possible to take a document from
* HTML to PDF, but this is unlikely. 
* 
* @author Sean Grimes <metallic@noworlater.net>
*/ 

/*** Todo ***
 ** - Implement the HTML and PDF rendering modes
 ** - Extend the parse() function to what is needed
 ** - Implement filesystem commands
 ** - Come up with the XML language syntax
 ** - Provide a better class interface 
 ** - Do some debugging
***/

require_once "Parser.php";

class XML_Render extends XML_Parser {

    var $data; // holds the file contents

	function XML_Render($charset = 'UTF-8', $mode = "event") {
		$this->XML_Parser($charset, $mode);

	}

	/**
	* Set the input file.
	*
	* This overrides XML_Parser::setInputFile(),
	* and creates a wrapper around XML_Parser::setInputFile()
	* and XML_Parser::inputFile(). The functions are so similar
	* that its confusing me(hint: document XML_Parser).
	*
	* @access public
	* @param $file file to be input
	*/
	function setInputFile($file) {
		$fp = @fopen($file, "r");
		if (is_resource($fp)) {
			$this->fp = $fp;
			return $this->setInput($file);
		} else {
			return new XML_Parser_Error($php_errormsg); // ??
		}
	}

    /** 
    * Parses the document
    *
    * This function extends the capabilities of the 
    * parse function in XML/Parser.php. The only real
    * notable change is the addition of a command to 
    * store the contents of the file to a variable to
    * make it available to the entire class
    */
    function parse() {
        if(!is_resource($this->fp)) {
            return new XML_Parser_Error("no input");
        }
        if (!is_resource($this->parser)) {
            return new XML_Parser_Error("no parser");
        }
        while ($data = fread($this->fp, 2048)) {
            $err = $this->parseString($data, $feof($this->fp));
            if (PEAR::isError($err)) {
                return $err;
            }
        }
        $this->data = $data;
        return $true;
    }
        

	/**
	* Renders the XML document.
	*
	* This function really isnt implemented yet. 
	* Basically, I just added the calls for the HTML
	* and PDF subclass rendering modes. I'm hoping this
	* class will be easily built onto over PEAR's lifetime.
	*
	* @param $mode Rendering mode. Defaults to HTML.
	* @author Sean Grimes <metallic@noworlater.net>
	*/
 
	function render($mode = 'HTML') {
		if($mode == 'HTML') {
			$html = new XML_Render_HTML();
			$html->render();
		}
		if($mode == 'PDF') {
			$pdf = new XML_Render_PDF();
			$pdf->render();
		} else {
			$message = "Error. Unsupported rendering mode.";
			new PEAR_Error($message, 0, PEAR_ERROR_RETURN, E_USER_NOTIFY);
		}
	}
}
