/*
 * STOP AND WAIT ARQ — FULL EXPLANATION (v2)
 *
 * What is Stop and Wait?
 *   A reliability protocol built on top of UDP.
 *   UDP by itself does NOT guarantee delivery — packets can get lost.
 *   Stop and Wait fixes this by:
 *     1. Sending ONE frame at a time
 *     2. Waiting for an ACK (acknowledgement) before sending the next
 *     3. Resending the same frame if ACK is not received
 *
 * Why UDP and not TCP?
 *   TCP already handles reliability automatically.
 *   We use UDP to manually demonstrate HOW reliability works.
 *
 * Files:
 *   sw_v2_server.c  —  receives frames, sends ACKs
 *   sw_v2_client.c  —  sends frames, waits for ACKs
 *
 * Run:
 *   Terminal 1:  ./sw_v2_server
 *   Terminal 2:  ./sw_v2_client
 */


/* ===========================================================================
   THE FRAME STRUCT — used by both client and server
   ===========================================================================

    typedef struct {
        int  seq;        // sequence number — which frame is this? (0, 1, 2...)
        int  ack;        // acknowledgement number — used only in ACK replies
        int  type;       // 1 = data frame (client → server)
                         // 0 = ACK  frame (server → client)
        char data[1024]; // the actual message content
    } Frame;

   Why sequence numbers?
     Suppose a frame is sent twice (retransmit).
     Without seq numbers, the server cannot tell if it's a NEW frame
     or a DUPLICATE. seq_no lets the server check:
       "Is this the frame I expected, or one I already processed?"
*/


/* ===========================================================================
   SERVER LOGIC  (sw_v2_server.c)
   ===========================================================================

   STEP 1 — socket(AF_INET, SOCK_DGRAM, 0)
     Creates a UDP socket. SOCK_DGRAM = UDP (no connection, no handshake).

   STEP 2 — Fill serverAddr struct
     sin_family  = AF_INET       → IPv4
     sin_port    = htons(PORT)   → port in network byte order
     sin_addr    = INADDR_ANY    → accept from any IP

   STEP 3 — bind()
     Reserves the port so incoming UDP packets reach this program.
     (Servers always bind. Clients do not need to bind.)

   STEP 4 — recvfrom()
     Waits (BLOCKS) until a frame arrives from the client.
     Unlike recv(), recvfrom() also fills the 'client' struct
     with the sender's IP + port — needed to send ACK back.

     recvfrom(sockfd, &recv_frame, sizeof(Frame), 0,
              (struct sockaddr *)&client, &len);
       sockfd      → our socket
       &recv_frame → where to store the incoming frame
       sizeof(Frame) → max bytes to read
       &client     → OUTPUT: filled with sender's address (so we can reply)

   STEP 5 — Check if frame is correct
     if (recv_frame.seq == expected)
       Two things must be true:
         a) We actually received something (recvfrom returned > 0 bytes)
         b) The sequence number matches what we expected
            If seq is wrong → it's a duplicate or out-of-order frame → ignore

   STEP 6 — Build and send ACK
     ack_frame.type = 0;              → this is an ACK frame
     ack_frame.ack  = expected + 1;   → "I got frame N, now send frame N+1"

     sendto(sockfd, &ack_frame, sizeof(ack_frame), 0,
            (struct sockaddr *)&client, len);
       We must use sendto() (not send()) because UDP has no connection —
       we must specify the destination every single time.

   STEP 7 — expected++
     Move to the next expected sequence number.
*/


/* ===========================================================================
   CLIENT LOGIC  (sw_v2_client.c)
   ===========================================================================

   STEP 1 — socket() + fill serverAddr
     Same as always. Client does NOT bind or listen.

   STEP 2 — need_new_data flag
     int need_new_data = 1;

     This flag controls whether to ask the user for new input
     or to resend the previous frame without asking.

       need_new_data = 1 → ACK received last time → get new message from user
       need_new_data = 0 → ACK NOT received       → skip scanf, resend same frame

   STEP 3 — Build and send frame (inside loop)
     Only rebuild the frame if need_new_data == 1:
       send_frame.type = 1;        → data frame
       send_frame.seq  = frame_id; → current sequence number
       scanf → read message from keyboard into send_frame.data

     sendto() sends the frame whether it's new or a retransmit.

   STEP 4 — STOP and wait for ACK
     recvfrom() BLOCKS here — this is the "STOP and WAIT" moment.
     Client does absolutely nothing until the server replies.

   STEP 5 — Check the ACK
     if (recv_frame.ack == frame_id + 1)
       The server's ACK contains ack = frame_id + 1.
       This means server successfully got frame_id and wants the next one.
         → frame_id++       (advance to next frame)
         → need_new_data=1  (ask user for new input next iteration)
     else
         → need_new_data=0  (resend same frame_id next iteration)
         → frame_id stays the same
*/


/* ===========================================================================
   FULL FLOW EXAMPLE
   ===========================================================================

   CLIENT                              SERVER
     │                                   │
     │  frame(seq=0, type=1, "hello")    │
     │ ─────────────────────────────→    │  seq==expected(0) ✓
     │                                   │  prints "hello"
     │                                   │  sends ack=1
     │    ack_frame(ack=1, type=0)       │
     │ ←─────────────────────────────    │
     │  ack==frame_id+1 ✓               │
     │  frame_id becomes 1               │
     │                                   │
     │  frame(seq=1, type=1, "world")    │
     │ ─────────────────────────────→    │  seq==expected(1) ✓
     │                                   │  prints "world"
     │                                   │  sends ack=2
     │    ack_frame(ack=2, type=0)       │
     │ ←─────────────────────────────    │
     │  frame_id becomes 2               │
     │                                   │
     │  frame(seq=2) ──────── LOST ──→  (never arrives)
     │                                   │
     │  (no ACK comes back)              │
     │  need_new_data = 0                │
     │  resends frame(seq=2, "hi")       │
     │ ─────────────────────────────→    │  seq==expected(2) ✓
     │    ack_frame(ack=3)               │
     │ ←─────────────────────────────    │
     │  frame_id becomes 3               │


   KEY POINTS FOR EXAM
   ─────────────────────────────────────────────────────────
   Q: What protocol does Stop-and-Wait use?
   A: UDP (SOCK_DGRAM) — reliability is added manually

   Q: What does ack = frame_id + 1 mean?
   A: "I received frame N, please send frame N+1 next"

   Q: What happens if ACK is lost?
   A: Client resends the same frame (need_new_data=0, frame_id unchanged)

   Q: What does the seq number prevent?
   A: Duplicate frames — server rejects any frame whose seq != expected

   Q: Difference between send() and sendto()?
   A: sendto() is for UDP — you must specify destination every time
      send() is for TCP — destination is fixed after connect()
*/
