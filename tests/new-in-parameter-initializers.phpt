--TEST--
Test PHP 8.1 new in parameter initializers works correctly on copied functions
--SKIPIF--
<?php if(PHP_VERSION_ID < 80100) die("skip this test is for 8.1+ only"); ?>
--FILE--
<?php

function test(\stdClass $object = new \stdClass()){
	var_dump($object);
}

$t = new class extends \Thread{
	public function run() : void{
		test();
	}
};
$t->start() && $t->join();

test();

?>
--EXPECT--
object(stdClass)#2 (0) {
}
object(stdClass)#2 (0) {
}
