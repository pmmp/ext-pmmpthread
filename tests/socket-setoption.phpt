--TEST--
Test of Socket::setOption() with and without parameters
--FILE--
<?php
    $socket = new Socket(\Socket::AF_INET, \Socket::SOCK_STREAM, \Socket::SOL_TCP);
    try{
        $socket->setOption();
    }catch(\ArgumentCountError $e){
        echo $e->getMessage() . PHP_EOL;
    }

    try {
        $socket->setOption("hello", "world", new stdClass());
    } catch(\TypeError $throwable) {
        echo "bad types for Socket::setOption()" . PHP_EOL;
    }
    var_dump($socket->setOption(\Socket::SOL_SOCKET, \Socket::SO_REUSEADDR, 1));
    var_dump($socket->getOption(\Socket::SOL_SOCKET, \Socket::SO_REUSEADDR));
?>
--EXPECTF--
Socket::setOption() expects exactly 3 %s, 0 given
bad types for Socket::setOption()
bool(true)
int(1)