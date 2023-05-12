--TEST--
Test that thread-safe object throw the correct exceptions when attempting to serialize them
--SKIPIF--
<?php if(PHP_VERSION_ID < 80100) die("skip __serialize() has broken behaviour on 8.0 (https://bugs.php.net/bug.php?id=81111)"); ?>
--FILE--
<?php

$array = new \pmmp\thread\ThreadSafeArray();
try{
	serialize($array);
}catch(\Throwable $e){
	echo $e->getMessage() . PHP_EOL;
}

class Test extends \pmmp\thread\ThreadSafe{
	public function __serialize() : array{
		return ["test" => 1];
	}
}

class TestSleep extends \pmmp\thread\ThreadSafe{
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
Serialization of 'pmmp\thread\ThreadSafeArray' is not allowed
Serialization of 'Test' is not allowed
Serialization of 'TestSleep' is not allowed
