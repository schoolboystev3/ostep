/*
### Challenge 8B: Firmware Command Validator
 
 A host driver or management daemon sends commands to firmware. Before executing a command, firmware must validate the command header.
 
 Command format:

  Requirements:

 - Return `0` if the command is valid.
 - Return `-1` if `cmd` is `NULL`.
 - Return `-2` if `opcode` is unknown.
 - Return `-3` if `length > CMD_MAX_PAYLOAD`.
 - Return `-4` if any reserved flag bit is set.
 - Return `-5` if `CMD_FLAG_PRIVILEGED` is set but `caller_is_privileged` is false.
 - `CMD_GET_STATUS` is allowed in any state.
 - `CMD_GET_CAPS` is allowed in any state except `FW_BOOTING`.
 - `CMD_RESET` is allowed only in `FW_READY`, `FW_ACTIVE`, or `FW_FAULTED`.
 - `CMD_PEEK` is allowed only if caller is privileged and state is not `FW_BOOTING`.
 - `CMD_POKE` is allowed only if caller is privileged and state is `FW_READY`.
 
 ```c
 */
 #define CMD_FLAG_ASYNC      (1u << 0)
 #define CMD_FLAG_PRIVILEGED (1u << 1)
 #define CMD_FLAG_RESERVED   (~(CMD_FLAG_ASYNC | CMD_FLAG_PRIVILEGED))
 
 #define CMD_MAX_PAYLOAD     256u
 
 enum fw_cmd_opcode {
     CMD_GET_STATUS = 1,
     CMD_GET_CAPS   = 2,
     CMD_RESET      = 3,
     CMD_PEEK       = 4,
     CMD_POKE       = 5,
 };
 
 enum fw_state {
     FW_BOOTING,
     FW_READY,
     FW_ACTIVE,
     FW_RESETTING,
     FW_FAULTED,
 };
 
 struct fw_cmd_hdr {
     uint16_t opcode;
     uint16_t length;
     uint32_t flags;
 };

 int validate_fw_cmd(const struct fw_cmd_hdr *cmd,
                     enum fw_state state,
                     bool caller_is_privileged) {
        if (!cmd) {
            return -1;
        }
        if (cmd->opcode < 1 || cmd->opcode > 5) {
            return -2;
        }
        if (cmd->length > CMD_MAX_PAYLOAD) {
            return -3;
        }
        if (cmd->flags & CMD_FLAG_RESERVED) {
            return -4;
        }
        if (cmd->flags & CMD_FLAG_PRIVILEGED && !caller_is_privileged) {
            return -5;
        }

        int valid = 0;
        switch (cmd->opcode) {
            case CMD_GET_STATUS:
                break;
            case CMD_GET_CAPS:
                if (state == FW_BOOTING) {
                    valid = -6;
                }
                break;
            case CMD_RESET: 
                if (state != FW_READY || state != FW_ACTIVE || 
                    state != FW_FAULTED) {
                    valid = -6;
                }
                break;
            case CMD_PEEK:
                if (state == FW_BOOTING || !caller_is_privileged) {
                    valid = -6;
                }
                break;
            case CMD_POKE:
                if (state != FW_READY || !caller_is_privileged) {
                    valid = -6;
                }
                break;
        }
        return valid;
 }          
/*
 - Why validate reserved flags instead of ignoring them?
    Answer: We should not trust users. Validating input to
            not have reserved flags set protects us from malicious
            intents.
 - Which of these commands should be allowed in production builds?
    Answer: It seems like based on the spec that getting status, caps
            and resetting is allowed in production builds because they
            do not require a privileged user.
 - Where should payload-specific validation happen?
    Answer: The payload is not accessible here. But before performing
            actions with the actual payload data, it should be verified.
 - How would you version this command ABI?
    Answer: There should be a version number attached to the handler
            or internal code. We can add a version number to the 
            fw_cmd_hdr so that we can check against internal version.
 - What if the command comes from a user-space daemon through a kernel driver?
    Answer: Not sure what this question is leading to specifically.
    But if it's versioning: user package version can be checked 
    against driver version number
 */
