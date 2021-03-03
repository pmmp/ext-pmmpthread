--TEST--
Test fix for #659
--DESCRIPTION--
Unbound anon class causing segfaults, we delay copy but still cannot serialize the anon
--FILE--
<?php
$task = new class extends Thread {
    public function run()
    {
        try {
            $this->prop = new class {};
        } catch (\Throwable $e) {
            var_dump($e->getMessage());
        }
    }
};
$task->start() && $task->join();
--EXPECTF--
string(%d) "Serialization of '%s' is not allowed"
