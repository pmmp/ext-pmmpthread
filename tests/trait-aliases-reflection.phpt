--TEST--
Test trait aliases reflection
--DESCRIPTION--
Trait aliases not properly copied
--FILE--
<?php
trait Hello {
    public function world():void { }
}

class Foo {
    use Hello { world as sun; }
}
$t = new class extends \pmmp\thread\Thread {
    public function run() : void{
        $foo = new Foo();

        $class = new ReflectionClass($foo);
        var_dump($class->getTraitAliases());
    }
};
$t->start(\pmmp\thread\Thread::INHERIT_ALL) && $t->join();
--EXPECT--
array(1) {
  ["sun"]=>
  string(12) "Hello::world"
}

