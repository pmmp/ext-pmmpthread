--TEST--
Test that PHP 8.3 static variable initializers work correctly
--SKIPIF--
<?php if(PHP_VERSION_ID < 80300) die("skip this test is for 8.3+ only"); ?>
--FILE--
<?php

use pmmp\thread\Thread;

function test() : void{
	static $var = sprintf("%s %s", "hello", "world");

	var_dump($var);
}

$t = new class extends Thread{
	public function run() : void{
		test();
	}
};
$t->start(Thread::INHERIT_ALL) && $t->join();
?>
--EXPECT--
string(11) "hello world"
