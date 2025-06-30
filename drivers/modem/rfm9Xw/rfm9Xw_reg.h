#ifndef RFM9XW_REG_H_
#define RFM9XW_REG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <zephyr/drivers/spi.h>

#define REG_FIFO            0x0
#define REG_OP_MODE         0x1
#define REG_BITRATE_MSB     0x2
#define REG_BITRATE_LSB     0x3
#define REG_FDEV_MSB        0x4
#define REG_FDEV_LSB        0x5
#define REG_FRF_MSB         0x6
#define REG_FRF_MID         0x7
#define REG_FRF_LSB         0x8
#define REG_PA_CONFIG       0x9
#define REG_PA_RAMP         0xa
#define REG_OCP             0xb
#define REG_LNA             0xc
#define REG_RX_CONFIG       0xd
#define REG_RSSI_CONFIG     0xe
#define REG_RSSI_COLLISION  0xf
#define REG_RSSI_THRESH     0x10
#define REG_RSSI_VALUE      0x11
#define REG_RX_BW           0x12
#define REG_AFC_BW          0x13
#define REG_OOK_PEAK        0x14
#define REG_OOK_FIX         0x15
#define REG_OOK_AVG         0x16
#define REG_AFC_FEI         0x1a
#define REG_AFC_MSB         0x1b
#define REG_AFC_LSB         0x1c
#define REG_FEI_MSB         0x1d
#define REG_FEI_LSB         0x1e
#define REG_PREAMBLE_DETECT 0x1f
#define REG_RX_TIMEOUT1     0x20
#define REG_RX_TIMEOUT2     0x21
#define REG_RX_TIMEOUT3     0x22
#define REG_RX_DELAY        0x23
#define REG_OSC             0x24
#define REG_PREAMBLE_MSB    0x25
#define REG_PREAMBLE_LSB    0x26
#define REG_PACKET_CONFIG1  0x30
#define REG_PACKET_CONFIG2  0x31
#define REG_PAYLOAD_LENGTH  0x32
#define REG_NODE_ADRS       0x33
#define REG_BROADCAST_ADRS  0x34
#define REG_FIFO_THRESH     0x35
#define REG_SEQ_CONFIG1     0x36
#define REG_SEQ_CONFIG2     0x37
#define REG_TIMER_RESOL     0x38
#define REG_TIMER1_COEF     0x39
#define REG_TIMER2_COEF     0x3a
#define REG_IMAGE_CAL       0x3b
#define REG_TEMP            0x3c
#define REG_LOW_BAT         0x3d
#define REG_IRQ_FLAGS1      0x3e
#define REG_IRQ_FLAGS2      0x3f
#define REG_DIO_MAPPING1    0x40
#define REG_DIO_MAPPING2    0x41
#define REG_VERSION         0x42
#define REG_PLL_HOP         0x44
#define REG_TCXO            0x4b
#define REG_PADAC           0x4d
#define REG_FORMERTEMP      0x5b
#define REG_BITRATEFRAC     0x5d
#define REG_BITAGCREF       0x61
#define REG_AGCTHRESH1      0x62
#define REG_AGCTHRESH2      0x63
#define REG_AGCTHRESH3      0x64
#define REG_PLL             0x70

#define RFM_SPI_WRITE_BIT 0b10000000
#define RFM_SPI_REG_MASK  0b01111111

#define RFM_PA_CONFIG_MASK_PA_SELECT    0x80
#define RFM_PA_CONFIG_MASK_MAX_POWER    0x70
#define RFM_PA_CONFIG_MASK_OUTPUT_POWER 0x0f;
#define RFM_MAX_OUTPUT_POWER            0x0f

#define RFM_REG_PA_RAMP_MASK_MODULATION_SHAPING 0b01100000
#define RFM_REG_PA_RAMP_MASK_PA_RAMP            0b00001111

#define RFM_MAX_NUM_DIOS 6

#define REG_OP_MODE_LONG_RANGE_MODE_MASK (0b10000000)
#define REG_OP_MODE_MODULATION_TYPE_MASK (0b01100000)
#define REG_OP_MODE_LOW_FREQ_MODE_MASK   (0b00001000)
#define REG_OP_MODE_TRANS_MODE_MASK      (0b00000111)

#define REG_FDEV_MSB_MAX (0b00111111)

#define REG_PACKET_CONFIG2_DATA_MODE_MASK (0b01000000)

// Frequency synthesizer step FSTEP = FXOSC/(2^19)
#define FXOSC_HZ                       32000000
#define RFM_FSTEP_HZ                   61.03515625
#define RFM_MAX_FREQUENCY_DEVIATION_HZ 999879

#define RFM_BIT_RATE_FSK_BPS_MIN 1200
#define RFM_BIT_RATE_FSK_BPS_MAX 300000
#define RFM_BIT_RATE_OOK_BPS_MIN 1200
#define RFM_BIT_RATE_OOK_BPS_MAX 32768

#define RFM_FREQUENCY_DEVIATION_MIN 600
#define RFM_FREQUENCY_DEVIATION_MAX 200000

#define RFM99_SYNTH_MIN_HZ 137000000
#define RFM99_SYNTH_MAX_HZ 175000000

#define RFM98_SYNTH_MIN_HZ 410000000
#define RFM98_SYNTH_MAX_HZ 525000000

// // Same as 98W
#define RFM96_SYNTH_MIN_HZ RFM98_SYNTH_MIN_HZ
#define RFM96_SYNTH_MAX_HZ RFM98_SYNTH_MAX_HZ

#define RFM95_SYNTH_MIN_HZ 862000000
#define RFM95_SYNTH_MAX_HZ 1020000000

const char *rfm9Xw_fsk_reg_to_string(uint8_t reg);

int rfm9Xw_read_reg(const struct spi_dt_spec *bus, const uint8_t reg, uint8_t *const result);
int rfm9Xw_write_reg_burst(const struct spi_dt_spec *bus, uint8_t reg, uint8_t *data, int32_t data_len);
int rfm9Xw_write_reg(const struct spi_dt_spec *bus, const uint8_t reg, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif