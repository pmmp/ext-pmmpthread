--TEST--
Test iterating on ThreadSafeArray objects produces the same ThreadSafeArray objects as property reads
--DESCRIPTION--
ThreadSafeArray iteration was bypassing local property cache, causing a new ThreadSafeArray object to be created every time a ThreadSafeArray member of a ThreadSafeArray object was yielded by foreach.
--FILE--
<?php
$threaded = new \pmmp\thread\ThreadSafeArray();

$threaded[] = new \pmmp\thread\ThreadSafeArray();
$threaded[] = new \pmmp\thread\ThreadSafeArray();

$thread = new class($threaded) extends \pmmp\thread\Thread{
	private \pmmp\thread\ThreadSafeArray $threaded;

	public function __construct(\pmmp\thread\ThreadSafeArray $t){
		$this->threaded = $t;
	}

	public function run() : void{
		var_dump($this->threaded[0]);
		var_dump($this->threaded[1]);

		foreach($this->threaded as $prop){
			//the object IDs should be the same as the ones above
			var_dump($prop);
		}
	}
};
$thread->start();
$thread->join();

?>
--EXPECTF--
object(pmmp\thread\ThreadSafeArray)#3 (0) {
}
object(pmmp\thread\ThreadSafeArray)#4 (0) {
}
object(pmmp\thread\ThreadSafeArray)#3 (0) {
}
object(pmmp\thread\ThreadSafeArray)#4 (0) {
}
