--TEST--
Test that Volatile doesn't crash when overwriting a member with a non-Threaded member
--FILE--
<?php

$v = new \Volatile();
$v2 = new \Volatile();
$v->a = function(){};

$thread = new class($v, $v2) extends \Thread{
	private $v;
	private $v2;

	public function __construct(\Volatile $v, \Volatile $v2){
		$this->v = $v;
		$this->v2 = $v2;
	}

	public function run(){
		$this->v->a = $this->v2;
	}
};
$thread->start() && $thread->join();
var_dump($v->a);

?>
--EXPECT--
object(Volatile)#2 (0) {
}
