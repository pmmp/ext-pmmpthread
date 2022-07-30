--TEST--
Basic anonymous class support, fix #505
--DESCRIPTION--
Unbound anon class causing segfaults, we delay copy but still cannot serialize the anon
--FILE--
<?php

interface TestInterface{}

class Test extends Thread {
	/**
	 * doccomment run
	 */
	public function run() : void{
		$this->alive = true;
		/**
		 * doccomment anonymous
		 */
		$this->anonymous = new class extends Thread implements TestInterface {
			const CONSTANT = 'constant';
			/**
			 * @var
			 */
			public $pubProp;
			protected $protProp;
			private $privProp;
			public static $staticProp;
			public function run() : void{
				var_dump('anonymous run');
				$this->ready = true;
			}
			public function method() {
				var_dump('method executed');
			}
			public static function staticMethod() {}
		};
		var_dump($this->anonymous);
		$this->anonymous->start();
		$this->anonymous->join();
		$this->synchronized(function() : void{
			$this->notify();
		});
		$this->synchronized(function() : void{
			while($this->alive) {
				$this->wait();
			}
		});
	}
}
$test = new Test();
$test->start();
$test->synchronized(function() use ($test) : void{
	while(!isset($test->anonymous, $test->anonymous->ready)) {
		$test->wait();
	}
});
var_dump($test->anonymous);
$test->anonymous->run();
$test->anonymous->method();

$test->alive = false;
$test->synchronized(function() use ($test) : void{
	$test->notify();
});
$test->join();
--EXPECT--
object(Thread@anonymous)#2 (3) {
  ["pubProp"]=>
  NULL
  ["protProp":protected]=>
  NULL
  ["privProp":"Thread@anonymous":private]=>
  NULL
}
string(13) "anonymous run"
object(Thread@anonymous)#3 (4) {
  ["pubProp"]=>
  NULL
  ["protProp":protected]=>
  NULL
  ["privProp":"Thread@anonymous":private]=>
  NULL
  ["ready"]=>
  bool(true)
}
string(13) "anonymous run"
string(15) "method executed"
