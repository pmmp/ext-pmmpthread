--TEST--
Test if ThreadedSocket::recvfrom() receives data sent by ThreadedSocket::sendto() via IPv4 UDP
--CREDITS--
Copied from php/php-src and adjusted, originally created by 
Falko Menge <mail at falko-menge dot de>
PHP Testfest Berlin 2009-05-09
--FILE--
<?php
    $socket = new ThreadedSocket(\ThreadedSocket::AF_INET, \ThreadedSocket::SOCK_DGRAM, \ThreadedSocket::SOL_UDP);
    if (!$socket) {
        die('Unable to create AF_INET socket');
    }
    if (!$socket->setBlocking(false)) {
        die('Unable to set nonblocking mode for socket');
    }

    $address = '127.0.0.1';

    try{
        $socket->sendto('', 1, 0, $address);
    }catch(\ArgumentCountError $e){
        echo $e->getMessage() . PHP_EOL;
    }
    if (!$socket->bind($address, 1223)) {
        die("Unable to bind to $address:1223");
    }
    var_dump($socket->recvfrom($buf, 12, 0, $from, $port)); // false
    
    $msg = "Ping!";
    $len = strlen($msg);
    $bytes_sent = $socket->sendto($msg, $len, 0, $address, 1223);
    if ($bytes_sent == -1) {
        die('An error occurred while sending to the socket');
    } else if ($bytes_sent != $len) {
        die($bytes_sent . ' bytes have been sent instead of the ' . $len . ' bytes expected');
    }

    $from = "";
    $port = 0;
    try{
        $socket->recvfrom($buf, 12, 0);
    }catch(\ArgumentCountError $e){
        echo $e->getMessage() . PHP_EOL;
    }
    try{
        $socket->recvfrom($buf, 12, 0, $from);
    }catch(\ArgumentCountError $e){
        echo $e->getMessage() . PHP_EOL;
    }
    $bytes_received = $socket->recvfrom($buf, 12, 0, $from, $port);
    if ($bytes_received == -1) {
        die('An error occurred while receiving from the socket');
    } else if ($bytes_received != $len) {
        die($bytes_received . ' bytes have been received instead of the ' . $len . ' bytes expected');
    }
    echo "Received $buf from remote address $from and remote port $port" . PHP_EOL;

    $socket->close();
--EXPECTF--
Port must be provided for AF_INET
bool(false)
ThreadedSocket::recvfrom() expects at least 4 %s, 3 given
Port must be provided for AF_INET
Received Ping! from remote address 127.0.0.1 and remote port 1223
