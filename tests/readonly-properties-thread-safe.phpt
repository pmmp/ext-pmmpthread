--TEST--
Test that readonly properties behave as expected on thread-safe class descendents
--DESCRIPTION--
pthreads doesn't (yet) mirror the exact behaviour of readonly properties on standard objects
for now, this test just ensures that the behaviour plays out as expected with lockless reads,
since those use local cache without locks if possible.
--SKIPIF--
<?php if(PHP_VERSION_ID < 80100) die("skip readonly properties are only supported in 8.1 and up"); ?>
--FILE--
<?php

class T extends \ThreadedBase{
	public readonly int $a;

	public readonly \ThreadedArray $array;

	public readonly int $initiallyUninit;

	public function __construct(){
		$this->a = 1;
		$this->array = new \ThreadedArray();
	}

	public function unsetProperties() : void{
		try{
			unset($this->a);
		}catch(\Error $e){
			echo $e->getMessage() . PHP_EOL;
		}
		try{
			unset($this->array);
		}catch(\Error $e){
			echo $e->getMessage() . PHP_EOL;
		}
		try{
			unset($this->initiallyUninit);
		}catch(\Error $e){
			echo $e->getMessage() . PHP_EOL;
		}
	}

	public function writeProperties() : void{
		try{
			$this->a = 2;
		}catch(\Error $e){
			echo $e->getMessage() . PHP_EOL;
		}
		try{
			$this->a++;
		}catch(\Error $e){
			echo $e->getMessage() . PHP_EOL;
		}
		try{
			$this->array = new \ThreadedArray();
		}catch(\Error $e){
			echo $e->getMessage() . PHP_EOL;
		}
		try{
			$this->initiallyUninit = 2;
		}catch(\Error $e){
			echo $e->getMessage() . PHP_EOL;
		}
	}

	public function readProperties() : void{
		var_dump($this->a);
		var_dump($this->array);
		var_dump($this->initiallyUninit);
	}

	public function issetProperties() : void{
		var_dump(isset($this->a));
		var_dump(isset($this->array));
		var_dump(isset($this->initiallyUninit));
	}
}

function test(T $t) : void{
	//these must run first, to ensure they work on an unpopulated local cache
	$t->unsetProperties();
	$t->writeProperties();
	$t->issetProperties();
	$t->readProperties();
}

echo "--- main thread start ---\n";
test(new T());
echo "--- main thread end ---\n";

//init properties from the main thread - ensure that the child thread receives an empty local cache
$thread = new class(new T) extends \Thread{
	public function __construct(
		private T $t
	){}

	public function run() : void{
		echo "--- child thread start ---\n";
		test($this->t);
		echo "--- child thread end ---\n";
	}
};
$thread->start();
$thread->join();
echo "OK\n";
?>
--EXPECTF--
--- main thread start ---
Cannot unset readonly property T::$a
Cannot unset readonly property T::$array
Cannot unset readonly property T::$initiallyUninit
Cannot modify readonly property T::$a
Cannot modify readonly property T::$a
Cannot modify readonly property T::$array
bool(true)
bool(true)
bool(true)
int(1)
object(ThreadedArray)#%d (0) {
}
int(2)
--- main thread end ---
--- child thread start ---
Cannot unset readonly property T::$a
Cannot unset readonly property T::$array
Cannot unset readonly property T::$initiallyUninit
Cannot modify readonly property T::$a
Cannot modify readonly property T::$a
Cannot modify readonly property T::$array
bool(true)
bool(true)
bool(true)
int(1)
object(ThreadedArray)#%d (0) {
}
int(2)
--- child thread end ---
OK
