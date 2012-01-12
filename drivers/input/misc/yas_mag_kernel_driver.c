/*
 * Copyright (c) 2010 Yamaha Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include <asm/atomic.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define __LINUX_KERNEL_DRIVER__
#include "yas.h"
#include "yas_mag_driver.c"

#define GEOMAGNETIC_I2C_DEVICE_NAME         "yas529"
#define GEOMAGNETIC_INPUT_NAME              "magnetic_sensor"
#define GEOMAGNETIC_INPUT_RAW_NAME          "raw_magnetic_sensor"
#undef GEOMAGNETIC_PLATFORM_API

#define REL_STATUS			(REL_RX)
#define REL_WAKE			(REL_RY)

#define REL_RAW_DISTORTION		(REL_HWHEEL)
#define REL_RAW_THRESHOLD		(REL_DIAL)
#define REL_RAW_SHAPE			(REL_WHEEL)
#define REL_RAW_REPORT			(REL_MISC)

struct geomagnetic_data {
    struct input_dev *input_data;
    struct input_dev *input_raw;
    struct delayed_work work;
    struct semaphore driver_lock;
    struct semaphore multi_lock;
    atomic_t last_data[3];
    atomic_t last_status;
    atomic_t enable;
    int filter_enable;
    int filter_len;
    int32_t filter_noise[3];
    int32_t filter_threshold;
    int delay;
    int32_t threshold;
    int32_t distortion[3];
    int32_t shape;
    struct yas_mag_offset driver_offset;
#if DEBUG
    int suspend;
#endif
#ifdef YAS_MAG_MANUAL_OFFSET
    struct yas_vector manual_offset;
#endif
};

extern unsigned int HWREV;
static struct i2c_client *this_client = NULL;

static int
geomagnetic_i2c_open(void)
{
    return 0;
}

static int
geomagnetic_i2c_close(void)
{
    return 0;
}

//#if YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS529
static int
geomagnetic_i2c_write(const uint8_t *buf, int len)
{
    if (i2c_master_send(this_client, buf, len) < 0) {
        return -1;
    }
#if DEBUG
    YLOGD(("[W] [%02x]\n", buf[0]));
#endif

    return 0;
}

static int
geomagnetic_i2c_read(uint8_t *buf, int len)
{
    if (i2c_master_recv(this_client, buf, len) < 0) {
        return -1;
    }

#if DEBUG
    if (len == 1) {
        YLOGD(("[R] [%02x]\n", buf[0]));
    }
    else if (len == 6) {
        YLOGD(("[R] "
        "[%02x%02x%02x%02x%02x%02x]\n",
        buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]));
    }
    else if (len == 8) {
        YLOGD(("[R] "
        "[%02x%02x%02x%02x%02x%02x%02x%02x]\n",
        buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]));
    }
    else if (len == 9) {
        YLOGD(("[R] "
        "[%02x%02x%02x%02x%02x%02x%02x%02x%02x]\n",
        buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8]));
    }
    else if (len == 16) {
        YLOGD(("[R] "
        "[%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]\n",
        buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
        buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]));
    }
#endif

    return 0;
}

//#else

static int
geomagnetic_i2c_write_yas530(uint8_t addr, const uint8_t *buf, int len)
{
    uint8_t tmp[16];

    if (sizeof(tmp) -1 < len) {
        return -1;
    }

    tmp[0] = addr;
    memcpy(&tmp[1], buf, len);

    if (i2c_master_send(this_client, tmp, len + 1) < 0) {
        return -1;
    }
#if DEBUG
    YLOGD(("[W] addr[%02x] [%02x]\n", addr, buf[0]));
#endif

    return 0;
}

static int
geomagnetic_i2c_read_yas530(uint8_t addr, uint8_t *buf, int len)
{
    struct i2c_msg msg[2];
    int err;

    msg[0].addr = this_client->addr;
    msg[0].flags = 0;
    msg[0].len = 1;
    msg[0].buf = &addr;
    msg[1].addr = this_client->addr;
    msg[1].flags = I2C_M_RD;
    msg[1].len = len;
    msg[1].buf = buf;

    err = i2c_transfer(this_client->adapter, msg, 2);
    if (err != 2) {
        dev_err(&this_client->dev,
                "i2c_transfer() read error: slave_addr=%02x, reg_addr=%02x, err=%d\n", this_client->addr, addr, err);
        return err;
    }


#if DEBUG
    if (len == 1) {
        YLOGD(("[R] addr[%02x] [%02x]\n", addr, buf[0]));
    }
    else if (len == 6) {
        YLOGD(("[R] addr[%02x] "
        "[%02x%02x%02x%02x%02x%02x]\n",
        addr, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]));
    }
    else if (len == 8) {
        YLOGD(("[R] addr[%02x] "
        "[%02x%02x%02x%02x%02x%02x%02x%02x]\n",
        addr, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]));
    }
    else if (len == 9) {
        YLOGD(("[R] addr[%02x] "
        "[%02x%02x%02x%02x%02x%02x%02x%02x%02x]\n",
        addr, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8]));
    }
    else if (len == 16) {
        YLOGD(("[R] addr[%02x] "
        "[%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]\n",
        addr,
        buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
        buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]));
    }
#endif

    return 0;
}

//#endif

static int
geomagnetic_lock(void)
{
    struct geomagnetic_data *data = NULL;
    int rt;

    if (this_client == NULL) {
        return -1;
    }

    data = i2c_get_clientdata(this_client);
    rt = down_interruptible(&data->driver_lock);
    if (rt < 0) {
        up(&data->driver_lock);
    }
    return rt;
}

static int
geomagnetic_unlock(void)
{
    struct geomagnetic_data *data = NULL;

    if (this_client == NULL) {
        return -1;
    }

    data = i2c_get_clientdata(this_client);
    up(&data->driver_lock);
    return 0;
}

static void
geomagnetic_msleep(int ms)
{
    msleep(ms);
}

static void
geomagnetic_current_time(int32_t *sec, int32_t *msec)
{
    struct timeval tv;

    do_gettimeofday(&tv);

    *sec = tv.tv_sec;
    *msec = tv.tv_usec / 1000;
}

static struct yas_mag_driver hwdep_driver = {
    .callback = {
        .lock           = geomagnetic_lock,
        .unlock         = geomagnetic_unlock,
        .device_open    = geomagnetic_i2c_open,
        .device_close   = geomagnetic_i2c_close,
        .device_read    = geomagnetic_i2c_read,
        .device_write   = geomagnetic_i2c_write,
        .device_read_yas530    = geomagnetic_i2c_read_yas530,
        .device_write_yas530   = geomagnetic_i2c_write_yas530,
        .msleep         = geomagnetic_msleep,
        .current_time   = geomagnetic_current_time,
    },
};

static int
geomagnetic_multi_lock(void)
{
    struct geomagnetic_data *data = NULL;
    int rt;

    if (this_client == NULL) {
        return -1;
    }

    data = i2c_get_clientdata(this_client);
    rt = down_interruptible(&data->multi_lock);
    if (rt < 0) {
        up(&data->multi_lock);
    }
    return rt;
}

static int
geomagnetic_multi_unlock(void)
{
    struct geomagnetic_data *data = NULL;

    if (this_client == NULL) {
        return -1;
    }

    data = i2c_get_clientdata(this_client);
    up(&data->multi_lock);
    return 0;
}

static int
geomagnetic_enable(struct geomagnetic_data *data)
{
    if (!atomic_cmpxchg(&data->enable, 0, 1)) {
        schedule_delayed_work(&data->work, 0);
    }

    return 0;
}

static int
geomagnetic_disable(struct geomagnetic_data *data)
{
    if (atomic_cmpxchg(&data->enable, 1, 0)) {
        cancel_delayed_work_sync(&data->work);
    }

    return 0;
}

/* Sysfs interface */
static ssize_t
geomagnetic_delay_show(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct input_dev *input_data = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_data);
    int delay;

    geomagnetic_multi_lock();

    delay = data->delay;

    geomagnetic_multi_unlock();

    return sprintf(buf, "%d\n", delay);
}

