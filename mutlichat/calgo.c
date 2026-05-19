ALGORITHM FOR TCP ECHO CLIENT

1. Start the program.

2. Check whether the server IP address is provided in the command line.
   - If not provided, display:
     "Usage: client <server_ip>"
   - Terminate the program.

3. Create a TCP socket using socket().

4. Fill the server address structure:
   - Set address family as AF_INET
   - Convert server IP using inet_addr()
   - Set port number using htons(PORT)

5. Connect the client socket to the server using connect().

6. If connection is successful:
   - Display:
     "Connected! Type messages:"

7. Repeat the following steps until end of input:
   - Read a message from keyboard using fgets()
   - Send the message to server using send()
   - Receive echoed message from server using recv()
   - Display the received message

8. Close the socket.

9. End the program.
