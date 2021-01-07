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
    } catch(Throwable $throwable) {
        var_dump($throwable->getMessage());
    }
    var_dump($socket->setOption(\ThreadedSocket::SOL_SOCKET, \ThreadedSocket::SO_REUSEADDR, 1));
    var_dump($socket->getOption(\ThreadedSocket::SOL_SOCKET, \ThreadedSocket::SO_REUSEADDR));
?>
--EXPECTF--
ThreadedSocket::setOption() expects exactly 3 parameters, 0 given
string(%i) "Argument 1 passed to ThreadedSocket::setOption() must be of the type %s, string given"
bool(true)
int(1)