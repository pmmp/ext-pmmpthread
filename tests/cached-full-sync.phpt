--TEST--
Test that full sync restores properties of already cached object
--DESCRIPTION--
pthreads_store_full_sync_local_properties() wasn't recursively syncing properties of objects which were already cached by the owning thread.
This means that attempts to connect with dead objects would sometimes be made despite full sync taking place.
This test verifies that the bug is fixed.
--FILE--
<?php

$array = new \ThreadedArray();

$t1 = new class($array) extends \Thread{
	public function __construct(
		private \ThreadedArray $array
	){}

	public function run() : void{
		$this->array[] = new \ThreadedArray();
	}
};
$t1->start() && $t1->join();

var_dump($array);
?>
--EXPECT--
object(ThreadedArray)#1 (1) {
  [0]=>
  object(ThreadedArray)#3 (0) {
  }
}
