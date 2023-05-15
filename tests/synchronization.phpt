--TEST--
Test synchronized blocks
--DESCRIPTION--
This test verifies that syncronization is working
--FILE--
<?php
class T extends \pmmp\thread\Thread {
        public $data;
        public function run() : void{
            $this->synchronized(function($thread){
				$thread->data = true;
				$thread->notify();
			}, $this);               
        }
}
$t = new T;
$t->start(\pmmp\thread\Thread::INHERIT_ALL);
$t->synchronized(function($thread){
	if (!$thread->data) {
		var_dump($thread->wait());
	} else var_dump($thread->data);
}, $t);
?>
--EXPECT--
bool(true)
