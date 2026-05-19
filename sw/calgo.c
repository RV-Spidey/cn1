ALGORITHM FOR STOP AND WAIT CLIENT

1. Start the program.

2. Create a UDP socket using socket().

3. Fill the server address structure:
   - Set address family as AF_INET
   - Set port number using htons(PORT)
   - Set server IP using inet_addr()

4. Initialize:
   - frame_id = 0
   - need_new = 1

5. Repeat forever:

   a) If need_new is true:
      - Set frame type as data frame
      - Set sequence number = frame_id
      - Set acknowledgement field = 0
      - Read data from user

   b) Send the frame to server using sendto().

   c) Display:
      "[+] Frame sent"

   d) Receive acknowledgement frame using recvfrom().

   e) Check ACK value:
      - If ACK == frame_id + 1:
          i. Display:
             "[+] ACK received"
          ii. Increment frame_id
          iii. Set need_new = 1
      - Else:
          i. Display:
             "[-] ACK not received — resending"
          ii. Set need_new = 0

6. End the program.
