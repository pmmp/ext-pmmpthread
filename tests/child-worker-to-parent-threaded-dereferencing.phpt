--TEST--
Test that dereferencing thread-safe objects from finished Worker tasks works correctly
--DESCRIPTION--
Since we don't add to pthreads_object_t refcount when adding it to another pthreads_object_t (we don't want to have to implement a cycle garbage collector),
we rely on being able to dereference thread-safe objects from the child thread or task that created them, just before they are destroyed by the child thread.
This works by having the child thread wait for the main thread to join() with it (or call collect() in the case of workers), and then allowing the child thread
to finish cleaning up its objects and exit.
This behaviour relies on a proper chain of ownership being preserved, but it's good enough for common use cases.
This test verifies that this behaviour works as intended.
--FILE--
<?php

$worker = new \Worker();
$worker->start();

$worker->stack(new class extends \ThreadedRunnable{
	public \ThreadedArray $array;

	public function run() : void{
		$this->array = new \ThreadedArray();
		$this->array["sub"] = new \ThreadedArray();
		$this->array["recursive"] = $this->array;
	}
});
$done = false;
while(!$done){
	$worker->collect(function(\ThreadedRunnable $work) use (&$done) : void{
		var_dump($work);
		$done = true;
	});
}
?>
--EXPECT--
object(ThreadedRunnable@anonymous)#2 (2) {
  ["array"]=>
  object(ThreadedArray)#4 (2) {
    ["sub"]=>
    object(ThreadedArray)#5 (0) {
    }
    ["recursive"]=>
    *RECURSION*
  }
  ["worker"]=>
  object(Worker)#1 (0) {
  }
}
