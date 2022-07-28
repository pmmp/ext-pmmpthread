--TEST--
Test class defaults
--DESCRIPTION--
Class defaults should now initialize defaults properly
--FILE--
<?php
class Test extends Thread {

	public function run() : void{
		var_dump($this);
	}
	
	protected $string = "hello world";
	private $pstring  = "world hello";
	protected static $nocopy = true;
}

$test =new Test();
$test->string = strrev($test->string);
$test->start();
$test->join();
?>
--EXPECTF--
object(Test)#%d (%d) {
  ["string"]=>
  string(11) "dlrow olleh"
  ["pstring"]=>
  string(11) "world hello"
}


