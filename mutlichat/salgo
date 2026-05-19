ALGORITHM FOR MULTI-CLIENT TCP ECHO SERVER

1. Start the program.

2. Create a TCP socket using socket().

3. Fill the server address structure:
   - Set address family as AF_INET
   - Set IP address as INADDR_ANY
   - Set port number using htons(PORT)

4. Bind the socket to the specified port using bind().

5. Put the socket into listening mode using listen().

6. Display:
   "Server listening on port..."

7. Repeat forever:
   a) Accept a client connection using accept().
   b) Display:
      "New client connected!"

   c) Create a child process using fork().

   d) If child process:
      - Close listening socket (server_fd)
      - Repeat forever:
        i. Clear buffer using memset()
        ii. Receive message using recv()
        iii. Display received message
        iv. Send the same message back using send()

   e) If parent process:
      - Close connected client socket (client_fd)
      - Continue accepting new clients.

8. End the program.
