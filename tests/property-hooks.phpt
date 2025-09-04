--TEST--
Test that PHP 8.4 property read/write hooks work as expected on copied classes and ThreadSafe classes
--SKIPIF--
<?php if(PHP_VERSION_ID < 80400) die("skip PHP 8.4+ only"); ?>
--FILE--
<?php

use pmmp\thread\ThreadSafe;

class PropertyHooks{

	private int $virtualBackingAsymmetric = 0;

	public int $virtualOnlyGet {
		get => $this->virtualBackingAsymmetric + 1;
		//can't be set because it's not backed
	}

	public int $virtualOnlySet {
		//do NOT use an arrow function for this - it will make the property non-virtual
		//thanks for the new footgun PHP!!!
		set { $this->virtualBackingAsymmetric = $value - 1; }
	}

	private int $virtualBacking = 0;

	public int $virtualReadWrite {
		get => $this->virtualBacking + 1;
		set => $this->virtualBacking = $value - 1;
	}

	public int $backedOnlyGet {
		get => $this->backedOnlyGet + 1;
		//set will use default property write behaviour
	}

	public int $backedOnlySet {
		set => $value + 1;
	}

	public int $backedGetSet {
		get => $this->backedGetSet + 1;
		set => $value - 1;
	}
}

class PropertyHooksThreadSafe extends ThreadSafe{

	private int $virtualBackingAsymmetric = 0;

	public int $virtualOnlyGet {
		get => $this->virtualBackingAsymmetric + 1;
		//can't be set because it's not backed
	}

	public int $virtualOnlySet {
		//do NOT use an arrow function for this - it will make the property non-virtual
		//thanks for the new footgun PHP!!!
		set { $this->virtualBackingAsymmetric = $value - 1; }
	}

	private int $virtualBacking = 0;

	public int $virtualReadWrite {
		get => $this->virtualBacking + 1;
		set => $this->virtualBacking = $value - 1;
	}

	public int $backedOnlyGet {
		get => $this->backedOnlyGet + 1;
		//set will use default property write behaviour
	}

	public int $backedOnlySet {
		set => $value + 1;
	}

	public int $backedGetSet {
		get => $this->backedGetSet + 1;
		set => $value - 1;
	}
}

function test(PropertyHooks|PropertyHooksThreadSafe $object) : void{
	var_dump($object);

	var_dump($object->virtualOnlyGet);
	var_dump(isset($object->virtualOnlyGet)); //true
	try{
		$object->virtualOnlyGet = 1; //error
		echo "Something is wrong, this is not supposed to be writable\n";
	}catch(\Error $e){
		echo $e->getMessage() . "\n";
	}

	$object->virtualOnlySet = 2;
	try{
		var_dump(isset($object->virtualOnlySet));
		echo "This should have generated an error\n";
	}catch(\Error $e){
		echo $e->getMessage() . "\n";
	}
	try{
		var_dump($object->virtualOnlySet);
		echo "Something is wrong, this is not supposed to be readable\n";
	}catch(\Error $e){
		echo $e->getMessage() . "\n";
	}

	$object->virtualReadWrite = 5;
	var_dump($object->virtualReadWrite); //5
	var_dump(isset($object->virtualReadWrite)); //true

	//backed without setter is a normal property write
	$object->backedOnlyGet = 6;
	var_dump($object->backedOnlyGet); //7
	var_dump(isset($object->backedOnlyGet)); //true

	//backed without getter is a normal property read
	var_dump(isset($object->backedOnlySet)); //false
	$object->backedOnlySet = 7;
	var_dump($object->backedOnlySet); //8
	var_dump(isset($object->backedOnlySet)); //true

	$object->backedGetSet = 8;
	var_dump($object->backedGetSet); //8
	var_dump(isset($object->backedGetSet));
	//TODO: test more stuff

	try{
		unset($object->backedGetSet);
		echo "This shouldn't be allowed\n";
	}catch(\Error $e){
		echo $e->getMessage() . "\n";
	}
}

