--TEST--
Test that unsupported types are correctly rejected from ThreadSafeArray::merge()
--FILE--
<?php

$t1 = new \pmmp\thread\ThreadSafeArray();

function test(\pmmp\thread\ThreadSafeArray $t1, array|object $input) : void{
	try{
		$t1->merge($input);
		echo "success" . PHP_EOL;
	}catch(\pmmp\thread\NonThreadSafeValueError $e){
		echo $e->getMessage() . PHP_EOL;
	}
}

test($t1, [
	"a" => 1,
	"b" => new \stdClass(),
	"c" => new \stdClass()
]);
test($t1, [
	0 => 0,
	1 => 1,
	2 => new \stdClass(),
	3 => new \stdClass()
]);
test($t1, [
	-1 => new \stdClass()
]);

$object = new \stdClass();
$object->a = 1;
$object->b = new \stdClass();
test($t1, $object);
?>
--EXPECT--
Cannot merge non-thread-safe value of type stdClass (input key "b") into pmmp\thread\ThreadSafeArray
Cannot merge non-thread-safe value of type stdClass (input key 2) into pmmp\thread\ThreadSafeArray
Cannot merge non-thread-safe value of type stdClass (input key -1) into pmmp\thread\ThreadSafeArray
Cannot merge non-thread-safe value of type stdClass (input key "b") into pmmp\thread\ThreadSafeArray
