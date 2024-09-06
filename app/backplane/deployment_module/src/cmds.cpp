#include <zephyr/settings/settings.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/reboot.h>

bool settings_modified = false;

static int noseover_detect_mix_handler(const struct shell *sh, size_t argc, char **argv, void *data) {
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

static int boost_detect_mix_handler(const struct shell *sh, size_t argc, char **argv, void *data) {
    int gain;

    /* data is a value corresponding to called command syntax */
    gain = (int) data;
    // adc_set_gain(gain);

    shell_print(sh,
                "Boost mix: %s\n"
                "Value sent: %d",
                argv[0], gain);

    return 0;
}

static int cmd_noseover_vel_handler(const struct shell *sh, size_t argc, char **argv, void *data) {
    shell_print(sh, "Me when i uhhhhhh %s", argv[0]);
    return 0;
}

int shell_get_char(const struct shell *sh, char *outc) {
    size_t cnt = 0;
    char c = 1;
    int err = sh->iface->api->read(sh->iface, &c, sizeof(c), &cnt);
    if (err != 0) {
        return err;
    }
    while (cnt == 0) {
        k_msleep(1);
        sh->iface->api->read(sh->iface, &c, sizeof(c), &cnt);
        if (err != 0) {
            return err;
        }
    }
    *outc = c;
    return 0;
}

bool validate_number(const struct shell *sh, char *number) {
    // convert to numbers

    for (int i = 0; i < 16; i++) {
        char c = number[i];
        if (c < '0' || c > '9') {
            shell_print(sh, "Invalid character '%c' at position %d", c, i);
            return false;
        }
        number[i] = c - '0';
    }
    // last_digit, remaining_numbers = self.last_digit_and_remaining_numbers()
    // nums = [int(num) if idx % 2 != 0 else int(num) * 2 if int(num) * 2 <= 9
    // else int(num) * 2 % 10 + int(num) * 2 // 10
    // for idx, num in enumerate(reversed(remaining_numbers))]
    char last = number[15];
    int sum = 0;

    for (int i = 14; i >= 0; i--) {
        int idx = 14 - i;
        int num = number[i];

        int out = 0;
        if (idx % 2 != 0) {
            out = num;
        } else if (num * 2 <= 9) {
            out = num * 2;
        } else {
            out = num * 2 % 10 + (num * 2) / 10;
        }
        sum += out;
    }
    return (sum + (int) last) % 10 == 0;
}
static int deploy_handler(const struct shell *sh, size_t argc, char **argv, void *data) {
    int which = (int) data;
    char *which_str = argv[0];

    // y/n
    shell_fprintf(sh, SHELL_NORMAL, "Do you want to deploy %s? (y/n): ", which_str);
    char c = 0;
    int err = shell_get_char(sh, &c);
    if (err != 0) {
        shell_print(sh, "Couldn't read: %d", err);
        return err;
    }
    if (c != 'y') {
        shell_print(sh, "Not deploying");
        return 0;
    }
    // Credit card number
    shell_print(sh, "%c", c);

    shell_warn(sh, "Enter a 16 digit credit card number to make sure you didn't just sit on the keyboard\nNo spaces, "
                   "just numbers, no backspaces:");
    char buf[16] = {0};
    for (int i = 0; i < 16; i++) {
        int err = shell_get_char(sh, &buf[i]);
        if (err != 0) {
            shell_print(sh, "Couldn't read: %d", err);
            return err;
        }
        size_t cnt = 0;
        sh->iface->api->write(sh->iface, &buf[i], sizeof(char), &cnt);
    }
    if (!validate_number(sh, buf)) {
        shell_print(sh, "\nInvalid Card Number");
        return -1;
    }
    // y
    shell_print(sh, "\nNumber validated, you're a human");
    shell_warn(sh, "PRESS 'y' TO START");

    c = 0;
    err = shell_get_char(sh, &c);
    if (err != 0) {
        shell_print(sh, "Couldn't read: %d", err);
        return err;
    }
    if (c != 'y') {
        return 0;
    }

    shell_error(sh, "DEPLOYING %s. GOOD LUCK", which_str);
    return 0;
}

// clang-format off

SHELL_SUBCMD_DICT_SET_CREATE(sub_noseover_detect_mix, noseover_detect_mix_handler, 
                            (Or, 1, "or"), 
                            (And, 2, "and")
                            );

SHELL_SUBCMD_DICT_SET_CREATE(sub_deploy, deploy_handler, 
                            (main, 1, "main"), 
                            (drogue, 2, "drogue")
                            );

SHELL_SUBCMD_DICT_SET_CREATE(sub_boost_detect_mix, boost_detect_mix_handler, 
                            (accel, 1, "accelerometer"), 
                            (altim, 2, "altimeter"),
                            (both, 3, "both"),
                            (either, 4, "either")
                            );

SHELL_STATIC_SUBCMD_SET_CREATE(sub_noseover, 
                                SHELL_CMD(velocity, NULL, "What velocity do we decide noseover at. 0 for perfect noseover (+ delay from filtering). +X for X ft/s going upwards. -X for X ft/s going down", cmd_noseover_vel_handler),
                                SHELL_CMD(mix, &sub_noseover_detect_mix, "How to mix the two barometers' noseover detections (or, and)", NULL),
                               SHELL_SUBCMD_SET_END);


SHELL_STATIC_SUBCMD_SET_CREATE(sub_boost, 
                                SHELL_CMD(mix, &sub_boost_detect_mix, "How do we mix the boost detection methods  (accel, altim, both, either)", NULL),
                               SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_drogue, 
                                // SHELL_CMD(mix, &sub_boost_detect_mix, "How do we mix the boost detection methods  (accel, altim, both, either)", NULL),
                               SHELL_SUBCMD_SET_END);


SHELL_STATIC_SUBCMD_SET_CREATE(sub_demo, 
                                SHELL_CMD(boost, &sub_boost, "Parameters for boost detection", NULL),
                                SHELL_CMD(noseover, &sub_noseover, "Parameters for noseover detection. The main charge is activated at noseover", NULL),
                                SHELL_CMD(drogue, &sub_drogue, "Parameters for drogue activation", NULL),
                                SHELL_CMD(deploy, &sub_deploy, "Deploy main or drogue as a test", NULL),
                               SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(flight, &sub_demo, "Commands for deployment mod", NULL);
