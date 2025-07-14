#include "rfm9Xw_reg.h"
#include <zephyr/drivers/spi.h>

static const char *reg_names[0x71] = {
    [REG_FIFO] = "RegFifo",
    [REG_OP_MODE] = "RegOpMode",
    [REG_BITRATE_MSB] = "RegBitrateMsb",
    [REG_BITRATE_LSB] = "RegBitrateLsb",
    [REG_FDEV_MSB] = "RegFdevMsb",
    [REG_FDEV_LSB] = "RegFdevLsb",
    [REG_FRF_MSB] = "RegFrfMsb",
    [REG_FRF_MID] = "RegFrfMid",
    [REG_FRF_LSB] = "RegFrfLsb",
    [REG_PA_CONFIG] = "RegPaConfig",
    [REG_PA_RAMP] = "RegPaRamp",
    [REG_OCP] = "RegOcp",
    [REG_LNA] = "RegLna",
    [REG_RX_CONFIG] = "RegRxConfig",
    [REG_RSSI_CONFIG] = "RegRssiConfig",
    [REG_RSSI_COLLISION] = "RegRssiCollision",
    [REG_RSSI_THRESH] = "RegRssiThresh",
    [REG_RSSI_VALUE] = "RegRssiValue",
    [REG_RX_BW] = "RegRxBw",
    [REG_AFC_BW] = "RegAfcBw",
    [REG_OOK_PEAK] = "RegOokPeak",
    [REG_OOK_FIX] = "RegOokFix",
    [REG_OOK_AVG] = "RegOokAvg",
    [REG_AFC_FEI] = "RegAfcFei",
    [REG_AFC_MSB] = "RegAfcMsb",
    [REG_AFC_LSB] = "RegAfcLsb",
    [REG_FEI_MSB] = "RegFeiMsb",
    [REG_FEI_LSB] = "RegFeiLsb",
    [REG_PREAMBLE_DETECT] = "RegPreambleDetect",
    [REG_RX_TIMEOUT1] = "RegRxTimeout1",
    [REG_RX_TIMEOUT2] = "RegRxTimeout2",
    [REG_RX_TIMEOUT3] = "RegRxTimeout3",
    [REG_RX_DELAY] = "RegRxDelay",
    [REG_OSC] = "RegOsc",
    [REG_PREAMBLE_MSB] = "RegPreambleMsb",
    [REG_PREAMBLE_LSB] = "RegPreambleLsb",
    [REG_PACKET_CONFIG1] = "RegPacketConfig1",
    [REG_PACKET_CONFIG2] = "RegPacketConfig2",
    [REG_PAYLOAD_LENGTH] = "RegPayloadLength",
    [REG_NODE_ADRS] = "RegNodeAdrs",
    [REG_BROADCAST_ADRS] = "RegBroadcastAdrs",
    [REG_FIFO_THRESH] = "RegFifoThresh",
    [REG_SEQ_CONFIG1] = "RegSeqConfig1",
    [REG_SEQ_CONFIG2] = "RegSeqConfig2",
    [REG_TIMER_RESOL] = "RegTimerResol",
    [REG_TIMER1_COEF] = "RegTimer1Coef",
    [REG_TIMER2_COEF] = "RegTimer2Coef",
    [REG_IMAGE_CAL] = "RegImageCal",
    [REG_TEMP] = "RegTemp",
    [REG_LOW_BAT] = "RegLowBat",
    [REG_IRQ_FLAGS1] = "RegIrqFlags1",
    [REG_IRQ_FLAGS2] = "RegIrqFlags2",
    [REG_DIO_MAPPING1] = "RegDioMapping1",
    [REG_DIO_MAPPING2] = "RegDioMapping2",
    [REG_VERSION] = "RegVersion",
    [REG_PLL_HOP] = "RegPllHop",
    [REG_TCXO] = "RegTcxo",
    [REG_PADAC] = "RegPadac",
    [REG_FORMERTEMP] = "RegFormertemp",
    [REG_BITRATEFRAC] = "RegBitratefrac",
    [REG_BITAGCREF] = "RegBitagcref",
    [REG_AGCTHRESH1] = "RegAgcthresh1",
    [REG_AGCTHRESH2] = "RegAgcthresh2",
    [REG_AGCTHRESH3] = "RegAgcthresh3",
    [REG_PLL] = "RegPll",
};

const char *rfm9Xw_fsk_reg_to_string(uint8_t reg) {
    if (reg > 0x71) {
        return "UNKNOWN REG";
    }
    return reg_names[reg];
}

int rfm9Xw_read_reg(const struct spi_dt_spec *bus, const uint8_t reg, uint8_t *const result) {
    // Write Data
    uint8_t cmd[2] = {reg & RFM_SPI_REG_MASK, 0};
    struct spi_buf txbuf = {
        .buf = &cmd,
        .len = 2,
    };
    struct spi_buf_set txbufset = {
        .buffers = &txbuf,
        .count = 1,
    };

    uint8_t reg8[2] = {0xbe, 0xad};
    struct spi_buf rxbuf = {
        .buf = &reg8,
        .len = 2,
    };
    struct spi_buf_set rxbufset = {
        .buffers = &rxbuf,
        .count = 1,
    };
    int res = spi_transceive_dt(bus, &txbufset, &rxbufset);
    *result = reg8[1]; // reg8[0] is just the command we wrote

    return res;
}

int rfm9Xw_write_reg_burst(const struct spi_dt_spec *bus, uint8_t reg, uint8_t *data, int32_t data_len) {
    // Write Data
    uint8_t write_and_reg = RFM_SPI_WRITE_BIT | (reg & RFM_SPI_REG_MASK);

    struct spi_buf write_and_reg_buf = {
        .buf = &write_and_reg,
        .len = 1,
    };
    struct spi_buf data_buf = {
        .buf = data,
        .len = data_len,
    };

    struct spi_buf bufs[2] = {write_and_reg_buf, data_buf};

    struct spi_buf_set set = {
        .buffers = bufs,
        .count = 2,
    };
    
    return spi_write_dt(bus, &set);
}

int rfm9Xw_write_reg(const struct spi_dt_spec *bus, const uint8_t reg, uint8_t data) {
    return rfm9Xw_write_reg_burst(bus, reg, &data, 1);
}
