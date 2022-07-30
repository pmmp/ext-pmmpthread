--TEST--
Test interceptors of anonymous classes
--DESCRIPTION--
This test verifies that interceptors of anonymous Thread objects work as expected
--FILE--
<?php
class Foo extends ThreadedBase {
    public $bar;

    public function __construct() {
        var_dump(__METHOD__);

        $this->bar = new class extends ThreadedBase {
            private $buzz;

            public function __construct() {
                var_dump('Bar::__construct');
            }
            public function __destruct() {
                var_dump('Bar::__destruct');
            }
            public function __set($name, $value) {
                var_dump($name, $value);
                $this->buzz = $value;
            }
            public function __get($name) {
                return $this->buzz;
            }
        };
        $this->bar->buzz = 'hello world';
        var_dump($this->bar->buzz);
    }
    public function __destruct() {
        var_dump(__METHOD__);
    }
}

class Test extends \Thread {
    public function run() : void{
        $foo = new Foo();
        var_dump($foo->bar);
    }
}
$thread = new Test();
$thread->start() && $thread->join();
--EXPECT--
string(16) "Foo::__construct"
string(16) "Bar::__construct"
string(4) "buzz"
string(11) "hello world"
string(11) "hello world"
object(ThreadedBase@anonymous)#3 (1) {
  ["buzz":"ThreadedBase@anonymous":private]=>
  string(11) "hello world"
}
string(15) "Foo::__destruct"
string(15) "Bar::__destruct"
