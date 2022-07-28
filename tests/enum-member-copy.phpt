--TEST--
Test that enum members are restored correctly
--SKIPIF--
<?php if(PHP_VERSION_ID < 80100) die("skip this test is for 8.1+ only"); ?>
--FILE--
<?php

enum TestEnum{
	case A;
	case B;
}

$t = new class extends Thread{
	private TestEnum $enum;

	public function __construct(){
		$this->enum = TestEnum::A;
	}

	public function run() : void{
		var_dump($this->enum);
	}
};

$t->start() && $t->join();

?>
--EXPECT--
enum(TestEnum::A)