static ssize_t
geomagnetic_delay_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf,
        size_t count)
{
    struct input_dev *input_data = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_data);
    int value = simple_strtol(buf, NULL, 10);

    if (hwdep_driver.set_delay == NULL) {
        return -ENOTTY;
    }

    geomagnetic_multi_lock();

    value = simple_strtol(buf, NULL, 10);
    if (hwdep_driver.set_delay(value) == 0) {
        data->delay = value;
    }

    geomagnetic_multi_unlock();

    return count;
}

static ssize_t
geomagnetic_enable_show(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct input_dev *input_data = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_data);

    return sprintf(buf, "%d\n", atomic_read(&data->enable));
}

static ssize_t
geomagnetic_enable_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf,
        size_t count)
{
    struct input_dev *input_data = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_data);
    int value;

    value = !!simple_strtol(buf, NULL, 10);
    if (hwdep_driver.set_enable == NULL) {
        return -ENOTTY;
    }

    if (geomagnetic_multi_lock() < 0) {
        return count;
    }

    if (hwdep_driver.set_enable(value) == 0) {
        if (value) {
            geomagnetic_enable(data);
        }
        else {
            geomagnetic_disable(data);
        }
    }

    geomagnetic_multi_unlock();

    return count;
}

static ssize_t
geomagnetic_filter_enable_show(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct input_dev *input_data = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_data);
    int filter_enable;

    geomagnetic_multi_lock();

    filter_enable = data->filter_enable;

    geomagnetic_multi_unlock();

    return sprintf(buf, "%d\n", filter_enable);
}

static ssize_t
geomagnetic_filter_enable_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf,
        size_t count)
{
    struct input_dev *input_data = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_data);
    int value;

    if (hwdep_driver.set_filter_enable == NULL) {
        return -ENOTTY;
    }

    value = simple_strtol(buf, NULL, 10);
    if (geomagnetic_multi_lock() < 0) {
        return count;
    }

    if (hwdep_driver.set_filter_enable(value) == 0) {
        data->filter_enable = !!value;
    }

    geomagnetic_multi_unlock();

    return count;
}

