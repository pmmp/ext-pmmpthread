--TEST--
ThreadedSocket::connect() - AF_UNIX - test with empty parameters
--SKIPIF--
<?php
if (substr(PHP_OS, 0, 3) == 'WIN') {
	die('skip.. Not valid for Windows');
}
--FILE--
<?php
    $rand = rand(1,999);
    $socket = new \ThreadedSocket(\ThreadedSocket::AF_UNIX,\ThreadedSocket::SOCK_DGRAM, 0);

    if (!$socket->setBlocking(false)) {
        die('Unable to set nonblocking mode for socket');
    }
    $address = sprintf("/tmp/%s.sock", uniqid());

    if (file_exists($address))
        die('Temporary file socket already exists.');

    if (!$socket->bind($address)) {
        die("Unable to bind to $address");
    }

    try{
        $socket->connect();
    }catch(\ArgumentCountError $e){
        echo $e->getMessage() . PHP_EOL;
    }
    $socket->connect($address);
    $socket->connect($address, 31330+$rand);

    $socket->close();
?>
--EXPECTF--
ThreadedSocket::connect() expects at least 1 parameter, 0 given
