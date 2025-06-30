

#ifndef RFM9XW_H
#define RFM9XW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>

enum RfmModelNumber {
    RfmModelNumber_95W,
    RfmModelNumber_96W,
    RfmModelNumber_98W,
    RfmModelNumber_99W,
};

enum RfmModemMode {
    RfmModemMode_FSK,
    RfmModemMode_OOK,
    RfmModemMode_LoRa,
};

enum RfmLongRangeModeSetting {
    // Bit 7 of RegOpMode
    RfmLongRangeModeSetting_FskOokMode = 0b00000000,
    RfmLongRangeModeSetting_LoraTmMode = 0b10000000,
};

enum RfmModulationType {
    // Bits 5-6 of RegOpMode
    // Built in Modulation Types (register values)
    RfmModulationType_FSK = 0b00000000,
    RfmModulationType_OOK = 0b00100000,
};

enum RfmLowFrequencyMode {
    // Bit 3 of RegOpMode
    RfmLowFrequencyMode_HighFrequency = 0b0000,
    RfmLowFrequencyMode_LowFrequency = 0b1000,
};
enum RfmTransceiverMode {
    // Bits 0-2 of RegOpMode
    RfmTransceiverMode_Sleep = 0b000,
    RfmTransceiverMode_Standby = 0b001,
    RfmTransceiverMode_FsModeTx = 0b010,    //(FSTx)
    RfmTransceiverMode_Transmitter = 0b011, //(Tx)
    RfmTransceiverMode_FsModeRx = 0b100,    //(FSRx)
    RfmTransceiverMode_Receiver = 0b101,    //(Rx)
};

enum RfmPacketConfigDataMode {
    // Bit 6 of RegPacketConfig2
    RfmPacketConfigDataMode_Continuous = 0b00000000,
    RfmPacketConfigDataMode_Packet = 0b01000000, // Default
};
enum RfmPowerAmplifierSelection {

    // bit 7 of RegPaConfig
    RfmPowerAmplifierSelection_RFO = 0b00000000,
    RfmPowerAmplifierSelection_PaBoost = 0b10000000,
};
enum RfmModulationShaping {
    RfmModulationShaping_FSK_NoShaping = 0b00000000, // Default
    RfmModulationShaping_FSK_GaussianBT_1_0 = 0b00100000,
    RfmModulationShaping_FSK_GaussianBT_0_5 = 0b01000000,
    RfmModulationShaping_FSK_GaussianBT_0_3 = 0b01100000,

    RfmModulationShaping_OOK_NoShaping = 0b00000000,
    RfmModulationShaping_OOK_FCutoffBitRate = 0b00100000,
    RfmModulationShaping_OOK_FCutoff2xBitRate = 0b01000000,
};

enum RfmPaRamp {
    RfmPaRamp_3400us = 0b0000,
    RfmPaRamp_2000us = 0b0001,
    RfmPaRamp_1000us = 0b0010,
    RfmPaRamp_500us = 0b0011,
    RfmPaRamp_250us = 0b0100,
    RfmPaRamp_125us = 0b0101,
    RfmPaRamp_100us = 0b0110,
    RfmPaRamp_62us = 0b0111,
    RfmPaRamp_50us = 0b1000,
    RfmPaRamp_40us = 0b1001, // Default
    RfmPaRamp_31us = 0b1010,
    RfmPaRamp_25us = 0b1011,
    RfmPaRamp_20us = 0b1100,
    RfmPaRamp_15us = 0b1101,
    RfmPaRamp_12us = 0b1110,
    RfmPaRamp_10us = 0b1111,
};

#define RFM_REG_DIO_MAPPING1_MASK_DIO0 0b11000000
enum RfmDio0Mapping {
    //Datasheet: Page 65, Table 28,29
    RfmDio0Mapping_Continuous_SyncAddressTxReady = 0b00000000, // Default
    RfmDio0Mapping_Continuous_RssiPreambleDetect = 0b01000000,
    RfmDio0Mapping_Continuous_RxReadyTxReady = 0b10000000,
    RfmDio0Mapping_Continuous_Nothing = 0b11000000,

    RfmDio0Mapping_Packet_PayloadReadyPacketSent = 0b00000000,
    RfmDio0Mapping_Packet_RxCrcOk = 0b01000000,
    RfmDio0Mapping_Packet_Nothing = 0b10000000,
    RfmDio0Mapping_Packet_TempChangeLowBat = 0b11000000,
};

#define RFM_REG_DIO_MAPPING1_MASK_DIO1 0b00110000
enum RfmDio1Mapping {
    //Datasheet: Page 65, Table 28,29
    RfmDio1Mapping_Continuous_Dclk = 0b000000, // Default
    RfmDio1Mapping_Continuous_RssiPreambleDetect = 0b010000,
    RfmDio1Mapping_Continuous_Nothing = 0b100000, // 0x03 is also nothing

    RfmDio1Mapping_Packet_FifoLevel = 0b000000,
    RfmDio1Mapping_Packet_FifoEmpty = 0b010000,
    RfmDio1Mapping_Packet_FifoFull = 0b100000,
    RfmDio1Mapping_Packet_Nothing = 0b110000,
};

#define RFM_REG_DIO_MAPPING1_MASK_DIO2 0b00001100
enum RfmDio2Mapping {
    //Datasheet: Page 65, Table 28,29
    // 00, 01, 10, 11 all are Data
    RfmDio2Mapping_Continuous_Data = 0b0000, // Default

    RfmDio2Mapping_Packet_FifoFull = 0b0000,
    RfmDio2Mapping_Packet_RxReady = 0b0100,
    RfmDio2Mapping_Packet_FifoFull_RxTimeout = 0b1000,
    RfmDio2Mapping_Packet_FifoFull_SyncAddress = 0b1100,

};

