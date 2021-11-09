/*
 * Copyright (c) 2021 Baumer
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief UART cases header file
 *
 * Header file for UART cases
 */

#ifndef __TEST_UART_H__
#define __TEST_UART_H__

#include <drivers/uart.h>
#include <ztest.h>

void test_uart_direction_rx(void);
void test_uart_direction_tx(void);

#endif /* __TEST_UART_H__ */
