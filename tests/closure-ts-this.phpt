--TEST--
Test that closures with thread-safe $this work correctly
--DESCRIPTION--
We weren't copying closure $this, causing them to suddenly become NULL on the destination thread.
Since the closure's $this is thread-safe, it should be restored onto the destination thread without issues.
--FILE--
<?php

class A extends \pmmp\thread\ThreadSafe{
	public function getClosure() : \Closure{
		return function() : void{
			var_dump($this);
		};
	}
}

$a = new A();
$closure = $a->getClosure();

$thread = new class($closure) extends \pmmp\thread\Thread{
	public function __construct(private \Closure $closure){}

	public function run() : void{
		var_dump($this->closure);
	}
};

$thread->start(\pmmp\thread\Thread::INHERIT_ALL);
$thread->join();

?>
--EXPECT--
object(Closure)#3 (1) {
  ["this"]=>
  object(A)#2 (0) {
  }
}
