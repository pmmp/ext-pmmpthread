--TEST--
Test that intersection types work correctly
--DESCRIPTION--
PHP 8.1 added support for intersection types, we need to ensure they work correctly on copied code
--SKIPIF--
<?php if(PHP_VERSION_ID < 80100) die("skip this test is for 8.1+ only"); ?>
--FILE--
<?php

interface A{}

interface B{}

class AAndB implements A, B{}

class JustA implements A{}

class JustB implements B{}

function acceptIntersection(A&B $aAndB) : A&B{
	return $aAndB;
}

function test() : void{
	var_dump(acceptIntersection(new AAndB()));

	try{
		var_dump(acceptIntersection(new JustA()));
	}catch(\TypeError $e){
		echo $e->getMessage() . PHP_EOL;
	}
	try{
		var_dump(acceptIntersection(new JustB()));
	}catch(\TypeError $e){
		echo $e->getMessage() . PHP_EOL;
	}
}

echo "--- main thread start ---\n";
test();
echo "--- main thread end ---\n";

$t = new class extends \Thread{
	public function run() : void{
		echo "--- child thread start ---\n";
		test();
		echo "--- child thread end ---\n";
	}
};
$t->start();
$t->join();

echo "OK\n";
?>
--EXPECTF--
--- main thread start ---
object(AAndB)#1 (0) {
}
acceptIntersection(): Argument #1 ($aAndB) must be of type A&B, JustA given, called in %s on line %d
acceptIntersection(): Argument #1 ($aAndB) must be of type A&B, JustB given, called in %s on line %d
--- main thread end ---
--- child thread start ---
object(AAndB)#2 (0) {
}
acceptIntersection(): Argument #1 ($aAndB) must be of type A&B, JustA given, called in %s on line %d
acceptIntersection(): Argument #1 ($aAndB) must be of type A&B, JustB given, called in %s on line %d
--- child thread end ---
OK
