#include <zephyr/settings/settings.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/reboot.h>

bool settings_modified = false;

static int gain_cmd_handler(const struct shell *sh, size_t argc, char **argv, void *data) {
    int gain;

    /* data is a value corresponding to called command syntax */
    gain = (int) data;
    // adc_set_gain(gain);

    shell_print(sh,
                "ADC gain set to: %s\n"
                "Value send to ADC driver: %d",
                argv[0], gain);

    return 0;
}

static int cmd_demo_ping(const struct shell *sh, size_t argc, char **argv, void *data) {
    shell_print(sh, "Pong");
    return 0;
}

SHELL_SUBCMD_DICT_SET_CREATE(sub_gain, gain_cmd_handler, (gain_1, 1, "gain 1"), (gain_2, 2, "gain 2"),
                             (gain_1_2, 3, "gain 1/2"), (gain_1_4, 4, "gain 1/4"));

SHELL_STATIC_SUBCMD_SET_CREATE(sub_demo, SHELL_CMD(ping, NULL, "Ping command.", cmd_demo_ping),
                               SHELL_CMD(gain, &sub_gain, "Gain command.", NULL),

                               SHELL_SUBCMD_SET_END);
//
// SHELL_CMD_REGISTER(gain, &sub_gain, "Set ADC gain", NULL);
SHELL_CMD_REGISTER(demo, &sub_demo, "Demo commands", NULL);
