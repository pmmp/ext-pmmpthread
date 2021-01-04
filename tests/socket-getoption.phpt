--TEST--
Test of ThreadedSocket::getOption() with and without parameters
--FILE--
<?php
    $socket = new ThreadedSocket(\ThreadedSocket::AF_INET, \ThreadedSocket::SOCK_STREAM, \ThreadedSocket::SOL_TCP);

    try{
        $socket->getOption();
    }catch(\ArgumentCountError $e){
        echo $e->getMessage() . PHP_EOL;
    }

    try {
        $socket->getOption("hello", "world");
    } catch(\TypeError $throwable) {
        echo "bad types for ThreadedSocket::getOption()" . PHP_EOL;
    }
    var_dump($socket->getOption(\ThreadedSocket::SOL_SOCKET, \ThreadedSocket::SO_REUSEADDR));
?>
--EXPECTF--
ThreadedSocket::getOption() expects exactly 2 %s, 0 given
bad types for ThreadedSocket::getOption()
int(%i)