static ssize_t
geomagnetic_filter_len_show(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct input_dev *input_data = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_data);
    int filter_len;

    geomagnetic_multi_lock();

    filter_len = data->filter_len;

    geomagnetic_multi_unlock();

    return sprintf(buf, "%d\n", filter_len);
}

static ssize_t
geomagnetic_filter_len_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf,
        size_t count)
{
    struct input_dev *input_data = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_data);
    struct yas_mag_filter filter;
    int value;


    if (hwdep_driver.get_filter == NULL || hwdep_driver.set_filter == NULL) {
        return -ENOTTY;
    }

    value = simple_strtol(buf, NULL, 10);

    if (geomagnetic_multi_lock() < 0) {
        return count;
    }

    hwdep_driver.get_filter(&filter);
    filter.len = value;
    if (hwdep_driver.set_filter(&filter) == 0) {
        data->filter_len = value;
    }

    geomagnetic_multi_unlock();

    return count;
}

static ssize_t
geomagnetic_filter_noise_show(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct input_dev *input_raw = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_raw);
    int rt;

    geomagnetic_multi_lock();

    rt = sprintf(buf, "%d %d %d\n",
            data->filter_noise[0],
            data->filter_noise[1],
            data->filter_noise[2]);

    geomagnetic_multi_unlock();

    return rt;
}

static ssize_t
geomagnetic_filter_noise_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf,
        size_t count)
{
    struct input_dev *input_raw = to_input_dev(dev);
    struct yas_mag_filter filter;
    struct geomagnetic_data *data = input_get_drvdata(input_raw);
    int32_t filter_noise[3];

    geomagnetic_multi_lock();

    sscanf(buf, "%d %d %d",
            &filter_noise[0],
            &filter_noise[1],
            &filter_noise[2]);
    hwdep_driver.get_filter(&filter);
    memcpy(filter.noise, filter_noise, sizeof(filter.noise));
    if (hwdep_driver.set_filter(&filter) == 0) {
        memcpy(data->filter_noise, filter_noise, sizeof(data->filter_noise));
    }

    geomagnetic_multi_unlock();

    return count;
}

static ssize_t
geomagnetic_filter_threshold_show(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct input_dev *input_data = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_data);
    int32_t filter_threshold;

    geomagnetic_multi_lock();

    filter_threshold = data->filter_threshold;

    geomagnetic_multi_unlock();

    return sprintf(buf, "%d\n", filter_threshold);
}

static ssize_t
geomagnetic_filter_threshold_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf,
        size_t count)
{
    struct input_dev *input_data = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_data);
    struct yas_mag_filter filter;
    int value;


    if (hwdep_driver.get_filter == NULL || hwdep_driver.set_filter == NULL) {
        return -ENOTTY;
    }

    value = simple_strtol(buf, NULL, 10);

    if (geomagnetic_multi_lock() < 0) {
        return count;
    }

    hwdep_driver.get_filter(&filter);
    filter.threshold = value;
    if (hwdep_driver.set_filter(&filter) == 0) {
        data->filter_threshold = value;
    }

    geomagnetic_multi_unlock();

    return count;
}

static ssize_t
geomagnetic_position_show(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    if (hwdep_driver.get_position == NULL) {
        return -ENOTTY;
    }
    return sprintf(buf, "%d\n", hwdep_driver.get_position());
}

static ssize_t
geomagnetic_position_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf,
        size_t count)
{
    int value = simple_strtol(buf, NULL, 10);

    if (hwdep_driver.set_position == NULL) {
        return -ENOTTY;
    }
    hwdep_driver.set_position(value);

    return count;
}

static ssize_t
geomagnetic_data_show(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct input_dev *input_data = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_data);
    int rt;

    rt = sprintf(buf, "%d %d %d\n",
            atomic_read(&data->last_data[0]),
            atomic_read(&data->last_data[1]),
            atomic_read(&data->last_data[2]));

    return rt;
}

static ssize_t
geomagnetic_status_show(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct input_dev *input_data = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_data);
    int rt;

    rt = sprintf(buf, "%d\n", atomic_read(&data->last_status));

    return rt;
}

static ssize_t
geomagnetic_wake_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf,
        size_t count)
{
    struct input_dev *input_data = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_data);
    static int16_t cnt = 1;

    input_report_rel(data->input_data, REL_WAKE, cnt++);

    return count;
}

#if DEBUG

static int geomagnetic_suspend(struct i2c_client *client, pm_message_t mesg);
static int geomagnetic_resume(struct i2c_client *client);

static ssize_t
geomagnetic_debug_suspend_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct input_dev *input = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input);

    return sprintf(buf, "%d\n", data->suspend);
}

static ssize_t
geomagnetic_debug_suspend_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    unsigned long suspend = simple_strtol(buf, NULL, 10);

    if (suspend) {
        pm_message_t msg;
        memset(&msg, 0, sizeof(msg));
        geomagnetic_suspend(this_client, msg);
    } else {
        geomagnetic_resume(this_client);
    }

    return count;
}

