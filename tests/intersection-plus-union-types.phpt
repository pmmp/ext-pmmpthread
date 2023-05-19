--TEST--
Test that combined intersection and union types work correctly
--DESCRIPTION--
PHP 8.2 added support for combined intersection and union types, we need to ensure they work correctly on copied code
--SKIPIF--
<?php if(PHP_VERSION_ID < 80200) die("skip this test is for 8.2+ only"); ?>
--FILE--
<?php

interface A{}

interface B{}

class AAndB implements A, B{}

class JustA implements A{}

class JustB implements B{}

class C{}

class D{}

function acceptIntersection((A&B)|C $aAndBOrC) : (A&B)|C{
	return $aAndBOrC;
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
	var_dump(acceptIntersection(new C()));
	try{
		var_dump(acceptIntersection(new D()));
	}catch(\TypeError $e){
		echo $e->getMessage() . PHP_EOL;
	}
}

echo "--- main thread start ---\n";
test();
echo "--- main thread end ---\n";

$t = new class extends \pmmp\thread\Thread{
	public function run() : void{
		echo "--- child thread start ---\n";
		test();
		echo "--- child thread end ---\n";
	}
};
$t->start(\pmmp\thread\Thread::INHERIT_ALL);
$t->join();

echo "OK\n";
?>
--EXPECTF--
--- main thread start ---
object(AAndB)#1 (0) {
}
acceptIntersection(): Argument #1 ($aAndBOrC) must be of type (A&B)|C, JustA given, called in %s on line %d
acceptIntersection(): Argument #1 ($aAndBOrC) must be of type (A&B)|C, JustB given, called in %s on line %d
object(C)#2 (0) {
}
acceptIntersection(): Argument #1 ($aAndBOrC) must be of type (A&B)|C, D given, called in %s on line %d
--- main thread end ---
--- child thread start ---
object(AAndB)#2 (0) {
}
acceptIntersection(): Argument #1 ($aAndBOrC) must be of type (A&B)|C, JustA given, called in %s on line %d
acceptIntersection(): Argument #1 ($aAndBOrC) must be of type (A&B)|C, JustB given, called in %s on line %d
object(C)#3 (0) {
}
acceptIntersection(): Argument #1 ($aAndBOrC) must be of type (A&B)|C, D given, called in %s on line %d
--- child thread end ---
OK
