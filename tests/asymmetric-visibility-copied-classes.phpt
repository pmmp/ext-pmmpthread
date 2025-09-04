--TEST--
Test PHP 8.4 asymmetric visibility is properly copied
--SKIPIF--
<?php if(PHP_VERSION_ID < 80400) die("skip PHP 8.4+ only"); ?>
--FILE--
<?php

use pmmp\thread\Thread;

class Test{

	public private(set) int $test = 1;

	public function setTest(int $value) : void{
		$this->test = $value;
	}
}

$t = new class extends Thread{
	public function run() : void{
		$test = new Test();

		var_dump($test->test);
		try{
			$test->test = 2;
			echo "Pretty sure this shouldn't work\n";
		}catch(\Error $e){
			echo $e->getMessage() . "\n";
		}
		$test->setTest(2);
		var_dump($test);
	}
};
$t->start(Thread::INHERIT_ALL);
$t->join();
?>
--EXPECT--
int(1)
Cannot modify private(set) property Test::$test from scope pmmp\thread\Thread@anonymous
object(Test)#3 (1) {
  ["test"]=>
  int(2)
}
