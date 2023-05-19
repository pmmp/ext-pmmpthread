--TEST--
Test shutdown handlers #204
--DESCRIPTION--
Shutdown handlers that were closures were causing segfaults
--FILE--
<?php
class Test extends \pmmp\thread\Thread {
        public function run() : void{
                register_shutdown_function(function(){
                        var_dump(new stdClass());
                });
        }
}

$test = new Test();
$test->start(\pmmp\thread\Thread::INHERIT_ALL);
?>
--EXPECTF--
object(stdClass)#%d (0) {
}

