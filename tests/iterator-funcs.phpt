--TEST--
Testing iterator functions
--DESCRIPTION--
Iterator functions were not properly copied, causing unexpected results and unsafe execution.
--FILE--
<?php
class MyIterator implements \Iterator { 
        protected $items;
        protected $position;

        public function __construct() {
                $this->items = array("joe", "bob", "fred");
                $this->position = 0;
        }

        public function next() : void{
                ++$this->position;
        }

        public function key() : int{
                return $this->position;
        }

        public function current() : string{
                return $this->items[$this->position];
        }

        public function valid() : bool{
                return ($this->position < count($this->items));
        }

        public function rewind() : void{
                $this->myProtectedMethod();
                $this->position = 0;
        }

        protected function myProtectedMethod() {}
}

class MyThread extends \Thread {
        public function run() {
                $it = new \MyIterator();
                foreach ($it as $item) {}
                print "SUCCESS";
        }
}

$items = new \MyIterator();
foreach ($items as $item) {}
$thread = new \MyThread();
$thread->start();
$thread->join();
--EXPECT--
SUCCESS
