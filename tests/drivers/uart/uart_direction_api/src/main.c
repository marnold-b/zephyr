/*
 * Copyright (c) 2021 Baumer
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @addtogroup t_driver_uart
 * @{
 * @defgroup t_uart_direction test_uart_direction_operations
 * @}
 */

#include "test_uart.h"
void test_main(void)
{
	ztest_test_suite(uart_direction_test,
			 ztest_unit_test(test_uart_direction_rx),
			 ztest_unit_test(test_uart_direction_tx));
	ztest_run_test_suite(uart_direction_test);
}
