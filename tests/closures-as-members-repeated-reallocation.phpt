--TEST--
Testing closure members reallocation
--DESCRIPTION--
This test verifies that closures can be set as members repeatedly
--FILE--
<?php
class TestClosure extends \pmmp\thread\Runnable {
    protected $closure;
    function __construct( $closure) {
        $this->closure = $closure;
    }
    public function run() : void{
        ($this->closure)();
    }
}
$count = 0;
$worker = new \pmmp\thread\Worker();
$worker->start(\pmmp\thread\Thread::INHERIT_ALL);
while ($count++ < 1000) {
    $function = new TestClosure(function() {});
    $worker->stack($function);
    while($worker->collect()){
        usleep(1);
    }
}
var_dump('ok');
--EXPECTF--
string(2) "ok"
