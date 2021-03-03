--TEST--
Test parameter handling in ThreadedSocket::select() on non-win32.
--SKIPIF--
<?php
if (substr(PHP_OS, 0, 3) == 'WIN') {
	die('skip.. Not valid for Windows');
}
--FILE--
<?php
    $socket = new \ThreadedSocket(\ThreadedSocket::AF_INET, \ThreadedSocket::SOCK_STREAM, \ThreadedSocket::SOL_TCP);
    $socket->listen();

    try {
        \ThreadedSocket::select();
    } catch(\ArgumentCountError $e) {
        echo $e->getMessage() . PHP_EOL;
    }
    $read   = [$socket];
    $write  = null;
    $except = null;

    // Valid arguments, return immediately
    var_dump(\ThreadedSocket::select($read, $write, $except, 0));

    $read = [$socket];

    // Valid sec argument, wait 1 second
    var_dump(\ThreadedSocket::select($read, $write, $except, 1, 0, $errno));
    var_dump($errno);

    $read = [$socket];

    // Invalid sec argument, return immediately
    var_dump(\ThreadedSocket::select($read, $write, $except, -1, 0, $errno));
    var_dump($errno);

    $socket->close();
--EXPECTF--
ThreadedSocket::select() expects at least 4 %s, 0 given
int(0)
int(0)
int(0)
bool(false)
int(22)