#define RFM_REG_DIO_MAPPING1_MASK_DIO3 0b00000011
enum RfmDio3Mapping {
    //Datasheet: Page 65, Table 28,29
    RfmDio3Mapping_Continuous_Timeout = 0b00, // Default
    RfmDio3Mapping_Continuous_RssiPreambleDetect = 0b01,
    RfmDio3Mapping_Continuous_Nothing = 0b10,
    RfmDio3Mapping_Continuous_TempChangeLowBat = 0b11,

    RfmDio3Mapping_Packet_FifoEmpty = 0b00,
    RfmDio3Mapping_Packet_TxReady = 0b01,
    // 0x2, 0x3 also FifoEmpty

};

#define RFM_REG_DIO_MAPPING2_MASK_DIO4 0b11000000
enum RfmDio4Mapping {
    //Datasheet: Page 65, Table 28,29
    RfmDio4Mapping_Continuous_TempChangeLowBat = 0b00000000, // Default
    RfmDio4Mapping_Continuous_PllLock = 0b01000000,
    RfmDio4Mapping_Continuous_Timeout = 0b10000000,
    RfmDio4Mapping_Continuous_ModeReady = 0b11000000,

    RfmDio4Mapping_Packet_TempChangeLowBat = 0b00000000,
    RfmDio4Mapping_Packet_PllLock = 0b01000000,
    RfmDio4Mapping_Packet_Timeout = 0b10000000,
    RfmDio4Mapping_Packet_RssiPreambleDetect = 0b11000000,
};
#define RFM_REG_DIO_MAPPING2_MASK_DIO5 0b00110000
enum RfmDio5Mapping {
    //Datasheet: Page 65, Table 28,29
    RfmDio5Mapping_Continuous_ClkOut = 0b000000, // Default
    RfmDio5Mapping_Continuous_PllLock = 0b010000,
    RfmDio5Mapping_Continuous_RssiPreambleDetect = 0b100000,
    RfmDio5Mapping_Continuous_ModeReady = 0b110000,

    RfmDio5Mapping_Packet_ClkOut = 0b000000,
    RfmDio5Mapping_Packet_PllLock = 0b010000,
    RfmDio5Mapping_Packet_Data = 0b100000,
    RfmDio5Mapping_Packet_ModeReady = 0b110000,
};

#define RFM_REG_DIO_MAPPING2_MASK_MAP_PREAMBLE_DETECT 0b00000001
enum RfmMapPreambleDetectInterrupt {
    RfmMapPreambleDetectInterrupt_RSSI = 0x0,
    RfmMapPreambleDetectInterrupt_PreambleDetect = 0x1,
};

enum RfmDcFreeEncodingType {
    RfmDcFreeEncodingType_None,
    RfmDcFreeEncodingType_Manchester,
    RfmDcFreeEncodingType_Whitening,
};

enum RfmDioEvent {
    RfmDioEvent_TempChange = BIT(0),
    RfmDioEvent_LowBat = BIT(1),
    RfmDioEvent_CrcOk = BIT(2),
    RfmDioEvent_PayloadReady = BIT(3),
    RfmDioEvent_PacketSent = BIT(4),
    RfmDioEvent_FifoLevel = BIT(5),
    RfmDioEvent_FifoFull = BIT(6),
    RfmDioEvent_FifoEmpty = BIT(7),
    RfmDioEvent_RxReady = BIT(8),
    RfmDioEvent_TxReady = BIT(9),
    RfmDioEvent_ModeReady = BIT(10),
    RfmDioEvent_SyncAddress = BIT(11),
    RfmDioEvent_PLLLock = BIT(12),
    RfmDioEvent_Timeout = BIT(13),
    RfmDioEvent_PreambleDetect = BIT(14),
    RfmDioEvent_RSSI = BIT(15),

    RfmDioEvent_Canceled = BIT(31), // Use this to wakeup active thread and release radio
};

struct rfm9Xw_fsk_modem_config {
    enum RfmPacketConfigDataMode data_mode;

    uint32_t carrier_freq;
    uint32_t deviation_freq;
    uint32_t bitrate;

    int16_t tx_power;

    enum RfmPaRamp pa_ramp;
    enum RfmModulationShaping modulation_shaping;

    uint8_t sync_word_len;
    uint64_t sync_word;
};

struct rfm9Xw_modem_config {
    enum RfmModemMode modem_mode;
    union {
        struct rfm9Xw_fsk_modem_config fsk;
    };
};

int rfm9Xw_configure_modem(const struct device *dev, struct rfm9Xw_modem_config *cfg);

/**
 * Read temperature in C
 * @param[in] dev rfm9xw device
 * @param[out] celsius temperature of radio in celsius
 * @return 0 on success.
 * @return -EAGAIN if mode is sleep or standby (temp measurement not available) 
 * @return other <0 on error from spi transmission
 */
int32_t rfm9xw_read_temperature(const struct device *dev, int8_t *celsius);

/**
 * Perform a 'Manual Reset' of the module 
 * (Datasheet pg.111 section 7.2.2)
 * @param[in] dev rfm9xw device
 */
int32_t rfm9xw_software_reset(const struct device *dev);

// int32_t rfm9xw_transmit(const struct device *dev, uint8_t buf, size_t buf_size, k_ticks_t timeout);
// int32_t rfm9xw_receive(const struct device *dev, uint8_t buf, size_t max_size, k_ticks_t timeout);

int32_t rfm9Xw_test_cw(const struct device *dev, uint32_t freq, int16_t power, k_timeout_t timeout);

#ifdef __cplusplus
}
#endif

#endif