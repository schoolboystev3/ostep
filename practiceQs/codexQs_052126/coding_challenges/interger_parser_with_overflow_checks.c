/*### Challenge 18: Integer Parser With Overflow Checks
 
Implement a strict parser for signed 32-bit decimal integers. This is a common interview problem because it exposes overflow, sign handling, and input validation.

Requirements:

- Return `0` on success.
- Return `-1` if `s` or `out_value` is `NULL`.
- Return `-2` if the string is empty.
- Allow an optional leading `+` or `-`.
- Return `-3` if the sign is not followed by at least one digit.
- Return `-4` if any non-digit character appears after the optional sign.
- Return `-5` if the value would overflow or underflow `int32_t`.
- Accept `2147483647`.
- Accept `-2147483648`.
- Reject `2147483648`.
- Reject `-2147483649`.
- Do not call `atoi`, `strtol`, `sscanf`, or similar parser helpers.
- Do not modify `*out_value` on failure.

Implement:
*/

int what_digit(const char s) {
    switch(s) {
        case ('0'):
            return 0;
        case ('1'):
            return 1;
        case ('2'):
            return 2;
        case ('3'):
            return 3;
        case ('4'):
            return 4;
        case ('5'):
            return 5;
        case ('6'):
            return 6;
        case ('7'):
            return 7;
        case ('8'):
            return 8;
        case ('9'):
            return 9;
    }
    return -1;
}

int parse_i32(const char *s, int32_t *out_value) {
    if (!out_value) return -1;
    if (!s) return -2;

    bool negative = false;
    size_t index = 0;
    int64_t rv = 0;


    if (s[0] == '-') {
        negative = true;
        index++;
    } else if (s[0] == '+') {
        negative = false;
        index++;
    } else if (what_digit(s[0]) < 0) {
        return -3;
    }

    if (index == 1 && !s[index]) return -3;

    int64_t curr = 0;
    int64_t multiplier = 1;
    
    size_t i = (index == 0) ? strlen(s) : strlen(s) - 1;
    for (; i > 0; i--) {
        curr = what_digit(s[i]);
        if (curr < 0) {
            return -4;
        }
        rv += (curr * multiplier);
        multiplier *= 10;
    }

    // TODO: Accept/Reject specific values in req

    if (rv > INT32_MAX || rv < INT32_MIN) {
        return -5;
    }
    *out_value = (int32_t)rv;
    return 0;
}

/*
Follow-ups:

- Why is `atoi` dangerous?
- Why is `INT32_MIN` trickier than `INT32_MAX`?
- How would you add whitespace handling without making the parser too permissive?
- What tests would you write for signs, empty input, invalid characters, and overflow?

 Variants:

- Implement `parse_u32_hex_or_dec`, accepting `1234` and `0x4d2`.
- Add min/max bounds: `parse_i32_range(s, min, max, out)`.
- Parse a register offset and require 4-byte alignment.
- Convert it into a command-line option parser where invalid input must produce a clear error string.
- Debug a parser that overflows before checking the limit.
*/
