--TEST--
Test creating and closing sockets
--DESCRIPTION--
Test that creating and closing sockets works as expected on all platforms (gh issue #798)
--FILE--
<?php
$socket = new \ThreadedSocket(\ThreadedSocket::AF_INET, \ThreadedSocket::SOCK_DGRAM, \ThreadedSocket::SOL_UDP);
echo "created\n";
$socket->close();
echo "closed\n";
?>
--EXPECTF--
created
closed
