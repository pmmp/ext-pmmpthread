--TEST--
Test object comparison
--DESCRIPTION--
Threaded object comparison was broken in pthreads v2, the handler was not implemented.

In pthreads v3 we implement the handler, but not in the way Zend is expecting us too.

To traverse (and reconstruct in most cases) the property store in order to perform a comparison is extremely inefficient.

In addition, because of the way pthreads works, it might be useful to determine if two references from two different contexts are infact references
to the same object from some other context.

So pthreads v3 implements the comparison handler and returns true when two references are for the same ThreadSafe object, this differs from normal object
comparison which would return true if the property tables of two distinct objects are uniform.
--FILE--
<?php
class Test extends \pmmp\thread\Thread {
	public function __construct(\pmmp\thread\ThreadSafeArray $arg1, \pmmp\thread\ThreadSafeArray $arg2) {
		$this->arg1 = $arg1;
		$this->arg2 = $arg2;
	}

	public function run() : void{
		var_dump($this->arg1 == $this->arg2);
	}
}

$a = new \pmmp\thread\ThreadSafeArray();
$b = new \pmmp\thread\ThreadSafeArray();

$test = new Test($a, $b);	# bool(false)
$test->start(\pmmp\thread\Thread::INHERIT_ALL);
$test->join();

$test = new Test($a, $a);	# bool(true)
$test->start(\pmmp\thread\Thread::INHERIT_ALL);
$test->join();
?>
--EXPECT--
bool(false)
bool(true)

