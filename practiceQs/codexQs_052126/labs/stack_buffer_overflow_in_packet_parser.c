//### Lab 1: Stack Buffer Overflow In Packet Parser
 
 //Prompt:
 
 //> A firmware parser crashes when given a malformed management packet. Find the bug and propose a fix.
 
 #include <stdint.h>
 #include <string.h>
 
 #define MAX_PAYLOAD 64 
 
 struct mgmt_msg {
     uint8_t opcode;
     uint8_t len;
     uint8_t payload[MAX_PAYLOAD];
 };
 
 int parse_msg(const uint8_t *buf, uint32_t buf_len, struct mgmt_msg *out)
 {
     uint8_t tmp[32];
 
     if (buf_len < 2)
         return -1;
 
     out->opcode = buf[0];
     out->len = buf[1];
 
     memcpy(tmp, &buf[2], out->len);
     memcpy(out->payload, tmp, out->len);
 
     return 0;
 }
 /*
 
 Questions:
 
 - What input controls the copy length?
    Answer: buf_len
 - Which buffer overflows first?
    Answer: tmp[32]
 - What if `buf_len == 2` and `out->len == 64`?
    Answer: That would cause the packet reader to read garbage values
            going beyond where there is valid data in the packet.
 - What checks are missing?
    Answer: buf[1] must be < buf_len-2.
                *buf_len refers to the max size of the buffer
                *buf[1] refers to the length of the payload
            tmp's length should be atleast MAX_PAYLOAD
            Both input pointers need to be verified.

 - Should the temporary buffer exist at all?
    Answer: tmp seems redundant. we can copy directly from buf to payload
 
 Expected fix direction:
 
 - Check `out != NULL` and `buf != NULL` if relevant.
 - Check `out->len <= MAX_PAYLOAD`.
 - Check `out->len <= sizeof(tmp)` if using `tmp`.
 - Check `buf_len >= 2 + out->len`, with overflow-safe arithmetic.
 - Prefer copying directly into `out->payload` if no temporary is needed.
 */
