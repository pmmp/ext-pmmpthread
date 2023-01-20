--TEST--
Test that Threaded doesn't crash when overwriting a member with a non-Threaded member
--FILE--
<?php

$v = new \ThreadedBase();
$v2 = new \ThreadedBase();
$v->a = function(){};

$thread = new class($v, $v2) extends \Thread{
	private $v;
	private $v2;

	public function __construct(\ThreadedBase $v, \ThreadedBase $v2){
		$this->v = $v;
		$this->v2 = $v2;
	}

	public function run() : void{
		$this->v->a = $this->v2;
	}
};
$thread->start() && $thread->join();
var_dump($v->a);

?>
--EXPECT--
object(ThreadedBase)#2 (0) {
}
