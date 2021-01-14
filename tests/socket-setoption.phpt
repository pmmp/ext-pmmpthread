--TEST--
Test of ThreadedSocket::setOption() with and without parameters
--FILE--
<?php
    $socket = new ThreadedSocket(\ThreadedSocket::AF_INET, \ThreadedSocket::SOCK_STREAM, \ThreadedSocket::SOL_TCP);
    try{
        $socket->setOption();
    }catch(\ArgumentCountError $e){
        echo $e->getMessage() . PHP_EOL;
    }

    try {
        $socket->setOption("hello", "world", new stdClass());
    } catch(\TypeError $throwable) {
        echo "bad types for ThreadedSocket::setOption()" . PHP_EOL;
    }
    var_dump($socket->setOption(\ThreadedSocket::SOL_SOCKET, \ThreadedSocket::SO_REUSEADDR, 1));
    var_dump($socket->getOption(\ThreadedSocket::SOL_SOCKET, \ThreadedSocket::SO_REUSEADDR));
?>
--EXPECTF--
ThreadedSocket::setOption() expects exactly 3 %s, 0 given
bad types for ThreadedSocket::setOption()
bool(true)
int(1)