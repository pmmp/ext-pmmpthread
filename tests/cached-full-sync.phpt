--TEST--
Test that full sync restores properties of already cached object
--DESCRIPTION--
pthreads_store_full_sync_local_properties() wasn't recursively syncing properties of objects which were already cached by the owning thread.
This means that attempts to connect with dead objects would sometimes be made despite full sync taking place.
This test verifies that the bug is fixed.
--FILE--
<?php

$array = new \pmmp\thread\ThreadSafeArray();

$t1 = new class($array) extends \pmmp\thread\Thread{
	public function __construct(
		private \pmmp\thread\ThreadSafeArray $array
	){}

	public function run() : void{
		$this->array[] = new \pmmp\thread\ThreadSafeArray();
	}
};
$t1->start(\pmmp\thread\Thread::INHERIT_ALL) && $t1->join();

var_dump($array);
?>
--EXPECT--
object(pmmp\thread\ThreadSafeArray)#2 (1) {
  [0]=>
  object(pmmp\thread\ThreadSafeArray)#4 (0) {
  }
}
