--TEST--
Test that Closure with use()d variables, use()d by another closure works correctly (pmmp/ext-pmmpthread#130)
--DESCRIPTION--
Weird edge case in #130 when a closure with a used variable is in turn used by another closure.
It's not clear why this breaks yet - closures without any use() work just fine, so it's pretty confusing.
--FILE--
<?php

use pmmp\thread\Runnable;
use pmmp\thread\Worker;
use pmmp\thread\Thread;

class AsyncClosureTask extends Runnable{

	public function __construct(private \Closure $onRun){}

	public function run(): void{
		($this->onRun)();
	}
}

function wrap(\Closure $closure, Worker $worker) : void{
	$wrapper = static function () use ($closure): void{
		var_dump($closure); //UNKNOWN:0
	};
	$worker->stack(new AsyncClosureTask($wrapper));
}

$worker = new Worker();
$worker->start(Thread::INHERIT_ALL);

$closureWithoutUse = static function (): void{
	var_dump("Test");
};
wrap($closureWithoutUse, $worker);

$test = "Test";
$closureWithUse = static function () use ($test): void{
	var_dump($test);
};
wrap($closureWithUse, $worker);
?>
--EXPECT--
object(Closure)#4 (0) {
}
object(Closure)#7 (1) {
  ["static"]=>
  array(1) {
    ["test"]=>
    string(4) "Test"
  }
}
