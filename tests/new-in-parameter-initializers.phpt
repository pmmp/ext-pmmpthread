--TEST--
Test PHP 8.1 new in parameter initializers works correctly on copied functions
--SKIPIF--
<?php if(PHP_VERSION_ID < 80100) die("skip this test is for 8.1+ only"); ?>
--FILE--
<?php

function test(\stdClass $object = new \stdClass()){
	var_dump($object);
}

$t = new class extends \pmmp\thread\Thread{
	public function run() : void{
		test();
	}
};
$t->start(\pmmp\thread\Thread::INHERIT_ALL) && $t->join();

test();

?>
--EXPECT--
object(stdClass)#3 (0) {
}
object(stdClass)#3 (0) {
}
