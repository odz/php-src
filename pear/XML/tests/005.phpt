--TEST--
XML Parser: mixing character encodings
--FILE--
<?php // -*- C++ -*-
//
// Test for: XML/Parser.php
// Parts tested: - mixing character encodings
//
// This is what we test:
// 1 UTF-8      -> ISO-8859-1
// 2 UTF-8      -> US-ASCII
// 3 ISO-8859-1 -> UTF-8
// 4 ISO-8859-1 -> US-ASCII
// 5 US-ASCII   -> UTF-8
// 6 US-ASCII   -> ISO-8859-1
//

require_once "XML/Parser.php";

$xml = "<?xml version='1.0' ?>";
$input = array(
    "UTF-8"      => "<a>abcæøå</a>",
    "ISO-8859-1" => "<a>abc���</a>",
    "US-ASCII"   => "<a>abcaoa</a>"
);

$encodings = array_keys($input);
for ($i = 0; $i < sizeof($encodings); $i++) {
    for ($j = 0; $j < sizeof($encodings); $j++) {
	if ($i == $j) {
	    continue;
	}
	print "Testing $encodings[$i] -> $encodings[$j]: ";
	$p = new __TestEncodings1("UTF-8", "ISO-8859-1");
	$snippet = $input[$encodings[$i]];
	$e = $p->test($input[$encodings[$i]]);
	if (PEAR::isError($e)) {
	    printf("OOPS: %s\n", $e->getMessage());
	} else {
	    var_dump($e);
	}
    }
}

class __TestEncodings1 extends XML_Parser {
    var $output = '';

    function __TestEncodings1($to, $from) {
	$this->XML_Parser(array('input_encoding' => $from,
				'output_encoding' => $to,
				'case_folding' => false));
    }
    function startHandler($xp, $elem, $attribs) {
	$this->output .= "<$elem>";
    }
    function endHandler($xp, $elem) {
	$this->output .= "</$elem>";
    }
    function cdataHandler($xp, $data) {
	$this->output .= $data;
    }
    function test($data) {
	$this->output = '';
	$this->parseString($data, true);
	return $this->output;
    }
}

?>
--EXPECT--
