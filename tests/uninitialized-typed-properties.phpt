--TEST--
Test that uninitialized and unset static properties behave correctly
--FILE--
<?php
class NTS{
	public int $a = 1;
	public string $b = "hello";
	public $c;
}
class TS extends \ThreadedBase{
	public int $a = 1;
	public string $b = "hello";
	public $c;
}
function test(object $t1) : void{
	var_dump($t1);
	unset($t1->a);
	var_dump($t1);
	var_dump(isset($t1->a));
	var_dump(property_exists($t1, "a"));

	try{
		var_dump($t1->a);
	}catch(\Error $e){
		echo $e->getMessage() . PHP_EOL;
	}

	foreach($t1 as $name => $value){
		var_dump($name, $value);
	}

	var_dump($t1->c);
	var_dump(isset($t1->c));
	var_dump(property_exists($t1, "c"));
}

echo "--- Normal object ---\n";
test(new NTS);
echo "--- pthreads object ---\n";
echo "--- properties are currently expected to be in an unstable order ---\n";
test(new TS);
echo "--- Done ---\n";
?>
--EXPECT--
--- Normal object ---
object(NTS)#1 (3) {
  ["a"]=>
  int(1)
  ["b"]=>
  string(5) "hello"
  ["c"]=>
  NULL
}
object(NTS)#1 (2) {
  ["a"]=>
  uninitialized(int)
  ["b"]=>
  string(5) "hello"
  ["c"]=>
  NULL
}
bool(false)
bool(true)
Typed property NTS::$a must not be accessed before initialization
string(1) "b"
string(5) "hello"
string(1) "c"
NULL
NULL
bool(false)
bool(true)
--- pthreads object ---
--- properties are currently expected to be in an unstable order ---
object(TS)#2 (3) {
  ["a"]=>
  int(1)
  ["b"]=>
  string(5) "hello"
  ["c"]=>
  NULL
}
object(TS)#2 (2) {
  ["b"]=>
  string(5) "hello"
  ["c"]=>
  NULL
  ["a"]=>
  uninitialized(int)
}
bool(false)
bool(true)
Typed property TS::$a must not be accessed before initialization
string(1) "b"
string(5) "hello"
string(1) "c"
NULL
NULL
bool(false)
bool(true)
--- Done ---
