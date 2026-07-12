/*
 *Requirements:

Return -1 if cmd is NULL.
Return -2 if src is NULL and len > 0.
Return -3 if len > CMD_MAX_PAYLOAD.
Set cmd->length on success.
Copy exactly len bytes.
Return 0 on success.
Follow-ups:

What happens if len came from an untrusted host command?
    Answer: If it is untrusted, then we should have strict checks to make sure 
            len is valid.
What if length is 16-bit and len is size_t?
    Answer: This is dangerous because it's possible that len exceeds length.
            So once again we'll need strict checks.
What if the command is shared with firmware/hardware and needs endianness conversion?
    Answer: As far as I know, payload is a byte array so endianness may be fine?
            But if the other fields in fw_cmd need endianness conversion we can 
            manually flip bit fields?
What if src overlaps cmd->payload?
    Answer: This is definitely dangerous and may cause data lose/overwritting depending
            on how they overlap. Perhaps another check should be added to prevent this
            behavior. Or we can state that if the two overlap it will result in undefined behavior
 */
#define CMD_MAX_PAYLOAD 256

struct fw_cmd {
    uint16_t opcode;
    uint16_t length;
    uint8_t payload[CMD_MAX_PAYLOAD];
};

int fw_cmd_set_payload(struct fw_cmd *cmd, const uint8_t *src, size_t len) {
    if (!cmd) {
        return -1;
    }
    if (!src && len > 0) {
        return -2;
    }
    if (len > CMD_MAX_PAYLOAD) {
        return -3;
    }

    for (size_t i = 0; i < len; i++) {
        cmd->payload[i] = src[i];
    }
    cmd->length = (uint16_t)len;
    return 0;
}
