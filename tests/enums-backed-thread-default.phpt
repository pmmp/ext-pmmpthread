--TEST--
Test that enum values work correctly when used as defaults for ThreadSafe property defaults (pmmp/ext-pmmpthread#128)
--FILE--
<?php

use pmmp\thread\Thread;

enum ThreadStatus: int{
    case STARTING = 0;
}

class TestThread extends Thread{

    private ThreadStatus $test = ThreadStatus::STARTING;

    public function run(): void{}
}

$t = new TestThread();
$t->start(Thread::INHERIT_NONE);
$t->join();
echo "OK\n";
?>
--EXPECT--
OK
