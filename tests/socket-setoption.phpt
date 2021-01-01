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
    } catch(Throwable $throwable) {
        var_dump($throwable->getMessage());
    }
    var_dump($socket->setOption(\Socket::SOL_SOCKET, \Socket::SO_REUSEADDR, 1));
    var_dump($socket->getOption(\Socket::SOL_SOCKET, \Socket::SO_REUSEADDR));
?>
--EXPECTF--
Socket::setOption() expects exactly 3 parameters, 0 given
string(%i) "Argument 1 passed to Socket::setOption() must be of the type %s, string given"
bool(true)
int(1)