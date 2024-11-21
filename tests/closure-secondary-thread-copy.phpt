--TEST--
Closure copying of an already copied closure
--DESCRIPTION--
We need to retain function flag information for closures, since their copying
semantics differ to that of normal function copying.
--FILE--
<?php

class SubThread extends \pmmp\thread\Thread
{
    public function run() : void{
        $this->testFunction();
    }

    public function testFunction()
    {
        (function () {})();
    }
}

$thread = new class extends \pmmp\thread\Thread {
    public function run() : void{
        $thread = new SubThread();
        $thread->start(\pmmp\thread\Thread::INHERIT_NONE) && $thread->join();
    }
};

$thread->start(\pmmp\thread\Thread::INHERIT_ALL) && $thread->join();
echo "OK\n";
--EXPECT--
OK
