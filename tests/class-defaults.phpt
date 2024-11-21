--TEST--
Test class defaults
--DESCRIPTION--
Class defaults should now initialize defaults properly
--FILE--
<?php
class Test extends \pmmp\thread\Thread {

	public function __construct(){
		$this->string = strrev($this->string);
	}

	public function run() : void{
		var_dump($this);
	}
	
	protected $string = "hello world";
	private $pstring  = "world hello";
	protected static $nocopy = true;
}

$test =new Test();
$test->start(\pmmp\thread\Thread::INHERIT_ALL);
$test->join();
?>
--EXPECT--
object(Test)#2 (2) {
  ["string":protected]=>
  string(11) "dlrow olleh"
  ["pstring":"Test":private]=>
  string(11) "world hello"
}