#endif /* DEBUG */

static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP,
        geomagnetic_delay_show, geomagnetic_delay_store);
static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP,
        geomagnetic_enable_show, geomagnetic_enable_store);
static DEVICE_ATTR(filter_enable, S_IRUGO|S_IWUSR|S_IWGRP,
        geomagnetic_filter_enable_show, geomagnetic_filter_enable_store);
static DEVICE_ATTR(filter_len, S_IRUGO|S_IWUSR|S_IWGRP,
        geomagnetic_filter_len_show, geomagnetic_filter_len_store);
static DEVICE_ATTR(filter_threshold, S_IRUGO|S_IWUSR|S_IWGRP,
        geomagnetic_filter_threshold_show, geomagnetic_filter_threshold_store);
static DEVICE_ATTR(filter_noise, S_IRUGO|S_IWUSR|S_IWGRP,
        geomagnetic_filter_noise_show, geomagnetic_filter_noise_store);
static DEVICE_ATTR(data, S_IRUGO, geomagnetic_data_show, NULL);
static DEVICE_ATTR(status, S_IRUGO, geomagnetic_status_show, NULL);
static DEVICE_ATTR(wake, S_IWUSR|S_IWGRP, NULL, geomagnetic_wake_store);
static DEVICE_ATTR(position, S_IRUGO|S_IWUSR,
        geomagnetic_position_show, geomagnetic_position_store);
#if DEBUG
static DEVICE_ATTR(debug_suspend, S_IRUGO|S_IWUSR,
        geomagnetic_debug_suspend_show, geomagnetic_debug_suspend_store);
#endif /* DEBUG */

static struct attribute *geomagnetic_attributes[] = {
    &dev_attr_delay.attr,
    &dev_attr_enable.attr,
    &dev_attr_filter_enable.attr,
    &dev_attr_filter_len.attr,
    &dev_attr_filter_threshold.attr,
    &dev_attr_filter_noise.attr,
    &dev_attr_data.attr,
    &dev_attr_status.attr,
    &dev_attr_wake.attr,
    &dev_attr_position.attr,
#if DEBUG
    &dev_attr_debug_suspend.attr,
#endif /* DEBUG */
    NULL
};

static struct attribute_group geomagnetic_attribute_group = {
    .attrs = geomagnetic_attributes
};

static ssize_t
geomagnetic_raw_threshold_show(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct input_dev *input_raw = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_raw);
    int threshold;

    geomagnetic_multi_lock();

    threshold = data->threshold;

    geomagnetic_multi_unlock();

    return sprintf(buf, "%d\n", threshold);
}

static ssize_t
geomagnetic_raw_threshold_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf,
        size_t count)
{
    struct input_dev *input_raw = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_raw);
    int value = simple_strtol(buf, NULL, 10);

    geomagnetic_multi_lock();

    if (0 <= value && value <= 2) {
        data->threshold = value;
        input_report_rel(data->input_raw, REL_RAW_THRESHOLD, value);
    }

    geomagnetic_multi_unlock();

    return count;
}

static ssize_t
geomagnetic_raw_distortion_show(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct input_dev *input_raw = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_raw);
    int rt;

    geomagnetic_multi_lock();

    rt = sprintf(buf, "%d %d %d\n",
            data->distortion[0],
            data->distortion[1],
            data->distortion[2]);

    geomagnetic_multi_unlock();

    return rt;
}

static ssize_t
geomagnetic_raw_distortion_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf,
        size_t count)
{
    struct input_dev *input_raw = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_raw);
    int32_t distortion[3];
    static int32_t val = 1;
    int i;

    geomagnetic_multi_lock();

    sscanf(buf, "%d %d %d",
            &distortion[0],
            &distortion[1],
            &distortion[2]);
    if (distortion[0] > 0 && distortion[1] > 0 && distortion[2] > 0) {
        for (i = 0; i < 3; i++) {
            data->distortion[i] = distortion[i];
        }
        input_report_rel(data->input_raw, REL_RAW_DISTORTION, val++);
    }

    geomagnetic_multi_unlock();

    return count;
}

static ssize_t
geomagnetic_raw_shape_show(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct input_dev *input_raw = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_raw);
    int shape;

    geomagnetic_multi_lock();

    shape = data->shape;

    geomagnetic_multi_unlock();

    return sprintf(buf, "%d\n", shape);
}

static ssize_t
geomagnetic_raw_shape_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf,
        size_t count)
{
    struct input_dev *input_raw = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_raw);
    int value = simple_strtol(buf, NULL, 10);

    geomagnetic_multi_lock();

    if (0 <= value && value <= 1) {
        data->shape = value;
        input_report_rel(data->input_raw, REL_RAW_SHAPE, value);
    }

    geomagnetic_multi_unlock();

    return count;
}

