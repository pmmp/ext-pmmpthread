--TEST--
Test that enum classes are correctly copied
--SKIPIF--
<?php if(PHP_VERSION_ID < 80100) die("skip this test is for 8.1+"); ?>
--FILE--
<?php

enum TestEnum{
	case A;
	case B;
}

function test() : void{
	foreach(TestEnum::cases() as $case){
		var_dump($case);
	}
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
enum(TestEnum::A)
enum(TestEnum::B)
enum(TestEnum::A)
enum(TestEnum::B)
