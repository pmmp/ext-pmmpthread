--TEST--
Test global constants
--DESCRIPTION--
This test verifies that global constants are inherited
--FILE--
<?php
define ("SCONST", "mystring");
define ("LCONST", 10);
define ("DCONST", 1.19);
define ("NCONST", null);
define ("BCONST", true);

class TestThread extends \pmmp\thread\Thread {
	public function run() : void{
		foreach (array(
			"string" => SCONST,
			"long" => LCONST,
			"double" => DCONST,
			"null" => NCONST,
			"boolean" => BCONST
		) as $key => $constant) {
			printf("%s:", $key);
			var_dump($constant);
		}
	}
}

$thread = new TestThread();
$thread->start(\pmmp\thread\Thread::INHERIT_ALL);
?>
--EXPECT--
string:string(8) "mystring"
long:int(10)
double:float(1.19)
null:NULL
boolean:bool(true)
