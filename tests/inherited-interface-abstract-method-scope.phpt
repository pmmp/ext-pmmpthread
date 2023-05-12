--TEST--
Test that NULL scope on class methods is properly handled
--DESCRIPTION--
At some point, inherited interface abstract methods started to have a NULL scope,
causing pthreads to crash when copying them.
--FILE--
<?php

interface Logger{
        public function emergency($message);
}

abstract class ThreadedLogger implements Logger{}
abstract class AttachableThreadedLogger extends \ThreadedLogger{}

$w = new \pmmp\thread\Worker();
$w->start(PTHREADS_INHERIT_ALL);
$w->shutdown();
echo "ok\n";
?>
--EXPECT--
ok
