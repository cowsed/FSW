/*
 * Copyright (c) 2019 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <f_core/os/c_datalogger.h>
#include <f_core/util/debouncer.hpp>
#include <stdio.h>
#include <string.h>
#include <zephyr/ztest.h>

ZTEST(debouncer, test_assert) {
    using Timestamp = uint32_t;
    using Value = float;
    Value timeOver = 100; //ms
    Value valOver = 10.0; //meters or something

    Debuouncer<ThresholdDirection::Over, Timestamp, Value> db(timeOver, valOver);

    zassert_equal(db.passed(), false, "Have no data. shouldn't be passing");
    // first sample
    db.feed(0, 5.0);
    zassert_equal(db.passed(), false, "Have only one point. shouldn't be passing");

    // second sample
    db.feed(100, 15.0);
    zassert_equal(db.passed(), false, "2 points. 1 under, 1 over. shouldn't be passing");
    // second sample
    db.feed(150, 15.0);
    zassert_equal(db.passed(), false, "2 points. both over but less than time. shouldn't be passing");

    db.feed(201, 15.0);
    zassert_equal(db.passed(), true, "2 points. both over with enough time. should be passing");
}

ZTEST_SUITE(debouncer, NULL, NULL, NULL, NULL, NULL);