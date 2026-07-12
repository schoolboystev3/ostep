/* ### Challenge 9: Capability Register Parser

A capability register:
Fresh variant: add a `driver_supported_caps` mask and return an error if firmware reports a required unsupported capability.

*/
#define CAP_RTOS          (1u << 0)
#define CAP_SECURE_BOOT   (1u << 1)
#define CAP_P2P           (1u << 2)
#define CAP_HBM           (1u << 3)
#define CAP_GEN_MASK      (0xfu << 8)
#define CAP_GEN_SHIFT     8
#define CAP_LINKS_MASK    (0xffu << 16)
#define CAP_LINKS_SHIFT   16

#define DRIVER_SUPPORTED_CAPS (CAP_RTOS | CAP_SECURE_BOOT | \
                               CAP_P2P | CAP_HBM | \
                               CAP_GEN_MASK | CAP_LINKS_MASK)

struct fw_caps {
    bool has_rtos;
    bool has_secure_boot;
    bool has_p2p;
    bool has_hbm;
    uint32_t generation;
    uint32_t num_links; 
    bool reserved_bits_detected;
};

struct fw_caps parse_fw_caps(uint32_t cap_reg) {
    struct fw_caps fwc = {0};

    if (cap_reg & ~DRIVER_SUPPORTED_CAPS) {
        fwc.reserved_bits_detected;
    }

    fwc.has_rtos = (cap_reg & CAP_RTOS) != 0;
    fwc.has_secure_boot = (cap_reg & CAP_SECURE_BOOT) != 0;
    fwc.has_p2p = (cap_reg & CAP_P2P) != 0;
    fwc.has_hbm = (cap_reg & CAP_HBM) != 0;

    fwc->generation = (cap_reg & CAP_GEN_MASK) >> CAP_GEN_SHIFT;
    fwc->num_links = (cap_reg & CAP_LINKS_MASK) >> CAP_LINKS_SHIFT;

    return fwc;
}

/* 
 * Follow ups
 
 - How would you support future generations?
    Answer: This function can be extended to handle more capabilities
 - What if a capability bit is set but unsupported by the driver?
    Answer: The driver should mask off that bit and return 
            a clear error message saying the driver does not support
            that capability
 - Where does version negotiation belong?
    Answer: At every interaction in layers of the stack there requires a version
            handshake. This ensures proper and safe execution.
 */
