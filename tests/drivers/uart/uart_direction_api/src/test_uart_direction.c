/*
 * Copyright (c) 2021 Baumer
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test_uart.h"

static volatile bool data_transmitted;
static volatile bool data_received;
static int char_sent;
static int char_received;

static const char *fifo_data = "This is a direction mode test.\r\n";

#define DATA_SIZE	(sizeof(fifo_data) - 1)

/* RX and TX pins have to be connected together*/
#if defined(CONFIG_BOARD_NUCLEO_G071RB)
#define UART_DEVICE DEVICE_DT_GET(DT_NODELABEL(usart1))
#else
#define UART_DEVICE DEVICE_DT_GET(DT_CHOSEN(zephyr_console))
#endif

static int tx_data_idx;

static void uart_fifo_callback(const struct device *dev, void *user_data)
{
	uint8_t recvData;


	ARG_UNUSED(user_data);

	if (!uart_irq_update(dev)) {
		TC_PRINT("retval should always be 1\n");
		return;
	}

	if (uart_irq_tx_ready(dev) && tx_data_idx < DATA_SIZE) {
		if (uart_fifo_fill(dev,
				   (uint8_t *)&fifo_data[tx_data_idx++], 1) > 0) {
			data_transmitted = true;
			char_sent++;
		}

		if (tx_data_idx == DATA_SIZE) {
			uart_irq_tx_disable(dev);
		}
	}

	if (uart_irq_rx_ready(dev)) {
		uart_fifo_read(dev, &recvData, 1);
		data_received = true;
		char_received++;
	}
}

static int test_rx(const struct device *uart_dev)
{
	data_received = false;
	char_received = 0;
	tx_data_idx = 0;

	uart_irq_rx_enable(uart_dev);
	uart_irq_tx_enable(uart_dev);

	k_sleep(K_MSEC(500));

	uart_irq_tx_disable(uart_dev);
	uart_irq_rx_disable(uart_dev);

	if (!data_received) {
		return TC_FAIL;
	}

	if (char_received == DATA_SIZE) {
		return TC_PASS;
	} else {
		return TC_FAIL;
	}
}

static int test_tx(const struct device *uart_dev)
{
	data_transmitted = false;
	char_sent = 0;
	tx_data_idx = 0;

	uart_irq_tx_enable(uart_dev);

	k_sleep(K_MSEC(500));

	uart_irq_tx_disable(uart_dev);

	if (!data_transmitted) {
		return TC_FAIL;
	}

	if (char_sent == DATA_SIZE) {
		return TC_PASS;
	} else {
		return TC_FAIL;
	}
}

static int test_direction_rx(void)
{
	const struct device *uart_dev = UART_DEVICE;

	if (!device_is_ready(uart_dev)) {
		TC_PRINT("UART device not ready\n");
		return TC_FAIL;
	}
	uart_irq_callback_set(uart_dev, uart_fifo_callback);

	uart_disable_rx_direction(uart_dev);
	zassert_true(test_rx(uart_dev) == TC_FAIL, NULL);

	uart_enable_rx_direction(uart_dev);
	zassert_true(test_rx(uart_dev) == TC_PASS, NULL);

	return TC_PASS;
}

static int test_direction_tx(void)
{
	const struct device *uart_dev = UART_DEVICE;

	if (!device_is_ready(uart_dev)) {
		TC_PRINT("UART device not ready\n");
		return TC_FAIL;
	}
	uart_irq_callback_set(uart_dev, uart_fifo_callback);

	uart_disable_tx_direction(uart_dev);
	zassert_true(test_tx(uart_dev) == TC_FAIL, NULL);

	uart_enable_tx_direction(uart_dev);
	zassert_true(test_tx(uart_dev) == TC_PASS, NULL);

	return TC_PASS;
}

void test_uart_direction_rx(void)
{
	zassert_true(test_direction_rx() == TC_PASS, NULL);
}

void test_uart_direction_tx(void)
{
	zassert_true(test_direction_tx() == TC_PASS, NULL);
}
