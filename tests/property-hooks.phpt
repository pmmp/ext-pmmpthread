--TEST--
Test that PHP 8.4 property read/write hooks work as expected on copied classes
--SKIPIF--
<?php if(PHP_VERSION_ID < 80400) die("skip PHP 8.4+ only"); ?>
--FILE--
<?php

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

function test(PropertyHooks $object) : void{
	var_dump($object);

	try{
		$object->virtualOnlyGet = 1; //error
		echo "Something is wrong, this is not supposed to be writable\n";
	}catch(\Error $e){
		echo $e->getMessage() . "\n";
	}

	//TODO: test more stuff
}

echo "--- main thread test ---\n";
test(new PropertyHooks());
echo "--- main thread done ---\n";

$t = new class extends \pmmp\thread\Thread{
	public function run() : void{
		echo "--- child thread test ---\n";
		test(new PropertyHooks());
		echo "--- child thread done ---\n";
	}
};
$t->start(\pmmp\thread\Thread::INHERIT_ALL) && $t->join();
echo "done\n";
--EXPECTF--
--- main thread test ---
object(PropertyHooks)#%d (2) {
  ["virtualBackingAsymmetric":"PropertyHooks":private]=>
  int(0)
  ["virtualBacking":"PropertyHooks":private]=>
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
Property PropertyHooks::$virtualOnlyGet is read-only
--- main thread done ---
--- child thread test ---
object(PropertyHooks)#%d (2) {
  ["virtualBackingAsymmetric":"PropertyHooks":private]=>
  int(0)
  ["virtualBacking":"PropertyHooks":private]=>
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
Property PropertyHooks::$virtualOnlyGet is read-only
--- child thread done ---
done
