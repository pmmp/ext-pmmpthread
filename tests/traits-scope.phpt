--TEST--
Test traits scope (gh issue #484)
--FILE--
<?php
class MyWork extends \pmmp\thread\Runnable {
	use \MyTrait;

	public function run() : void{
    	$this->getSomething();
	}
}

trait MyTrait {
	protected function getSomething($test = false) {
		if (!$test) {
			require_once 'traits-scope.inc';
			return (new \MyClass())->doSomething();
		}
	}
}

$pool = new \pmmp\thread\Pool(1, \pmmp\thread\Worker::class);
$pool->submit(new MyWork());
$pool->shutdown();
--EXPECT--
OK

