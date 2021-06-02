--TEST--
ThreadedSocket::connect() - AF_INET - test with empty parameters
--FILE--
<?php
    $rand = rand(1,999);
    $socket = new \ThreadedSocket(\ThreadedSocket::AF_INET, \ThreadedSocket::SOCK_DGRAM, 0);
    $socket->bind('0.0.0.0');
    // wrong parameter count

    try{
        $socket->connect();
    }catch(\ArgumentCountError $e){
        echo $e->getMessage() . PHP_EOL;
    }
    try {
        $socket->connect('0.0.0.0');
    } catch(\ArgumentCountError $exception) {
        var_dump($exception->getMessage());
    }
    $socket->connect('127.0.0.1', 31330+$rand);
    $socket->close();
?>
--EXPECTF--
ThreadedSocket::connect() expects at least 1 %s, 0 given
string(%d) "Socket of type AF_INET requires 2 arguments"
