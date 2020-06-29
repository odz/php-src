<?php
/**
* Superclass of all Renderer. 
*
* Derive all custom renderer from this class.
*
* @version $Id: PhpdocRendererObject.php,v 1.4 2001/02/18 16:29:20 uw Exp $
*/
class PhpdocRendererObject extends PhpdocObject {

    var $warn;

    var $accessor;

    /**
    * Extension for generated files.
    * @var  string  
    */
    var $file_extension = ".html";

} // end class PhpdocRendererObject
?>