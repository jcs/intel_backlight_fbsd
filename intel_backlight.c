/*
 * Copyright © 2011 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *	Chris Wilson <chris@chris-wilson.co.uk>
 *	Maurizio Vairani <maurizio.vairani@cloverinformatica.it>
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "intel_gpu_tools.h"
#define NUM_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

/* XXX PCH only today */

static uint32_t reg_read(uint32_t reg)
{
	return *(volatile uint32_t *)((volatile char*)mmio + reg);
}

static void reg_write(uint32_t reg, uint32_t val)
{
	*(volatile uint32_t *)((volatile char*)mmio + reg) = val;
}

static int brightness_levels[] = {1, 2, 4, 6, 9, 12, 16, 20, 25, 30, 36, 43,
				  51, 60, 70, 80, 90, 100};

static int brightness_incr(int curr)
{
	int i;
	for (i = 0; i < NUM_ELEMENTS(brightness_levels) - 1; ++i)
		if (curr < brightness_levels[i])
			break;
	return brightness_levels[i];
}

static int brightness_decr(int curr)
{
	int i;
	for (i = NUM_ELEMENTS(brightness_levels) - 1; i > 0; --i)
		if (brightness_levels[i] < curr)
			break;
	return brightness_levels[i];
}

int main(int argc, char** argv)
{
	uint32_t current, max, min;
	struct pci_device *dev;
	int result;

	dev = intel_get_pci_device();
	intel_get_mmio(dev);

	if (IS_GEN8(dev->device_id) || IS_GEN9(dev->device_id)) /* bdw/skl */
		current = reg_read(BLC_PWM_PCH_CTL2) & BACKLIGHT_DUTY_CYCLE_MASK;
	else
		current = reg_read(BLC_PWM_CPU_CTL) & BACKLIGHT_DUTY_CYCLE_MASK;

	max = reg_read(BLC_PWM_PCH_CTL2) >> 16;

	min = 0.5 + 0.5 * max / 100.0;	// 0.5%
	/*
	printf ("min: %d, NUM_ELEMENTS(brightness_levels): %d\n", min,
		NUM_ELEMENTS(brightness_levels));
	*/
	result = 0.5 + current * 100.0 / max;
	printf ("current backlight value: %d%% (%d/%d)\n", result, current, max);

	if (argc > 1) {
		uint32_t v;
		if (0 == strcmp(argv[1], "incr"))
			v = 0.5 + brightness_incr(result) * max / 100.0;
		else if (0 == strcmp(argv[1], "decr"))
			v = 0.5 + brightness_decr(result) * max / 100.0;
		else
			v = 0.5 + atoi (argv[1]) * max / 100.0;
		/*
		printf("v: %d\n", v);
		*/
		if (v < min)
			v = min;
		else if (v > max)
			v = max;

		if (IS_GEN8(dev->device_id) || IS_GEN9(dev->device_id)) {
			reg_write(BLC_PWM_PCH_CTL2,
				  (reg_read(BLC_PWM_PCH_CTL2) &~
				  BACKLIGHT_DUTY_CYCLE_MASK) | v);
			(void) reg_read(BLC_PWM_PCH_CTL2);
		} else {
			reg_write(BLC_PWM_CPU_CTL,
				  (reg_read(BLC_PWM_CPU_CTL) &~
				  BACKLIGHT_DUTY_CYCLE_MASK) | v);
			(void) reg_read(BLC_PWM_CPU_CTL);
		}
		result = 0.5 + v * 100.0 / max;
		printf ("set backlight to %d%% (%d/%d)\n", result, v, max);
	}

	return result;
}

