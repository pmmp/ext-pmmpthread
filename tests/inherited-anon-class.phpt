--TEST--
Test fix for #658 with inheritance
--DESCRIPTION--
Unbound anon class causing segfaults, we delay copy but still cannot serialize the anon
--FILE--
<?php
$task = new class extends Thread {
    public function run() : void{
        $this->prop = new class extends ThreadedBase {};
		var_dump($this->prop);
    }
};
$task->start() && $task->join();
--EXPECTF--
object(%s@anonymous)#2 (0) {
}
