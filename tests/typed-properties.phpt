--TEST--
Test that typed properties on copied classes work properly
--SKIPIF--
<?php if(PHP_VERSION_ID < 70400) die("skip: this test is for 7.4+ only"); ?>
--FILE--
<?php

class Test{
	public static int $a = 1;
	public static ?Test2 $b = null;

	public int $int = 1;
	public ?Test2 $obj = null;
}

class Test2 extends Test{

}

function testStatics() : void{
	Test::$a = 1;
	try{
		Test::$a = "bang";
	}catch(\TypeError $e){
		var_dump($e->getMessage());
	}
	Test::$b = null;
	Test::$b = new Test2;
	try{
		Test::$b = new \stdClass;
	}catch(\TypeError $e){
		var_dump($e->getMessage());
	}
}

function testNonStatics() : void{
	$a = new Test;
	$a->int = 1;
	try{
		$a->int = "bang";
	}catch(\TypeError $e){
		var_dump($e->getMessage());
	}

	$a->obj = null;
	$a->obj = new Test2;
	try{
		$a->obj = new \stdClass;
	}catch(\TypeError $e){
		var_dump($e->getMessage());
	}
}

echo "--- main thread start ---\n";
testStatics();
testNonStatics();
echo "--- main thread end ---\n";

$w = new Worker;
$w->start();
$w->stack(new class extends \Threaded{
	public function run() : void{
		echo "--- worker thread start ---\n";
		testStatics();
		testNonStatics();
		echo "--- worker thread end ---\n";
	}
});
$w->shutdown();
	
--EXPECT--
--- main thread start ---
string(48) "Typed property Test::$a must be int, string used"
string(75) "Typed property Test::$b must be an instance of Test2 or null, stdClass used"
string(50) "Typed property Test::$int must be int, string used"
string(77) "Typed property Test::$obj must be an instance of Test2 or null, stdClass used"
--- main thread end ---
--- worker thread start ---
string(48) "Typed property Test::$a must be int, string used"
string(75) "Typed property Test::$b must be an instance of Test2 or null, stdClass used"
string(50) "Typed property Test::$int must be int, string used"
string(77) "Typed property Test::$obj must be an instance of Test2 or null, stdClass used"
--- worker thread end ---
