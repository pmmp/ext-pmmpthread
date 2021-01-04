--TEST--
Basic test of ThreadedSocket::getSockName() with AF_INET sockets
--FILE--
<?php
    $address = '127.0.0.1';
    $port = 31330 + rand(1,999);

    $inetThreadedSocket = new \ThreadedSocket(\ThreadedSocket::AF_INET,\ThreadedSocket::SOCK_STREAM, \ThreadedSocket::SOL_TCP);

    if (!$inetThreadedSocket->bind($address, $port)) {
        die("Unable to bind to $address");
    }
    var_dump($inetThreadedSocket->getSockName(), $inetThreadedSocket->getSockName(false));

    $inetThreadedSocket->close();
?>
--EXPECTF--
array(2) {
  ["host"]=>
  string(9) "127.0.0.1"
  ["port"]=>
  int(%d)
}
array(1) {
  ["host"]=>
  string(9) "127.0.0.1"
}