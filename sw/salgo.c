ALGORITHM FOR STOP AND WAIT SERVER

1. Start the program.

2. Create a UDP socket using socket().

3. Fill the server address structure:
   - Set address family as AF_INET
   - Set port number using htons(PORT)
   - Set IP address as INADDR_ANY

4. Bind the socket to the server address using bind().

5. Display:
   "Server ready..."

6. Initialize:
   - expected = 0

7. Repeat forever:

   a) Receive a frame from client using recvfrom().

   b) Check sequence number:
      - If received frame sequence number == expected:
          
          i. Display received data.

          ii. Prepare ACK frame:
              - Set frame type = ACK frame
              - Set sequence number = 0
              - Set acknowledgement number = expected + 1

          iii. Send ACK to client using sendto().

          iv. Display:
              "[+] ACK sent"

          v. Increment expected value.

      - Else:
          
          i. Display:
             "[-] Wrong frame"

8. End the program.