static ssize_t
geomagnetic_raw_offsets_show(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct input_dev *input_raw = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_raw);
    struct yas_mag_offset offset;
    int accuracy;

    geomagnetic_multi_lock();

    offset = data->driver_offset;
    accuracy = atomic_read(&data->last_status);

    geomagnetic_multi_unlock();

    return sprintf(buf, "%d %d %d %d %d %d %d\n",
            offset.hard_offset[0],
            offset.hard_offset[1],
            offset.hard_offset[2],
            offset.calib_offset.v[0],
            offset.calib_offset.v[1],
            offset.calib_offset.v[2],
            accuracy);
}

static ssize_t
geomagnetic_raw_offsets_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf,
        size_t count)
{
    struct input_dev *input_raw = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_raw);
    struct yas_mag_offset offset;
    int32_t hard_offset[3];
    int i, accuracy;

    geomagnetic_multi_lock();

    sscanf(buf, "%d %d %d %d %d %d %d",
            &hard_offset[0],
            &hard_offset[1],
            &hard_offset[2],
            &offset.calib_offset.v[0],
            &offset.calib_offset.v[1],
            &offset.calib_offset.v[2],
            &accuracy);
    if (0 <= accuracy && accuracy <= 3) {
        for (i = 0; i < 3; i++) {
            offset.hard_offset[i] = (int8_t)hard_offset[i];
        }
        if (hwdep_driver.set_offset(&offset) == 0) {
            atomic_set(&data->last_status, accuracy);
            data->driver_offset = offset;
        }
    }

    geomagnetic_multi_unlock();

    return count;
}

#ifdef YAS_MAG_MANUAL_OFFSET
static ssize_t
geomagnetic_raw_manual_offsets_show(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct input_dev *input_raw = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_raw);
    struct yas_vector offset;

    geomagnetic_multi_lock();

    offset = data->manual_offset;

    geomagnetic_multi_unlock();

    return sprintf(buf, "%d %d %d\n", offset.v[0], offset.v[1], offset.v[2]);
}

static ssize_t
geomagnetic_raw_manual_offsets_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf,
        size_t count)
{
    struct input_dev *input_raw = to_input_dev(dev);
    struct geomagnetic_data *data = input_get_drvdata(input_raw);
    struct yas_vector offset;

    geomagnetic_multi_lock();

    sscanf(buf, "%d %d %d", &offset.v[0], &offset.v[1], &offset.v[2]);
    if (hwdep_driver.set_manual_offset(&offset) == 0) {
        data->manual_offset = offset;
    }

    geomagnetic_multi_unlock();

    return count;
}
#endif

static DEVICE_ATTR(threshold, S_IRUGO|S_IWUSR,
        geomagnetic_raw_threshold_show, geomagnetic_raw_threshold_store);
static DEVICE_ATTR(distortion, S_IRUGO|S_IWUSR,
        geomagnetic_raw_distortion_show, geomagnetic_raw_distortion_store);
static DEVICE_ATTR(shape, S_IRUGO|S_IWUSR,
        geomagnetic_raw_shape_show, geomagnetic_raw_shape_store);
static DEVICE_ATTR(offsets, S_IRUGO|S_IWUSR,
        geomagnetic_raw_offsets_show, geomagnetic_raw_offsets_store);
#ifdef YAS_MAG_MANUAL_OFFSET
static DEVICE_ATTR(manual_offsets, S_IRUGO|S_IWUSR,
        geomagnetic_raw_manual_offsets_show, geomagnetic_raw_manual_offsets_store);
#endif

static struct attribute *geomagnetic_raw_attributes[] = {
    &dev_attr_threshold.attr,
    &dev_attr_distortion.attr,
    &dev_attr_shape.attr,
    &dev_attr_offsets.attr,
#ifdef YAS_MAG_MANUAL_OFFSET
    &dev_attr_manual_offsets.attr,
#endif
    NULL
};

static struct attribute_group geomagnetic_raw_attribute_group = {
    .attrs = geomagnetic_raw_attributes
};

/* Interface Functions for Lower Layer */

