/*
### L04: Safe IOCTL Validator

Definitions:
*/

#define CMD_VERSION 1
#define CMD_MAX_PAYLOAD 256

struct user_cmd {
    uint16_t version;
    uint16_t opcode;
    uint32_t flags;
    uint32_t payload_len;
    uint64_t user_ptr;
};

int validate_user_cmd(const struct user_cmd *cmd) {
    // User input must be validated carefully because
    // it can harm the hardware. Always distrust
    // user inputs and data
    if (!cmd) {
        return -1;
    }

    if (cmd->version != CMD_VERSION) {
        return -2;
    }

    if (cmd->flags & CMD_RESERVED_MASK) {
        return -2;
    }

    if (cmd->opcode > CMD_OPCODE_MAX) {
        return -2
    }

    if (cmd->payload_len > CMD_MAX_PAYLOAD) {
        return -2;
    }
    return 0;
}

/*
Requirements:

- Reject null.
- Check version.
- Reject unknown/reserved flags.
- Check `payload_len <= CMD_MAX_PAYLOAD`.
- Check opcode range.
- Explain why the kernel must copy and validate user memory carefully.

Follow-ups:

- How do you version an ioctl ABI?
    Answer: Everytime there are updates to ioctl interfaces, increment
            the version number?
- Why include reserved fields?
    Answer: They may be unlocked at some point in the future? Or 
            useful for other commands or maybe it just matches
            the hardware interface?
- What should happen across 32-bit/64-bit user space?
    Answer: Not sure what is meant by this

*/
