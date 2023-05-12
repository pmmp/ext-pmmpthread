--TEST--
Test constants copying in worker
--DESCRIPTION--
Constants copying in worker results in endless loop and crashes
--FILE--
<?php

$worker = new \pmmp\thread\Worker();
$worker->start(PTHREADS_INHERIT_NONE);
$worker->stack(new Testing());
$worker->shutdown();

class Testing extends \pmmp\thread\Runnable
{
    const MY_VAR = "Testing";

    public function run() : void{
        var_dump("Running...");
    }
}
?>
--EXPECTF--
string(10) "Running..."
