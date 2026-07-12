//### Lab 3: Signed/Unsigned Length Bug                                                                
 
 //Prompt:
 
 //> A parser rejects very large packets, but a negative length still causes memory corruption.         
 
 //```c                                                                                                 
 #include <stdint.h>
 #include <string.h>
                                                                                                      
 #define FW_MAX_DATA 128
                                                                                                      
 struct fw_packet {
     int16_t len;
     uint8_t data[FW_MAX_DATA];
 };
 
 int parse_fw_packet(const uint8_t *src, int16_t len, struct fw_packet *pkt)                          
 {                                                                                                    
     if (len > FW_MAX_DATA)
         return -1;                                                                                   
 
     pkt->len = len;
     memcpy(pkt->data, src, len);                                                                     
     return 0;                                                                                        
 }
/*
 ```                                                                                                  
 
 Questions: 
 
 - What happens if `len` is negative?                                                                 
 - What type does `memcpy` expect for its third argument?                                             
 - How can signed-to-unsigned conversion create a huge copy?                                          
 - What checks are missing?
 
 Expected fix direction:
                                                                                                      
 - Reject `len < 0`.
 - Use unsigned lengths for wire formats only after validation.                                       
 - Check `src` and `pkt`.
 - Convert to `size_t` only after bounds are proven.
 
*/
