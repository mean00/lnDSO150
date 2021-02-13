/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2010 Perry Hung.
 * Copyright (c) 2012 LeafLabs, LLC.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *****************************************************************************/

/**
 * @file libmaple/i2c.c
 * @author Perry Hung <perry@leaflabs.com>
 * @author Barry Carter <barry.carter@gmail.com>
 * @brief Inter-Integrated Circuit (I2C) support.
 *
 * Master and Slave supported
 * I2C slave support added 2012 by Barry Carter. barry.carter@gmail.com, headfuzz.co.uk
 * 
 * Modified 2019 by Donna Whisnant to merge WireSlave changes with the core to
 * make slave mode work and without having conflicting data type defintions,
 * and rewrote logic to function better.
 * 
 */

#include <libmaple/i2c_common.h>
#include "i2c_private.h"

#include <libmaple/libmaple.h>
#include <libmaple/rcc.h>
#include <libmaple/gpio.h>
#include <libmaple/nvic.h>
#include <libmaple/i2c.h>
#include <libmaple/systick.h>

#include <string.h>
// This disables USB altogether
void __irq_usb_lp_can_rx0()
{
}

