#ifndef BOARD_GRIM_REEFER
#define USE_SIM_DATA
#endif

#define LOGIC_VOLTAGE_LOW_MILLIVOLTS 3500
#define LOGIC_VOLTAGE_LOW_MILLIVOLTS 3500

#define BOOST_TO_NOSEOVER_TIMER_SECONDS 30
#define NOSEOVE_TO_MAIN_TIMER_SECONDS 30
#define MAIN_TO_GROUND_TIMER_SECONDS 30

// Data collection thread info

// Phase -1 - After startup, before launch detecting
#define CANCEL_FLIGHT_TIMEOUT_SECONDS 15

// Phase 0 - Launch Detection
#define LAUNCH_DETECT_PHASE_IMU_ALT_SAMPLE_PERIOD_MS 1

// Phase 1 - Boost

// 1 ms
#define BOOST_PHASE_IMU_ALT_SAMPLE_PERIOD_MS 1
#define BOOST_PHASE_LOAD_CELL_SAMPLE_PERIOD_US 1000
#define BOOST_PHASE_TEMP_VOLTAGE_SAMPLE_PERIOD_MS 1000

// Phase 2 - Reef Events
#define REEF_PHASE_IMU_ALT_SAMPLE_PERIOD_MS 1
#define REEF_PHASE_LOAD_CELL_SAMPLE_PERIOD_US 100
#define REEF_PHASE_TEMP_VOLTAGE_SAMPLE_PERIOD_MS 1000

// Phase 3 - Under Main
#define MAIN_PHASE_IMU_ALT_SAMPLE_PERIOD_MS 1
#define MAIN_PHASE_LOAD_CELL_SAMPLE_PERIOD_US 1000
#define MAIN_PHASE_TEMP_VOLTAGE_SAMPLE_PERIOD_MS 1000

// Data locations

#define PREDETECT_FILEPATH "/lfs/predetect.bin"
#define DATA_SLOW_FILEPATH "/lfs/slow.bin"
#define DATA_FAST_FILEPATH "/lfs/fast.bin"
#define DATA_ADC_FILEPATH "/lfs/adc.bin"

// Thread stuff
#define DATA_STORAGE_STACK_SIZE (4096 * 4)
#define DATA_STORAGE_PRIORITY 5

#define ADC_STACK_SIZE 500
#define ADC_PRIORITY 5

#define SLOW_STACK_SIZE 500
#define SLOW_PRIORITY 5
