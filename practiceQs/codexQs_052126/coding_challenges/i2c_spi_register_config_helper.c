/*### Challenge 11: I2C/SPI Register Config Helper

 Requirements:

 - Return `0` on success.
 - Return `-1` if `cfg` or `out_reg` is `NULL`.
 - Return `-2` if `mode` is invalid.
 - Return `-3` if `clock_divider` is `0` or greater than `255`.
 - For I2C:
   - `BUS_CTRL_MODE_SPI` must not be set.
   - `spi_cpol` and `spi_cpha` must be false.
   - `chip_select` must be `0`.
   - `BUS_CTRL_I2C_10BIT` is set only if `i2c_10bit_addr` is true.
 - For SPI:
   - `BUS_CTRL_MODE_SPI` must be set.
   - `chip_select` must fit in 4 bits.
   - `BUS_CTRL_SPI_CPOL` and `BUS_CTRL_SPI_CPHA` reflect the requested SPI mode.
   - `i2c_10bit_addr` must be false.
 - `BUS_CTRL_ENABLE` is set only if `enable` is true.
 - Store the final register value in `*out_reg`.


An embedded controller can configure a peripheral bus for either I2C or SPI. You are writing a helper that validates a requested configuration and builds the control register value.

*/

#define BUS_CTRL_ENABLE        (1u << 0)
#define BUS_CTRL_MODE_SPI      (1u << 1)  // 0 = I2C, 1 = SPI
#define BUS_CTRL_SPI_CPOL      (1u << 2)
#define BUS_CTRL_SPI_CPHA      (1u << 3)
#define BUS_CTRL_I2C_10BIT     (1u << 4)
#define BUS_CTRL_CLKDIV_MASK   (0xffu << 8)   // bits [15:8]
#define BUS_CTRL_CLKDIV_SHIFT  8
#define BUS_CTRL_CS_MASK       (0xfu << 16)   // bits [19:16], SPI only
#define BUS_CTRL_CS_SHIFT      16


enum bus_mode {
    BUS_I2C,
    BUS_SPI,
};

struct bus_config {
    enum bus_mode mode;
    uint32_t clock_divider;   // 1..255
    uint32_t chip_select;     // 0..15, SPI only
    bool spi_cpol;
    bool spi_cpha;
    bool i2c_10bit_addr;
    bool enable;
};

// Implement:

int build_bus_ctrl(const struct bus_config *cfg, uint32_t *out_reg) {
    if (!cfg || !out_reg) {
        return -1;
    }
    if (cfg->mode != BUS_I2C || cfg->mode != BUS_SPI) {
        return -2;
    }
    if ((cfg->clock_divider == 0) || (cfg->clock_divider > 255)) {
        return -3;
    }

    uint32_t reg_val = 0;

    if (cfg->mode == BUS_I2C) {
        if (cfg->spi_cpol || cfg->spi_cpha || (cfpg->chip_select != 0)) {
            return -2;
        }

        reg_val = (cfg->i2c_10bit_addr) ? 
            (reg_val | BUS_CTRL_I2C_10BIT) : (reg_val);
        
    } else {
        if ((cfg->chip_select > BUS_CTRL_CS_MASK >> BUS_CTRL_CS_SHIFT) ||
            (cfg->i2c_10bit_addr)){
            return -2;
        }

        // Set SPI bits
        reg_val |= BUS_CTRL_MODE_SPI;
        reg_val |= ((cfg->chip_select) << BUS_CTRL_CS_SHIFT);
        reg_val = (cfg->spi_cpol) ? (reg_val | BUS_CTRL_SPI_CPOL) :
            *reg_val;
        reg_val = (cfg->spi_cpha) ? (reg_val | BUS_CTRL_SPI_CPHA) :
            *reg_val;
    }

    reg_val = (cfg->enable) ? (reg_val | BUS_CTRL_ENABLE) :
        *reg_val;
    // Set clock divider
    reg_val &= ~(BUS_CTRL_CLKDIV_MASK);
    reg_val |= (cfg->clock_divider << BUS_CTRL_CLKDIV_SHIFT);

    *out_reg = reg_val;

    return 0;
}
/*
 Follow-ups:
 
 - What SPI bugs happen if CPOL/CPHA are wrong?
    Answer: Data will be sampled incorrectly, leading to undefined behavior
 - Why does I2C not use chip select?
    Answer: I2C protocol does not have a CS line and instead relies on 
            transmitting an target address.
 - What would you check with a logic analyzer for each protocol?
    Answer: For I2C, checking the start/stop conditions are correct. 
            Address selection is correct with appropriate NACK/ACKS
            For SPI, make sure the mode is as expected. Check for 
            appropriate commands and responses from slave.
 - Should this helper write MMIO directly, or just build a value for another layer to write?
    Answer: It doesn't look like this correct layer to interact with HW but 
            instead the layer above to parse info before atomically interacting with hw.
 - What changes if the hardware requires disabling the bus before changing mode?
    Answer: We should add an enum state, so that we can perform this "reset/disable"
            and handle that configuration here.
*/
