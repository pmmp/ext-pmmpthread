--TEST--
Test that readonly properties on copied classes work correctly
--SKIPIF--
<?php if(PHP_VERSION_ID < 80100) die("skip this test is for 8.1+ only"); ?>
--FILE--
<?php

class Test{
	public readonly int $b;

	public function __construct(
		public readonly int $a
	){}

	public function setA(int $a) : void{
		$this->a = $a;
	}
}

function test() : void{
	$t = new Test(1);
	var_dump($t->a);
	try{
		$t->setA(2);
	}catch(\Error $e){
		var_dump($e->getMessage());
	}
	try{
		$t->b = 3;
	}catch(\Error $e){
		var_dump($e->getMessage());
	}
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
int(1)
string(40) "Cannot modify readonly property Test::$a"
string(62) "Cannot initialize readonly property Test::$b from global scope"
int(1)
string(40) "Cannot modify readonly property Test::$a"
string(62) "Cannot initialize readonly property Test::$b from global scope"
