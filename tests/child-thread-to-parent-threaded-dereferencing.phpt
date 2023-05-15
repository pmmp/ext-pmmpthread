--TEST--
Test that dereferencing thread-safe objects from joined Threads works correctly
--DESCRIPTION--
Since we don't add to pthreads_object_t refcount when adding it to another pthreads_object_t (we don't want to have to implement a cycle garbage collector),
we rely on being able to dereference thread-safe objects from the child thread or task that created them, just before they are destroyed by the child thread.
This works by having the child thread wait for the main thread to join() with it (or call collect() in the case of workers), and then allowing the child thread
to finish cleaning up its objects and exit.
This behaviour relies on a proper chain of ownership being preserved, but it's good enough for common use cases.
This test verifies that this behaviour works as intended.
--FILE--
<?php

$worker = new class extends \pmmp\thread\Thread{
	public \pmmp\thread\ThreadSafeArray $array;

	public function run() : void{
		$this->array = new \pmmp\thread\ThreadSafeArray();
		$this->array["sub"] = new \pmmp\thread\ThreadSafeArray();
		$this->array["recursive"] = $this->array;
	}
};
$worker->start(\pmmp\thread\Thread::INHERIT_ALL);
$worker->join();
var_dump($worker);
?>
--EXPECT--
object(pmmp\thread\Thread@anonymous)#1 (1) {
  ["array"]=>
  object(pmmp\thread\ThreadSafeArray)#2 (2) {
    ["sub"]=>
    object(pmmp\thread\ThreadSafeArray)#3 (0) {
    }
    ["recursive"]=>
    *RECURSION*
  }
}
