--TEST--
Test reading object properties without debug info
--DESCRIPTION--
This test verifies that reading properties from the object without var_dump/print_r will work
--FILE--
<?php
class Test extends \pmmp\thread\Thread {
	public function run() : void{ 
		$this->name = sprintf("%s", __CLASS__);
	}
}

$thread = new Test();
$thread->start(\pmmp\thread\Thread::INHERIT_ALL);
$thread->join();
foreach($thread as $key => $value)
	var_dump($value);
?>
--EXPECT--
string(4) "Test"
