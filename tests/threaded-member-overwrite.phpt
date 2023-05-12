--TEST--
Test that ThreadSafe doesn't crash when overwriting a member with a non-ThreadSafe member
--FILE--
<?php

$v = new \pmmp\thread\ThreadSafe();
$v2 = new \pmmp\thread\ThreadSafe();
$v->a = function(){};

$thread = new class($v, $v2) extends \pmmp\thread\Thread{
	private $v;
	private $v2;

	public function __construct(\pmmp\thread\ThreadSafe $v, \pmmp\thread\ThreadSafe $v2){
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
object(pmmp\thread\ThreadSafe)#2 (0) {
}
