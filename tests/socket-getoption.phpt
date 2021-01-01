--TEST--
Test of Socket::getOption() with and without parameters
--FILE--
<?php
    $socket = new Socket(\Socket::AF_INET, \Socket::SOCK_STREAM, \Socket::SOL_TCP);

    try{
        $socket->getOption();
    }catch(\ArgumentCountError $e){
        echo $e->getMessage() . PHP_EOL;
    }

    try {
        $socket->getOption("hello", "world");
    } catch(\TypeError $throwable) {
        echo "bad types for Socket::getOption()" . PHP_EOL;
    }
    var_dump($socket->getOption(\Socket::SOL_SOCKET, \Socket::SO_REUSEADDR));
?>
--EXPECTF--
Socket::getOption() expects exactly 2 %s, 0 given
bad types for Socket::getOption()
int(%i)
