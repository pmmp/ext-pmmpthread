--TEST--
Test that uninitialized and unset static properties behave correctly
--FILE--
<?php

function test(object $t1) : void{
	var_dump($t1);
	unset($t1->a);
	var_dump($t1);

	try{
		var_dump($t1->a);
	}catch(\Error $e){
		echo $e->getMessage() . PHP_EOL;
	}

	foreach($t1 as $name => $value){
		var_dump($name, $value);
	}
}

echo "--- Normal object ---\n";
test(new class{
	public int $a = 1;
	public string $b = "hello";
});
echo "--- pthreads object ---\n";
echo "--- properties are currently expected to be in an unstable order ---\n";
test(new class extends \ThreadedBase{
	public int $a = 1;
	public string $b = "hello";
});
echo "--- Done ---\n";
?>
--EXPECT--
--- Normal object ---
object(class@anonymous)#1 (2) {
  ["a"]=>
  int(1)
  ["b"]=>
  string(5) "hello"
}
object(class@anonymous)#1 (1) {
  ["a"]=>
  uninitialized(int)
  ["b"]=>
  string(5) "hello"
}
Typed property class@anonymous::$a must not be accessed before initialization
string(1) "b"
string(5) "hello"
--- pthreads object ---
--- properties are currently expected to be in an unstable order ---
object(ThreadedBase@anonymous)#2 (2) {
  ["a"]=>
  int(1)
  ["b"]=>
  string(5) "hello"
}
object(ThreadedBase@anonymous)#2 (2) {
  ["b"]=>
  string(5) "hello"
  ["a"]=>
  uninitialized(int)
}
Typed property ThreadedBase@anonymous::$a must not be accessed before initialization
string(1) "b"
string(5) "hello"
--- Done ---
