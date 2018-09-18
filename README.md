MFTP README File

A client server program connected over Berkeley sockets and using TCP (transmission control protocol) to exchange data over a network. I used simple commands for the client to either receive from the server or send files to the server using commands like get and put. I also did other things like listing either the server’s or client’s contents of directories and files.

1. Begin by using the make command in the terminal and it will create two executables (Client & Server)
2. Start the server by simply running mftpserve executable
3. Afterwards start the client by simply running the mftp executable followed by either the hostname or IP address
4. The commands for the client are...
    exit (Quit)
    cd (Change directory of client)
    rcd (Change directory of server)
    ls (List contents in client's current working directory)
    rls (List contents in server's current working directory)
    get (Client gets file from server's current working directory)
    show (Client shows the contents of file sent from the server's current working directory) 
    put (Client puts file into server's current working directory)
5. To end the the server please use ^C
