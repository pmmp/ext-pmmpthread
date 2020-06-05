--TEST--
Test that Threaded object properties and sync still works when their creator is destroyed
--DESCRIPTION--
Previously, a Threaded object's monitor and property store would be directly destroyed when the original object went out of scope, which would lead to segfaults if other threads tried to use their refs afterwards.

Now, we refcount the internal structures for Threaded objects, so they should continue to work after their creator has gone out of scope.

This test verifies that locking, property write and more continue to work when the thread that created it destroys its original reference.
--FILE--
<?php

$t = new class extends \Thread{
	/** @var string|null */
	public $chan = null;

	/** @var bool */
	public $shutdown = false;

	public function run() : void{
		$chan = $this->synchronized(function() : \Threaded{
			$chan = new \Threaded;
			$chan[] = 1;
			$this->chan = serialize($chan);
			$this->notify();
			return $chan;
		});
		$this->synchronized(function() use(&$chan) : void{
			while(!$this->shutdown){
				$this->wait();
			}
			$chan = null; //destroy from creator context
		});
	}
};

$t->start();
$chan = $t->synchronized(function() use($t) : \Threaded{
	while($t->chan === null){
		$t->wait();
	}
	return unserialize($t->chan);
});
$t->synchronized(function() use($t) : void{
	$t->shutdown = true;
	$t->notify();
});
$t->join();
var_dump($chan);
var_dump($chan->shift());
var_dump($chan);
var_dump($chan->property = "test");
var_dump($chan);
var_dump($chan->count());
$chan->synchronized(function() : void{
	echo "sync works!\n";
});
echo "ok\n";
?>
--EXPECTF--
object(Threaded)#%d (1) {
  [0]=>
  int(1)
}
int(1)
object(Threaded)#%d (0) {
}
string(4) "test"
object(Threaded)#%d (1) {
  ["property"]=>
  string(4) "test"
}
int(1)
sync works!
ok
