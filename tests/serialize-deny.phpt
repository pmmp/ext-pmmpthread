--TEST--
Test that thread-safe object throw the correct exceptions when attempting to serialize them
--FILE--
<?php

$array = new \ThreadedArray();
try{
	serialize($array);
}catch(\Throwable $e){
	echo $e->getMessage() . PHP_EOL;
}

class Test extends \ThreadedBase{
	public function __serialize() : array{
		return ["test" => 1];
	}
}

class TestSleep extends \ThreadedBase{
	public $test = 1;
	public function __sleep() : array{
		return ["test"];
	}
}

try{
	serialize(new Test());
}catch(\Throwable $e){
	echo $e->getMessage() . PHP_EOL;
}

try{
	serialize(new TestSleep());
}catch(\Throwable $e){
	echo $e->getMessage() . PHP_EOL;
}

?>
--EXPECT--
Serialization of 'ThreadedArray' is not allowed
Serialization of 'Test' is not allowed
Serialization of 'TestSleep' is not allowed
