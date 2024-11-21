--TEST--
Test pooling
--DESCRIPTION--
This test verifies the functionality of selective inheritance
--FILE--
<?php
class WebWorker extends \pmmp\thread\Worker {
	public function __construct(SafeLog $logger) {
		$this->logger = $logger;
	}
	
	public $logger;
}

class WebWork extends \pmmp\thread\Runnable {
	public function __construct(int $id) {
		$this->id = $id;
	}

	public function run() : void{
		$worker = \pmmp\thread\Thread::getCurrentThread();
		$worker
			->logger
			->log("%s(%d) executing in Thread #%lu",
				  __CLASS__, $this->id, $worker->getThreadId());
	}

	protected $id;
}

class SafeLog extends \pmmp\thread\ThreadSafe {
	public function log($message, ... $args) {
		$this->synchronized(function($message, ... $args) {
			echo vsprintf("{$message}\n", ...$args);
		}, $message, $args);
	}
}

$pool = new \pmmp\thread\Pool(8, 'WebWorker', array(new SafeLog()));
while (@$i++<10)
	$pool->submit(new WebWork($i));
$pool->shutdown();
var_dump($pool);
?>
--EXPECTF--
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
object(pmmp\thread\Pool)#%d (%d) {
  ["size":protected]=>
  int(8)
  ["class":protected]=>
  string(9) "WebWorker"
  ["workers":protected]=>
  array(0) {
  }
  ["ctor":protected]=>
  array(1) {
    [0]=>
    object(SafeLog)#%d (0) {
    }
  }
  ["last":protected]=>
  int(%d)
}