static int
geomagnetic_work(struct yas_mag_data *magdata)
{
    struct geomagnetic_data *data = i2c_get_clientdata(this_client);
    uint32_t time_delay_ms = 100;
    static int cnt = 0;
    int rt, i, accuracy;

    if (hwdep_driver.measure == NULL || hwdep_driver.get_offset == NULL) {
        return time_delay_ms;
    }

    rt = hwdep_driver.measure(magdata, &time_delay_ms);
    if (rt < 0) {
        YLOGE(("measure failed[%d]\n", rt));
    }
    YLOGD(("xy1y2 [%d][%d][%d] raw[%d][%d][%d]\n",
            magdata->xy1y2.v[0], magdata->xy1y2.v[1], magdata->xy1y2.v[2],
            magdata->xyz.v[0], magdata->xyz.v[1], magdata->xyz.v[2]));

    if (rt >= 0) {
        accuracy = atomic_read(&data->last_status);

        if ((rt & YAS_REPORT_OVERFLOW_OCCURED)
                || (rt & YAS_REPORT_HARD_OFFSET_CHANGED)
                || (rt & YAS_REPORT_CALIB_OFFSET_CHANGED)) {
            static uint16_t count = 1;
            int code = 0;
            int value = 0;

            hwdep_driver.get_offset(&data->driver_offset);
            if (rt & YAS_REPORT_OVERFLOW_OCCURED) {
                atomic_set(&data->last_status, 0);
                accuracy = 0;
            }

            /* report event */
            code |= (rt & YAS_REPORT_OVERFLOW_OCCURED);
            code |= (rt & YAS_REPORT_HARD_OFFSET_CHANGED);
            code |= (rt & YAS_REPORT_CALIB_OFFSET_CHANGED);
            value = (count++ << 16) | (code);
            input_report_rel(data->input_raw, REL_RAW_REPORT, value);
        }

        if (rt & YAS_REPORT_DATA) {
            /* report magnetic data in [nT] */
            input_report_rel(data->input_data, REL_X, magdata->xyz.v[0]);
            input_report_rel(data->input_data, REL_Y, magdata->xyz.v[1]);
            input_report_rel(data->input_data, REL_Z, magdata->xyz.v[2]);

            if (atomic_read(&data->last_data[0]) == magdata->xyz.v[0]
                    && atomic_read(&data->last_data[1]) == magdata->xyz.v[1]
                    && atomic_read(&data->last_data[2]) == magdata->xyz.v[2]) {
                input_report_rel(data->input_data, REL_RAW_THRESHOLD, cnt++);
            }
            if (accuracy == 0) // EV_REL does not process "0". 
	        accuracy = -1;
            input_report_rel(data->input_data, REL_STATUS, accuracy);
            input_sync(data->input_data);

            for (i = 0; i < 3; i++) {
                atomic_set(&data->last_data[i], magdata->xyz.v[i]);
            }
        }

        if (rt & YAS_REPORT_CALIB) {
            /* report raw magnetic data */
            input_report_rel(data->input_raw, REL_X, magdata->raw.v[0]);
            input_report_rel(data->input_raw, REL_Y, magdata->raw.v[1]);
            input_report_rel(data->input_raw, REL_Z, magdata->raw.v[2]);
            input_sync(data->input_raw);
        }
    }
    else {
        time_delay_ms = 100;
    }

    return time_delay_ms;

}

static void
geomagnetic_input_work_func(struct work_struct *work)
{
    struct geomagnetic_data *data = container_of((struct delayed_work *)work,
            struct geomagnetic_data, work);
    uint32_t time_delay_ms;
    struct yas_mag_data magdata;

    time_delay_ms = geomagnetic_work(&magdata);

    if (time_delay_ms > 0) {
        schedule_delayed_work(&data->work, msecs_to_jiffies(time_delay_ms) + 1);
    }
    else {
        schedule_delayed_work(&data->work, 0);
    }
}

static int
geomagnetic_suspend(struct i2c_client *client, pm_message_t mesg)
{
    struct geomagnetic_data *data = i2c_get_clientdata(client);

    if (atomic_read(&data->enable)) {
        cancel_delayed_work_sync(&data->work);
    }
#if DEBUG
    data->suspend = 1;
#endif

    return 0;
}

static int
geomagnetic_resume(struct i2c_client *client)
{
    struct geomagnetic_data *data = i2c_get_clientdata(client);

    if (atomic_read(&data->enable)) {
        schedule_delayed_work(&data->work, 0);
    }

#if DEBUG
    data->suspend = 0;
#endif

    return 0;
}


