--TEST--
Test that Thread::getCurrentThread() returns the proper result inside a shutdown function (pmmp/ext-pmmpthread#68)
--FILE--
<?php

use pmmp\thread\Thread;

class TestThread extends Thread{
	public function run() : void{
		register_shutdown_function(function() : void{
			var_dump(Thread::getCurrentThread());
		});
	}
};

$t = new TestThread;
$t->start(Thread::INHERIT_NONE);
$t->join();
echo "OK\n";
?>
--EXPECT--
object(TestThread)#2 (0) {
}
OK
