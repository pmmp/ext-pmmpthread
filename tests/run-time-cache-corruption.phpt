--TEST--
Test run time cache bug #494
--DESCRIPTION--
Runtime Caches were being incorrectly used (shared!), this caused strange behaviour for various things
--FILE--
<?php
interface TestInterface {}

class DebugClass implements TestInterface {}

class Some extends \pmmp\thread\ThreadSafe {
    public static function staticNess() {
        $closure = function() : DebugClass {
            return (new \DebugClass());
        };

        $objCreated = $closure();
        var_dump((new \ReflectionClass($objCreated))->getInterfaceNames());
		var_dump($objCreated instanceof \TestInterface);
    }
}

class Test extends \pmmp\thread\Thread {
    public function run() : void{
        Some::staticNess();
    }
}

Some::staticNess();

$test = new class extends \pmmp\thread\Thread {
	public function run() : void{
		Some::staticNess();
	}
};

$test->start(\pmmp\thread\Thread::INHERIT_NONE | 
			 \pmmp\thread\Thread::INHERIT_INI | 
			 \pmmp\thread\Thread::ALLOW_HEADERS | 
			 \pmmp\thread\Thread::INHERIT_COMMENTS | 
			 \pmmp\thread\Thread::INHERIT_INCLUDES | 
			 \pmmp\thread\Thread::INHERIT_FUNCTIONS | 
			 \pmmp\thread\Thread::INHERIT_CLASSES) && $test->join();
?>
--EXPECT--
array(1) {
  [0]=>
  string(13) "TestInterface"
}
bool(true)
array(1) {
  [0]=>
  string(13) "TestInterface"
}
bool(true)



