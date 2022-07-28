--TEST--
Test that backed enum classes are correctly copied when their AST was already resolved before copying
--SKIPIF--
<?php if(PHP_VERSION_ID < 80100) die("skip this test is for 8.1+"); ?>
--FILE--
<?php

enum TestEnum : string{
	case A = "yes";
	case B = "yes2";
}

function test() : void{
	foreach(TestEnum::cases() as $case){
		var_dump($case->value);
		var_dump(TestEnum::from($case->value));
	}
}

test();

$t = new class extends \Thread{
	public function run(){
		test();
	}
};
$t->start() && $t->join();

?>
--EXPECT--
string(3) "yes"
enum(TestEnum::A)
string(4) "yes2"
enum(TestEnum::B)
string(3) "yes"
enum(TestEnum::A)
string(4) "yes2"
enum(TestEnum::B)
