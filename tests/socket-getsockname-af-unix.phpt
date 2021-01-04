--TEST--
Basic test of ThreadedSocket::getSockName() with AF_UNIX sockets
--SKIPIF--
<?php
if (substr(PHP_OS, 0, 3) == 'WIN') {
	die('skip.. Not valid for Windows');
}
--FILE--
<?php
    $address = sprintf("/tmp/%s.sock", uniqid());

    if (file_exists($address))
        die('Temporary file socket already exists.');

    $unixThreadedSocket = new \ThreadedSocket(\ThreadedSocket::AF_UNIX,\ThreadedSocket::SOCK_STREAM, 0);

    if (!$unixThreadedSocket->bind($address)) {
        die("Unable to bind to $address");
    }
    var_dump($unixThreadedSocket->getSockName());

    $unixThreadedSocket->close();
?>
--EXPECTF--
array(1) {
  ["host"]=>
  string(23) "/tmp/%s.sock"
}