static int
geomagnetic_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct geomagnetic_data *data = NULL;
    struct input_dev *input_data = NULL, *input_raw = NULL;
    int rt, sysfs_created = 0, sysfs_raw_created = 0;
    int data_registered = 0, raw_registered = 0, i;
    struct yas_mag_filter filter;

    i2c_set_clientdata(client, NULL);
    data = kzalloc(sizeof(struct geomagnetic_data), GFP_KERNEL);
    if (data == NULL) {
        rt = -ENOMEM;
        goto err;
    }
    data->threshold = YAS_DEFAULT_MAGCALIB_THRESHOLD;
	/* Default distortion values are needed to be changed depends on device. */
    //for (i = 0; i < 3; i++) {
    data->distortion[0] = YAS_DEFAULT_MAGCALIB_DISTORTION_1;
    data->distortion[1] = YAS_DEFAULT_MAGCALIB_DISTORTION_2;
    data->distortion[2] = YAS_DEFAULT_MAGCALIB_DISTORTION_3;
    //}
    data->shape = 0;
    atomic_set(&data->enable, 0);
    for (i = 0; i < 3; i++) {
        atomic_set(&data->last_data[i], 0);
    }
    atomic_set(&data->last_status, 0);
    INIT_DELAYED_WORK(&data->work, geomagnetic_input_work_func);
    init_MUTEX(&data->driver_lock);
    init_MUTEX(&data->multi_lock);

    input_data = input_allocate_device();
    if (input_data == NULL) {
        rt = -ENOMEM;
        YLOGE(("geomagnetic_probe: Failed to allocate input_data device\n"));
        goto err;
    }

    input_data->name = GEOMAGNETIC_INPUT_NAME;
    input_data->id.bustype = BUS_I2C;
    set_bit(EV_REL, input_data->evbit);
	input_set_capability(input_data, EV_REL, REL_X);
	input_set_capability(input_data, EV_REL, REL_Y);
	input_set_capability(input_data, EV_REL, REL_Z);
	input_set_capability(input_data, EV_REL, REL_RAW_THRESHOLD);
	input_set_capability(input_data, EV_REL, REL_STATUS);
	input_set_capability(input_data, EV_REL, REL_WAKE);
    input_data->dev.parent = &client->dev;

    rt = input_register_device(input_data);
    if (rt) {
        YLOGE(("geomagnetic_probe: Unable to register input_data device: %s\n",
               input_data->name));
        goto err;
    }
    data_registered = 1;

    rt = sysfs_create_group(&input_data->dev.kobj,
            &geomagnetic_attribute_group);
    if (rt) {
        YLOGE(("geomagnetic_probe: sysfs_create_group failed[%s]\n",
               input_data->name));
        goto err;
    }
    sysfs_created = 1;

    input_raw = input_allocate_device();
    if (input_raw == NULL) {
        rt = -ENOMEM;
        YLOGE(("geomagnetic_probe: Failed to allocate input_raw device\n"));
        goto err;
    }

    input_raw->name = GEOMAGNETIC_INPUT_RAW_NAME;
    input_raw->id.bustype = BUS_I2C;
    set_bit(EV_REL, input_raw->evbit);
	input_set_capability(input_raw, EV_REL, REL_X);
	input_set_capability(input_raw, EV_REL, REL_Y);
	input_set_capability(input_raw, EV_REL, REL_Z);
	input_set_capability(input_raw, EV_REL, REL_RAW_DISTORTION);
	input_set_capability(input_raw, EV_REL, REL_RAW_THRESHOLD);
	input_set_capability(input_raw, EV_REL, REL_RAW_SHAPE);
	input_set_capability(input_raw, EV_REL, REL_RAW_REPORT);
    input_raw->dev.parent = &client->dev;

    rt = input_register_device(input_raw);
    if (rt) {
        YLOGE(("geomagnetic_probe: Unable to register input_raw device: %s\n",
               input_raw->name));
        goto err;
    }
    raw_registered = 1;

    rt = sysfs_create_group(&input_raw->dev.kobj,
            &geomagnetic_raw_attribute_group);
    if (rt) {
        YLOGE(("geomagnetic_probe: sysfs_create_group failed[%s]\n",
               input_data->name));
        goto err;
    }
    sysfs_raw_created = 1;

    this_client = client;
    data->input_raw = input_raw;
    data->input_data = input_data;
    input_set_drvdata(input_data, data);
    input_set_drvdata(input_raw, data);
    i2c_set_clientdata(client, data);

    #if defined(CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER) \
		|| defined(CONFIG_MACH_TIKAL)
    #if defined(CONFIG_MACH_AEGIS)
    if (HWREV < 0x02)
    #elif (CONFIG_MACH_VIPER)
    if (HWREV < 0x03)
	#elif (CONFIG_MACH_TIKAL)
    if (HWREV < 0x12)
    #endif
    {
    	if ((rt = yas529_mag_driver_init(&hwdep_driver)) < 0) {
            YLOGE(("yas_mag_driver_init failed[%d]\n", rt));
            goto err;
        }
    }
    else
    {
    	if ((rt = yas530_mag_driver_init(&hwdep_driver)) < 0) {
            YLOGE(("yas_mag_driver_init failed[%d]\n", rt));
            goto err;
        }
    }
    #else
    #if (YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS530)
    if ((rt = yas530_mag_driver_init(&hwdep_driver)) < 0) {
        YLOGE(("yas_mag_driver_init failed[%d]\n", rt));
        goto err;
    }
    #else
    if ((rt = yas529_mag_driver_init(&hwdep_driver)) < 0) {
        YLOGE(("yas_mag_driver_init failed[%d]\n", rt));
        goto err;
    }
    #endif
    #endif
    if (hwdep_driver.init != NULL) {
        if ((rt = hwdep_driver.init()) < 0) {
            YLOGE(("hwdep_driver.init() failed[%d]\n", rt));
            goto err;
        }
    }
    if (hwdep_driver.set_position != NULL) {
        if (hwdep_driver.set_position(CONFIG_INPUT_YAS_MAGNETOMETER_POSITION) < 0) {
            YLOGE(("hwdep_driver.set_position() failed[%d]\n", rt));
            goto err;
        }
    }
    if (hwdep_driver.get_offset != NULL) {
        if (hwdep_driver.get_offset(&data->driver_offset) < 0) {
            YLOGE(("hwdep_driver get_driver_state failed\n"));
            goto err;
        }
    }
    if (hwdep_driver.get_delay != NULL) {
        data->delay = hwdep_driver.get_delay();
    }
    if (hwdep_driver.set_filter_enable != NULL) {
        /* default to enable */
        if (hwdep_driver.set_filter_enable(1) == 0) {
            data->filter_enable = 1;
        }
    }
    if (hwdep_driver.get_filter != NULL) {
        if (hwdep_driver.get_filter(&filter) < 0) {
            YLOGE(("hwdep_driver get_filter failed\n"));
            goto err;
        }
        data->filter_len = filter.len;
        for (i = 0; i < 3; i++) {
            data->filter_noise[i] = filter.noise[i];
        }
        data->filter_threshold = filter.threshold;
    }

    return 0;

