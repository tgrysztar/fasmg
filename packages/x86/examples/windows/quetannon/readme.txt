
Quetannon is an example of using WinSock library to make TCP/IP connections, 
both as a client and as a server.

Coded in 2003 by Tomasz Grysztar, logo graphics by Michał Gąsior.

-------------------------------------------------------------------------------

To listen for the incoming connection, write port number into the edit box 
next to the "Listen" button and press Enter to start listening.

To connect to the remote server, write the the host name into appropriate edit
box, and press Enter to convert the name into IP number. Then write port number
into the edit box you will get moved to and press Enter to connect.

The most simple test is to run two instances of program, start listening on
some fixed port with the first one, and connect to it with the second on,
by using the same port number and "localhost" as a host name.

You can also connect to some remote services, like SMTP server on port 25 or
POP3 server on port 110. If you know the protocols, you might be even able to
check you mail box or send e-mail this way. Have fun!

-------------------------------------------------------------------------------
