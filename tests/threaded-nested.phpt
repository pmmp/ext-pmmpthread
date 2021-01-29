--TEST--
Test nested Threaded objects
--DESCRIPTION--
This test verifies the possibility to nest Threaded objects
--FILE--
<?php
class Node extends Threaded {}

class TestNestedWrite extends Thread {
    private $shared;

    public function __construct(Threaded $shared) {
        $this->shared = $shared;
    }

    public function run() {
        var_dump($this->shared['queue'][0]);

        // link to new var
        $this->shared['queue'][1] = $this->shared['queue'][0];

        // unset old ref
        //unset($this->shared['queue'][0]);

        // or replace ref
        $this->shared['queue'][0] = new Threaded();

        $this->shared['lock'] = true;

        while(!isset($this->shared['lock2'])) {}

        var_dump($this->shared['queue'][1]);

        $this->shared['lock3'] = true;
    }
}

class TestNestedRead extends Thread {
    private $shared;

    public function __construct(Threaded $shared) {
        $this->shared = $shared;
    }

    public function run() {
        while(!isset($this->shared['lock']));

        var_dump($this->shared['queue'][1]);

        $this->shared['queue'][1] = new Node();
        $this->shared['lock2'] = true;

        while(!isset($this->shared['quit']));
    }
}

class Test extends Thread {
    public function run() {
        $queue = new Threaded();
        $queue[0] = new Threaded();

        $shared = new Threaded();
        $shared['queue'] = $queue;

        $thread = new TestNestedWrite($shared);
        $thread->start();

        $thread2 = new TestNestedRead($shared);
        $thread2->start();

        while(!isset($shared['lock3']));

        $shared['quit'] = true;

        $thread2->join();
        $thread->join();
    }
}
$thread = new Test();
$thread->start();
$thread->join();
?>
--EXPECT--
object(Threaded)#4 (0) {
}
object(Threaded)#4 (0) {
}
object(Node)#4 (0) {
}

