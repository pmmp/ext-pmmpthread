--TEST--
Test iterating on Threaded objects produces the same Threaded objects as property reads
--DESCRIPTION--
Threaded iteration was bypassing local property cache, causing a new Threaded object to be created every time a Threaded member of a Threaded object was yielded by foreach.
--FILE--
<?php
$threaded = new Threaded();

$threaded[] = new \Threaded();
$threaded[] = new \Threaded();

$thread = new class($threaded) extends \Thread{
	private \Threaded $threaded;

	public function __construct(\Threaded $t){
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
object(Threaded)#3 (0) {
}
object(Threaded)#4 (0) {
}
object(Threaded)#3 (0) {
}
object(Threaded)#4 (0) {
}
