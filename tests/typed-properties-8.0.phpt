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
string(53) "Cannot assign string to property Test::$a of type int"
string(58) "Cannot assign stdClass to property Test::$b of type ?Test2"
string(55) "Cannot assign string to property Test::$int of type int"
string(60) "Cannot assign stdClass to property Test::$obj of type ?Test2"
--- main thread end ---
--- worker thread start ---
string(53) "Cannot assign string to property Test::$a of type int"
string(58) "Cannot assign stdClass to property Test::$b of type ?Test2"
string(55) "Cannot assign string to property Test::$int of type int"
string(60) "Cannot assign stdClass to property Test::$obj of type ?Test2"
--- worker thread end ---
