--TEST--
Test that string caches get properly invalidated when updated from another thread (pmmp/ext-pmmpthread#119)
--FILE--
<?php

use pmmp\thread\Thread;

$thread = new class extends Thread {
    public int $integer = 0;
    public bool $boolean = false;
    public string $string = "Initial";
    public function run() : void {
        $this->integer = 1;
        $this->boolean = true;
        $this->string = "Hello World";
    }
};

$thread->string; //Commenting this out will make the issue go away
$thread->start(Thread::INHERIT_NONE) && $thread->join();
var_dump($thread->integer, $thread->boolean, $thread->string);
?>
--EXPECT--
int(1)
bool(true)
string(11) "Hello World"
