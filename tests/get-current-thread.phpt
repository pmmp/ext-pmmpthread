--TEST--
Test that Thread::getCurrentThread() works correctly
--FILE--
<?php

var_dump(Thread::getCurrentThread());

$c = new class extends \Thread{
	public function run() : void{
		var_dump($this);
		var_dump(Thread::getCurrentThread());
	}
};
$c->start();
$c->join();

var_dump(Thread::getCurrentThread());
?>
--EXPECT--
NULL
object(Thread@anonymous)#1 (0) {
}
object(Thread@anonymous)#1 (0) {
}
NULL