err:
    if (data != NULL) {
        if (input_raw != NULL) {
            if (sysfs_raw_created) {
                sysfs_remove_group(&input_raw->dev.kobj,
                        &geomagnetic_raw_attribute_group);
            }
            if (raw_registered) {
                input_unregister_device(input_raw);
            }
            else {
                input_free_device(input_raw);
            }
        }
        if (input_data != NULL) {
            if (sysfs_created) {
                sysfs_remove_group(&input_data->dev.kobj,
                        &geomagnetic_attribute_group);
            }
            if (data_registered) {
                input_unregister_device(input_data);
            }
            else {
                input_free_device(input_data);
            }
        }
        kfree(data);
    }

    return rt;
}

static int
geomagnetic_remove(struct i2c_client *client)
{
    struct geomagnetic_data *data = i2c_get_clientdata(client);

    if (data != NULL) {
        geomagnetic_disable(data);
        if (hwdep_driver.term != NULL) {
            hwdep_driver.term();
        }

        input_unregister_device(data->input_raw);
        sysfs_remove_group(&data->input_data->dev.kobj,
                &geomagnetic_attribute_group);
        sysfs_remove_group(&data->input_raw->dev.kobj,
                &geomagnetic_raw_attribute_group);
        input_unregister_device(data->input_data);
        kfree(data);
    }

    return 0;
}

/* I2C Device Driver */
static struct i2c_device_id geomagnetic_idtable[] = {
    {GEOMAGNETIC_I2C_DEVICE_NAME, 0},
    {}
};
MODULE_DEVICE_TABLE(i2c, geomagnetic_idtable);

static struct i2c_driver geomagnetic_i2c_driver = {
    .driver = {
        .name       = GEOMAGNETIC_I2C_DEVICE_NAME,
        .owner      = THIS_MODULE,
    },

    .id_table       = geomagnetic_idtable,
    .probe          = geomagnetic_probe,
    .remove         = geomagnetic_remove,
    .suspend        = geomagnetic_suspend,
    .resume         = geomagnetic_resume,
};

static int __init
geomagnetic_init(void)
{
    return i2c_add_driver(&geomagnetic_i2c_driver);
}

static void __exit
geomagnetic_term(void)
{
    i2c_del_driver(&geomagnetic_i2c_driver);
}

#ifdef GEOMAGNETIC_PLATFORM_API
static int
geomagnetic_api_enable(int enable)
{
    struct geomagnetic_data *data = i2c_get_clientdata(this_client);
    int rt;

    if (geomagnetic_multi_lock() < 0) {
        return -1;
    }
    enable = !!enable;

    if ((rt = hwdep_driver.set_enable(enable)) == 0) {
        atomic_set(&data->enable, enable);
        if (enable) {
            rt = hwdep_driver.set_delay(20);
        }
    }

    geomagnetic_multi_unlock();

    return rt;
}

int
geomagnetic_api_resume(void)
{
    return geomagnetic_api_enable(1);
}
EXPORT_SYMBOL(geomagnetic_api_resume);

int
geomagnetic_api_suspend(void)
{
    return geomagnetic_api_enable(0);
}
EXPORT_SYMBOL(geomagnetic_api_suspend);

int
geomagnetic_api_read(int *xyz, int *raw, int *xy1y2, int *accuracy)
{
    struct geomagnetic_data *data = i2c_get_clientdata(this_client);
    struct yas_mag_data magdata;
    int i;

    geomagnetic_work(&magdata);
    if (xyz != NULL) {
        for (i = 0; i < 3; i++) {
            xyz[i] = magdata.xyz.v[i];
        }
    }
    if (raw != NULL) {
        for (i = 0; i < 3; i++) {
            raw[i] = magdata.raw.v[i];
        }
    }
    if (xy1y2 != NULL) {
        for (i = 0; i < 3; i++) {
            xy1y2[i] = magdata.xy1y2.v[i];
        }
    }
    if (accuracy != NULL) {
        *accuracy = atomic_read(&data->last_status);
    }

    return 0;
}
EXPORT_SYMBOL(geomagnetic_api_read);
#endif

module_init(geomagnetic_init);
module_exit(geomagnetic_term);

MODULE_AUTHOR("Yamaha Corporation");
#if YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS529
MODULE_DESCRIPTION("YAS529 Geomagnetic Sensor Driver");
#elif YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS530
MODULE_DESCRIPTION("YAS530 Geomagnetic Sensor Driver");
#elif defined(CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER)
MODULE_DESCRIPTION("YAS530 Geomagnetic Sensor Driver");
#endif
MODULE_LICENSE( "GPL" );
MODULE_VERSION("3.2.410");
