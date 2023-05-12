--TEST--
Test that ThreadSafe object properties and sync still works when their creator is destroyed
--DESCRIPTION--
Previously, a ThreadSafe object's monitor and property store would be directly destroyed when the original object went out of scope, which would lead to segfaults if other threads tried to use their refs afterwards.

Now, we refcount the internal structures for ThreadSafe objects, so they should continue to work after their creator has gone out of scope.

This test verifies that locking, property write and more continue to work when the thread that created it destroys its original reference.
--FILE--
<?php

$t = new class extends \pmmp\thread\Thread{
	/** @var string|null */
	public $chan = null;

	/** @var bool */
	public $shutdown = false;

	public function run() : void{
		$this->synchronized(function() : void{
			$chan = new \pmmp\thread\ThreadSafeArray;
			$chan[] = 1;
			$this->chan = $chan;
			$this->notify();
		});
		$this->synchronized(function() : void{
			while(!$this->shutdown){
				$this->wait();
			}
			$this->chan = null; //destroy from creator context
		});
	}
};

$t->start();
$chan = $t->synchronized(function() use($t) : \pmmp\thread\ThreadSafeArray{
	while($t->chan === null){
		$t->wait();
	}
	return $t->chan;
});
$t->synchronized(function() use($t) : void{
	$t->shutdown = true;
	$t->notify();
});
$t->join();
var_dump($chan);
var_dump($chan->shift());
var_dump($chan);
var_dump($chan["property"] = "test");
var_dump($chan);
var_dump($chan->count());
$chan->synchronized(function() : void{
	echo "sync works!\n";
});
echo "ok\n";
?>
--EXPECTF--
object(pmmp\thread\ThreadSafeArray)#%d (1) {
  [0]=>
  int(1)
}
int(1)
object(pmmp\thread\ThreadSafeArray)#%d (0) {
}
string(4) "test"
object(pmmp\thread\ThreadSafeArray)#%d (1) {
  ["property"]=>
  string(4) "test"
}
int(1)
sync works!
ok
