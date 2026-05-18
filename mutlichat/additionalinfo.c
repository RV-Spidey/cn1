/*
 * ADDITIONAL INFO — Common doubts explained with code context
 *
 * This file covers 3 concepts:
 *   1. What address does bind() refer to?
 *   2. What does accept() do line by line?
 *   3. When does the argc < 2 check trigger?
 */


/* ===========================================================================
   CONCEPT 1 — Which address does bind() refer to?
   ===========================================================================

   Before bind() is called, we fill a struct called 'server':

        server.sin_family      = AF_INET;    // IPv4
        server.sin_addr.s_addr = INADDR_ANY; // accept from any IP/interface
        server.sin_port        = htons(PORT);// port 4444 in network byte order

   bind() takes that struct and glues it to the socket:

        bind(server_fd, (struct sockaddr*)&server, sizeof(server));

   Before bind():   socket server_fd exists but has NO address
   After  bind():   socket server_fd is tied to  IP=any, Port=4444

   That is why clients can reach the server by connecting to port 4444.
   Without bind(), the OS has no idea which port to listen on.
*/


/* ===========================================================================
   CONCEPT 2 — What does this accept() line mean?
   ===========================================================================

        client_fd = accept(server_fd, (struct sockaddr*)&client, &len);

   Argument by argument:

        server_fd              → the listening socket (created by socket())
                                 accept() watches THIS for incoming connections

        (struct sockaddr*)&client → a pointer to the 'client' struct
                                    accept() will FILL this with the
                                    connecting client's IP and port

        &len                   → pointer to the size of the client struct
                                 you pass the size IN so accept() knows
                                 how much space it has to write into

   What accept() does step by step:
     1. Blocks (waits) until a client calls connect()
     2. Completes the TCP handshake automatically (SYN, SYN-ACK, ACK)
     3. Fills the 'client' struct with the client's IP and port
     4. Creates a BRAND NEW socket dedicated to this one client
     5. Returns that new socket as client_fd

   Why two sockets exist after accept():

        server_fd  →  still listening for MORE clients  (never used for send/recv)
        client_fd  →  private line to THIS one client   (used for send/recv)

   Think of it like a receptionist:
        server_fd = front desk phone  → always open for new callers
        client_fd = internal extension→ handed to the child process
                                        to handle that specific caller
*/


/* ===========================================================================
   CONCEPT 3 — When does argc < 2 trigger?
   ===========================================================================

   When you run:    ./client 127.0.0.1
        argc    = 2
        argv[0] = "./client"
        argv[1] = "127.0.0.1"   ← the IP the program needs

   When you run:    ./client          (forgot to pass the IP)
        argc    = 1
        argv[0] = "./client"
        argv[1] = doesn't exist  ← accessing this would CRASH the program

   So the check:

        if (argc < 2) { printf("Usage: client <server_ip>\n"); exit(1); }

   is a GUARD. It prevents the program from crashing on this line below it:

        server.sin_addr.s_addr = inet_addr(argv[1]); // crashes if argv[1] missing

   Simple rule:
        argc < 2  → no IP was passed → exit safely with a helpful message
        argc >= 2 → argv[1] exists   → safe to use
*/
