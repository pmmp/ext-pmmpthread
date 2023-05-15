--TEST--
Test waiting timeouts
--DESCRIPTION--
This test verifies that reaching a timeout returns the correct value

Note that this is a dodgy test, ONLY EVER WAIT FOR SOMETHING
We assume, wrongly, that the thread will be active and waiting within 3
seconds.
--SKIPIF--
<?php if (defined('PHP_WINDOWS_VERSION_MAJOR')) die("skip: no support for this on windows"); ?>
--FILE--
<?php
class T extends \pmmp\thread\Thread {
        public $data;
        public function run() : void{
			$start = time();
            $this->synchronized(function() use($start) {
				while (time() - $start < 3) {
					$this->wait(100000);
				}
			});
        }
}

$t = new T;
$t->start(\pmmp\thread\Thread::INHERIT_ALL);
$t->synchronized(function($thread){
	var_dump($thread->wait(100)); # should return false because no notification sent
								  # but may wake up (and return true) because notification might come from
								  # somewhere else in the system and there is no precondition
								  # we are testing for faults rather than output here ...
}, $t);
?>
--EXPECTF--
bool(%s)
