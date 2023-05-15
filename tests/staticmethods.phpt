--TEST--
Test access to static methods from within user threads
--DESCRIPTION--
Static methods as declared in the users implementation of Thread should now be available for calling in the thread scope
--FILE--
<?php
class ThreadTest extends \pmmp\thread\Thread {
	public static function staticTest(){
		return 1;
	}
	
	public function run() : void{
		$this->result = self::staticTest();
	}
}
$thread = new ThreadTest();
if($thread->start(\pmmp\thread\Thread::INHERIT_ALL))
	if ($thread->join())
		var_dump($thread->result);
?>
--EXPECT--
int(1)
