--TEST--
Test that new in attribute declarations works properly on threads
--SKIPIF--
<?php if(PHP_VERSION_ID < 80100) die("skip this test is for 8.1 only"); ?>
--FILE--
<?php

#[Attribute(Attribute::TARGET_CLASS)]
class Attr{
	public function __construct(public stdClass $object){}
}

#[Attr(new stdClass())]
class A{
}

function test() : void{
	var_dump(array_map(fn(ReflectionAttribute $attribute) => $attribute->newInstance(), (new ReflectionClass(A::class))->getAttributes()));
}

$w = new class extends \Thread{
	public function run() : void{
		test();
	}
};
$w->start() && $w->join();

test();

?>
--EXPECT--
array(1) {
  [0]=>
  object(Attr)#3 (1) {
    ["object"]=>
    object(stdClass)#5 (0) {
    }
  }
}
array(1) {
  [0]=>
  object(Attr)#3 (1) {
    ["object"]=>
    object(stdClass)#5 (0) {
    }
  }
}