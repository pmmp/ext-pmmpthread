--TEST--
Test wait/notify
--DESCRIPTION--
This test will verify wait/notify functionality
--FILE--
<?php
class ThreadTest extends \pmmp\thread\Thread {
	public $sent;
	
	public function __construct() {
	    $this->sent = false;
	}
	
	public function run() : void{
	    $this->synchronized(function($self){
		    $self->sent = true;
		    $self->notify();
	    }, $this);
	}
}
$thread = new ThreadTest();
if($thread->start()) {
	$thread->synchronized(function($me){
	    if (!$me->sent) {
		    var_dump($me->wait());
	    } else var_dump($me->sent);
	}, $thread);
	
} else printf("bool(false)\n");
?>
--EXPECT--
bool(true)