echo "--- main thread test ---\n";
echo "--- normal object\n";
test(new PropertyHooks());
echo "--- threadsafe object\n";
test(new PropertyHooksThreadSafe());
echo "--- main thread done ---\n";

$t = new class extends \pmmp\thread\Thread{
	public function run() : void{
		echo "--- child thread test ---\n";
		echo "--- normal object\n";
		test(new PropertyHooks());
		echo "--- threadsafe object\n";
		test(new PropertyHooksThreadSafe());
		echo "--- child thread done ---\n";
	}
};
$t->start(\pmmp\thread\Thread::INHERIT_ALL) && $t->join();
echo "done\n";
?>
--EXPECTF--
--- main thread test ---
--- normal object
object(%s)#%d (2) {
  ["virtualBackingAsymmetric":"%s":private]=>
  int(0)
  ["virtualBacking":"%s":private]=>
  int(0)
  ["virtualReadWrite"]=>
  uninitialized(int)
  ["backedOnlyGet"]=>
  uninitialized(int)
  ["backedOnlySet"]=>
  uninitialized(int)
  ["backedGetSet"]=>
  uninitialized(int)
}
int(1)
bool(true)
Property %s::$virtualOnlyGet is read-only
Property %s::$virtualOnlySet is write-only
Property %s::$virtualOnlySet is write-only
int(5)
bool(true)
int(7)
bool(true)
bool(false)
int(8)
bool(true)
int(8)
bool(true)
Cannot unset hooked property %s::$backedGetSet
--- threadsafe object
object(%s)#%d (2) {
  ["virtualBackingAsymmetric":"%s":private]=>
  int(0)
  ["virtualBacking":"%s":private]=>
  int(0)
  ["virtualReadWrite"]=>
  uninitialized(int)
  ["backedOnlyGet"]=>
  uninitialized(int)
  ["backedOnlySet"]=>
  uninitialized(int)
  ["backedGetSet"]=>
  uninitialized(int)
}
int(1)
bool(true)
Property %s::$virtualOnlyGet is read-only
Property %s::$virtualOnlySet is write-only
Property %s::$virtualOnlySet is write-only
int(5)
bool(true)
int(7)
bool(true)
bool(false)
int(8)
bool(true)
int(8)
bool(true)
Cannot unset hooked property %s::$backedGetSet
--- main thread done ---
--- child thread test ---
--- normal object
object(%s)#%d (2) {
  ["virtualBackingAsymmetric":"%s":private]=>
  int(0)
  ["virtualBacking":"%s":private]=>
  int(0)
  ["virtualReadWrite"]=>
  uninitialized(int)
  ["backedOnlyGet"]=>
  uninitialized(int)
  ["backedOnlySet"]=>
  uninitialized(int)
  ["backedGetSet"]=>
  uninitialized(int)
}
int(1)
bool(true)
Property %s::$virtualOnlyGet is read-only
Property %s::$virtualOnlySet is write-only
Property %s::$virtualOnlySet is write-only
int(5)
bool(true)
int(7)
bool(true)
bool(false)
int(8)
bool(true)
int(8)
bool(true)
Cannot unset hooked property %s::$backedGetSet
--- threadsafe object
object(%s)#%d (2) {
  ["virtualBackingAsymmetric":"%s":private]=>
  int(0)
  ["virtualBacking":"%s":private]=>
  int(0)
  ["virtualReadWrite"]=>
  uninitialized(int)
  ["backedOnlyGet"]=>
  uninitialized(int)
  ["backedOnlySet"]=>
  uninitialized(int)
  ["backedGetSet"]=>
  uninitialized(int)
}
int(1)
bool(true)
Property %s::$virtualOnlyGet is read-only
Property %s::$virtualOnlySet is write-only
Property %s::$virtualOnlySet is write-only
int(5)
bool(true)
int(7)
bool(true)
bool(false)
int(8)
bool(true)
int(8)
bool(true)
Cannot unset hooked property %s::$backedGetSet
--- child thread done ---
done
