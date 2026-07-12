//### L02: PCIe Link Status Parser

struct pcie_link_info {
    uint8_t speed_gen;
    uint8_t width;
    bool training;
    bool degraded;
};

int parse_pcie_link_status(uint16_t link_status,
                           uint8_t expected_gen,
                           uint8_t expected_width,
                           struct pcie_link_info *out) {
    if (!out) {
        return -1;
    }

    out->degraded = false;

    out->speed_gen = (link_status & LINK_SPEED_GEN_MASK) >> LINK_SPEED_GEN_OFFSET;
    out->width = (link_status & LINK_WIDTH_MASK) >> LINK_WIDTH_OFFSET;
    out->training = (link_status & LINK_TRAINING_MASK) >> LINK_TRAINING_OFFSET;

    if (out->speed_gen != expected_gen || out->width != expected_width) {
        out->degraded = true;
        // Check other registers for errors in the link. Attempt to retrain
        // if possible. Otherwise mark faulted so it's not used and so it
        // can be inspected further.
    }
    // Not sure what the training bool is for?
    return 0;
}

/*

Requirements:

- Decode speed and width from a simplified link status register.
- Mark degraded if speed or width is below expected.
- Return `-1` for invalid output pointer.
- Explain what you would check next if degraded.

Follow-ups:

- What are LTSSM, speed, and width?
    Answer: LTSSM is the hardware state machine for the pcie link
            Speed refers to how fast data is traveling through the link
            Width refers to how much how much data is passing 
            through at any given moment on the link
- What can cause lower-than-expected training?
    Answer: Could be transient error or a actual issue with hw
            What it is specifically I don't know
- What should the daemon do versus the kernel?
    Answer: The daemon can orchestrate all of these accesses but
            the kernel protects actual MMIO/HW access.
*/
