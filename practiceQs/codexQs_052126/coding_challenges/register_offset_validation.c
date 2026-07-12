/* ### Challenge 8: Register Offset Validation                                                          
 
You are exposing a debug-only peek/poke path for internal tools.                                     
                                                                                                     
Requirements:                                                                                        
                                                                                                     
- `width` may be 1, 2, 4, or 8 bytes.                                                                
- `offset` must be aligned to `width`.                                                               
- access must fit entirely inside `region_size`.                                                     
- Reject all other widths.       

Implement:                                                                                           
*/
                                                                                                     
bool valid_mmio_access(uint32_t offset, uint32_t width, uint32_t region_size) {

    if (!(width == 1 || width == 2 ||
          width == 4 || width == 8)) {
        return false;
    }

    if (offset & (width - 1)) {
        return false;
    }

    if (width > region_size) {
        return false;
    }

    if (offset > region_size - width) {
        return false;
    }

    return true;
}
/*
 *  Follow-ups:

 - Why is unrestricted peek/poke dangerous?
    Answer: Malicious users or incompetent ones can place
            hw in a vulnerable state if improperly accessed.
 - How would production firmware restrict this?
    Answer: Access can be restricted through various levels of the
            stack. Kernel/driver can be designed to only expose
            certain parts of MMIO and have strong checks
            to ensure access is within bounds. At the firmware level
            access can be independently verified again.
 - What authorization or build-mode checks should exist?
    Answer: If this is only for internal tools we can have build flags
            to only expose this MMIO for internally built "debug"
            packages. User packages will not include these ioctls.
 */
