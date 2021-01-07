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
    } catch(Throwable $throwable) {
        var_dump($throwable->getMessage());
    }
    var_dump($socket->getOption(\ThreadedSocket::SOL_SOCKET, \ThreadedSocket::SO_REUSEADDR));
?>
--EXPECTF--
ThreadedSocket::getOption() expects exactly 2 parameters, 0 given
string(%i) "Argument 1 passed to ThreadedSocket::getOption() must be of the type %s, string given"
int(%i)