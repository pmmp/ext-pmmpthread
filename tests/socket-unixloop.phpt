--TEST--
Unix domain socket Loopback test
--SKIPIF--
<?php
if (substr(PHP_OS, 0, 3) == 'WIN') {
	die('skip.. Not valid for Windows');
}
?>
--FILE--
<?php
	$sock_path = sprintf("/tmp/%s.sock", uniqid());

    if (file_exists($sock_path))
        die('Temporary socket already exists.');

    /* Setup socket server */
    $server = new ThreadedSocket(\ThreadedSocket::AF_UNIX, \ThreadedSocket::SOCK_STREAM, 0);
    if (!$server) {
        die('Unable to create AF_UNIX socket [server]');
    }
    if (!$server->bind($sock_path)) {
        die("Unable to bind to $sock_path");
    }
    if (!$server->listen(2)) {
        die('Unable to listen on socket');
    }

    /* Connect to it */
    $client = new ThreadedSocket(\ThreadedSocket::AF_UNIX, \ThreadedSocket::SOCK_STREAM, 0);
    if (!$client) {
        die('Unable to create AF_UNIX socket [client]');
    }
    if (!$client->connect($sock_path)) {
        die('Unable to connect to server socket');
    }

    /* Accept that connection */
    /** @var ThreadedSocket $socket */
    $socket = $server->accept(\ThreadedSocket::class);
    if (!$socket) {
        die('Unable to accept connection');
    }

    $client->write("ABCdef123\n");

    $data = $socket->read(10, 0, \ThreadedSocket::BINARY_READ);
    var_dump($data);

    $client->close();
    $socket->close();
    $server->close();
    @unlink($sock_path);
?>
--EXPECT--
string(10) "ABCdef123
"
