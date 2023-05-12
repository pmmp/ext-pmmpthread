--TEST--
Test that Thread::getCurrentThread() works correctly
--FILE--
<?php

var_dump(\pmmp\thread\Thread::getCurrentThread());

$c = new class extends \pmmp\thread\Thread{
	public function run() : void{
		var_dump($this);
		var_dump(\pmmp\thread\Thread::getCurrentThread());
	}
};
$c->start();
$c->join();

var_dump(\pmmp\thread\Thread::getCurrentThread());
?>
--EXPECT--
NULL
object(pmmp\thread\Thread@anonymous)#1 (0) {
}
object(pmmp\thread\Thread@anonymous)#1 (0) {
}
NULL
