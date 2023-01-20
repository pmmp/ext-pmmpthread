--TEST--
Test iterating on ThreadedArray objects produces the same ThreadedArray objects as property reads
--DESCRIPTION--
ThreadedArray iteration was bypassing local property cache, causing a new ThreadedArray object to be created every time a ThreadedArray member of a ThreadedArray object was yielded by foreach.
--FILE--
<?php
$threaded = new ThreadedArray();

$threaded[] = new \ThreadedArray();
$threaded[] = new \ThreadedArray();

$thread = new class($threaded) extends \Thread{
	private \ThreadedArray $threaded;

	public function __construct(\ThreadedArray $t){
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
object(ThreadedArray)#3 (0) {
}
object(ThreadedArray)#4 (0) {
}
object(ThreadedArray)#3 (0) {
}
object(ThreadedArray)#4 (0) {
}
