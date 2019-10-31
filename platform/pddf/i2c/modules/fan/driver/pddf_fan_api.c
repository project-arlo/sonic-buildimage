/*
 * Copyright 2019 Broadcom. All rights reserved.
 * The term “Broadcom” refers to Broadcom Inc. and/or its subsidiaries.
 *
 * Description of various APIs related to FAN component
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/dmi.h>
#include "pddf_fan_defs.h"
#include "pddf_fan_driver.h"

/*#define FAN_DEBUG*/
#ifdef FAN_DEBUG
#define fan_dbg(...) printk(__VA_ARGS__)
#else
#define fan_dbg(...)
#endif


void get_fan_duplicate_sysfs(int idx, char *str)
{
	switch (idx)
	{
		case FAN1_FRONT_RPM:
			strcpy(str, "fan1_input");
			break;
		case FAN2_FRONT_RPM:
			strcpy(str, "fan2_input");
            break;
		case FAN3_FRONT_RPM:
			strcpy(str, "fan3_input");
            break;
		case FAN4_FRONT_RPM:
			strcpy(str, "fan4_input");
            break;
		case FAN5_FRONT_RPM:
			strcpy(str, "fan5_input");
            break;
		case FAN6_FRONT_RPM:
			strcpy(str, "fan6_input");
            break;
		case FAN1_REAR_RPM:
			strcpy(str, "fan11_input");
            break;
		case FAN2_REAR_RPM:
			strcpy(str, "fan12_input");
            break;
		case FAN3_REAR_RPM:
			strcpy(str, "fan13_input");
            break;
		case FAN4_REAR_RPM:
			strcpy(str, "fan14_input");
            break;
		case FAN5_REAR_RPM:
			strcpy(str, "fan15_input");
            break;
		case FAN6_REAR_RPM:
			strcpy(str, "fan16_input");
            break;
		default:
			break;
	}

	return;
}


int fan_update_hw(struct device *dev, struct fan_attr_info *info, FAN_DATA_ATTR *udata)
{
	int status = 0;
    struct i2c_client *client = to_i2c_client(dev);
	FAN_SYSFS_ATTR_DATA *sysfs_attr_data = NULL;


    mutex_lock(&info->update_lock);

	sysfs_attr_data = udata->access_data;
	if (sysfs_attr_data->pre_set != NULL)
	{
		status = (sysfs_attr_data->pre_set)(client, udata, info);
		if (status!=0)
			printk(KERN_ERR "%s: pre_set function fails for %s attribute\n", __FUNCTION__, udata->aname);
	}
	if (sysfs_attr_data->do_set != NULL)
	{
		status = (sysfs_attr_data->do_set)(client, udata, info);
		if (status!=0)
			printk(KERN_ERR "%s: do_set function fails for %s attribute\n", __FUNCTION__, udata->aname);

	}
	if (sysfs_attr_data->post_set != NULL)
	{
		status = (sysfs_attr_data->post_set)(client, udata, info);
		if (status!=0)
			printk(KERN_ERR "%s: post_set function fails for %s attribute\n", __FUNCTION__, udata->aname);
	}

    mutex_unlock(&info->update_lock);

    return 0;
}

int fan_update_attr(struct device *dev, struct fan_attr_info *info, FAN_DATA_ATTR *udata)
{
	int status = 0;
    struct i2c_client *client = to_i2c_client(dev);
	FAN_SYSFS_ATTR_DATA *sysfs_attr_data = NULL;


    mutex_lock(&info->update_lock);

    if (time_after(jiffies, info->last_updated + HZ + HZ / 2) || !info->valid) 
	{
        dev_dbg(&client->dev, "Starting pddf_fan update\n");
        info->valid = 0;

		sysfs_attr_data = udata->access_data;
		if (sysfs_attr_data->pre_get != NULL)
		{
			status = (sysfs_attr_data->pre_get)(client, udata, info);
			if (status!=0)
				printk(KERN_ERR "%s: pre_get function fails for %s attribute\n", __FUNCTION__, udata->aname);
		}
		if (sysfs_attr_data->do_get != NULL)
		{
			status = (sysfs_attr_data->do_get)(client, udata, info);
			if (status!=0)
                printk(KERN_ERR "%s: do_get function fails for %s attribute\n", __FUNCTION__, udata->aname);

		}
		if (sysfs_attr_data->post_get != NULL)
		{
			status = (sysfs_attr_data->post_get)(client, udata, info);
			if (status!=0)
                printk(KERN_ERR "%s: post_get function fails for %s attribute\n", __FUNCTION__, udata->aname);
		}

		
        info->last_updated = jiffies;
        info->valid = 1;
    }

    mutex_unlock(&info->update_lock);

    return 0;
}

ssize_t fan_show_default(struct device *dev, struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct i2c_client *client = to_i2c_client(dev);
    struct fan_data *data = i2c_get_clientdata(client);
    FAN_PDATA *pdata = (FAN_PDATA *)(client->dev.platform_data);
    FAN_DATA_ATTR *usr_data = NULL;
    struct fan_attr_info *attr_info = NULL;
    int i, status=0;
	char new_str[ATTR_NAME_LEN] = "";
	FAN_SYSFS_ATTR_DATA *ptr = NULL;

    for (i=0;i<data->num_attr;i++)
    {
		ptr = (FAN_SYSFS_ATTR_DATA *)pdata->fan_attrs[i].access_data;
		get_fan_duplicate_sysfs(ptr->index , new_str);
        if (strcmp(attr->dev_attr.attr.name, pdata->fan_attrs[i].aname) == 0 || strcmp(attr->dev_attr.attr.name, new_str) == 0)
        {
			/*printk(KERN_ERR "%s's show func: access_data from %s, idx %d, new_str=%s\n", attr->dev_attr.attr.name, pdata->fan_attrs[i].aname, ptr->index, new_str);*/
			attr_info = &data->attr_info[i];
            usr_data = &pdata->fan_attrs[i];
			strcpy(new_str, "");
        }
    }

    if (attr_info==NULL || usr_data==NULL)
    {
        printk(KERN_ERR "%s is not supported attribute for this client\n", usr_data->aname);
		goto exit;
	}

    fan_update_attr(dev, attr_info, usr_data);

	/*Decide the o/p based on attribute type */
	switch(attr->index)
	{
		case FAN1_PRESENT:
		case FAN2_PRESENT:
		case FAN3_PRESENT:
		case FAN4_PRESENT:
		case FAN5_PRESENT:
		case FAN6_PRESENT:
		case FAN1_DIRECTION:
		case FAN2_DIRECTION:
		case FAN3_DIRECTION:
		case FAN4_DIRECTION:
		case FAN5_DIRECTION:
		case FAN6_DIRECTION:
		case FAN1_FRONT_RPM:
		case FAN2_FRONT_RPM:
		case FAN3_FRONT_RPM:
		case FAN4_FRONT_RPM:
		case FAN5_FRONT_RPM:
		case FAN6_FRONT_RPM:
		case FAN1_REAR_RPM:
		case FAN2_REAR_RPM:
		case FAN3_REAR_RPM:
		case FAN4_REAR_RPM:
		case FAN5_REAR_RPM:
		case FAN6_REAR_RPM:
		case FAN1_PWM:
		case FAN2_PWM:
		case FAN3_PWM:
		case FAN4_PWM:
		case FAN5_PWM:
		case FAN6_PWM:
		case FAN1_FAULT:
		case FAN2_FAULT:
		case FAN3_FAULT:
		case FAN4_FAULT:
		case FAN5_FAULT:
		case FAN6_FAULT:
    status = attr_info->val.intval;
			break;
		default:
			fan_dbg(KERN_ERR "%s: Unable to find the attribute index for %s\n", __FUNCTION__, usr_data->aname);
			status = 0;
	}

exit:
    return sprintf(buf, "%d\n", status);
}


ssize_t fan_store_default(struct device *dev, struct device_attribute *da, const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct i2c_client *client = to_i2c_client(dev);
    struct fan_data *data = i2c_get_clientdata(client);
    FAN_PDATA *pdata = (FAN_PDATA *)(client->dev.platform_data);
    FAN_DATA_ATTR *usr_data = NULL;
    struct fan_attr_info *attr_info = NULL;
    int i, ret ;
	uint32_t val;

    for (i=0;i<data->num_attr;i++)
    {
		if (strcmp(data->attr_info[i].name, attr->dev_attr.attr.name) == 0 && strcmp(pdata->fan_attrs[i].aname, attr->dev_attr.attr.name) == 0)
        {
            attr_info = &data->attr_info[i];
            usr_data = &pdata->fan_attrs[i];
        }
    }

    if (attr_info==NULL || usr_data==NULL) {
		printk(KERN_ERR "%s is not supported attribute for this client\n", attr->dev_attr.attr.name);
		goto exit;
	}

	switch(attr->index)
	{
		case FAN1_PWM:
		case FAN2_PWM:
		case FAN3_PWM:
		case FAN4_PWM:
		case FAN5_PWM:
		case FAN6_PWM:
			ret = kstrtoint(buf, 10, &val);
			if (ret)
			{
				printk(KERN_ERR "%s: Unable to convert string into value for %s\n", __FUNCTION__, usr_data->aname);
				return ret;
			}
			/*Update the value of attr_info here, and use it to update the HW values*/
			attr_info->val.intval = val;
			break;
		default:
			printk(KERN_ERR "%s: Unable to find the attr index for %s\n", __FUNCTION__, usr_data->aname);
			goto exit;
	}

	fan_dbg(KERN_ERR "%s: pwm to be set is %d\n", __FUNCTION__, val);
	fan_update_hw(dev, attr_info, usr_data);

exit:
	return count;
}

int sonic_i2c_get_fan_present_default(void *client, FAN_DATA_ATTR *udata, void *info)
{
    int status = 0;
    int val = 0;
    struct fan_attr_info *painfo = (struct fan_attr_info *)info;

	val = i2c_smbus_read_byte_data((struct i2c_client *)client, udata->offset);
	painfo->val.intval = ((val & udata->mask) == udata->cmpval);

	/*fan_dbg(KERN_ERR "presence: val:0x%x, mask:0x%x, present_value = 0x%x\n", val, udata->mask, painfo->val.intval);*/

    return status;
}

int sonic_i2c_get_fan_rpm_default(void *client, FAN_DATA_ATTR *udata, void *info)
{
    int status = 0;
	uint32_t val = 0;
    struct fan_attr_info *painfo = (struct fan_attr_info *)info;

	if (udata->len == 1)
	{
		val = i2c_smbus_read_byte_data((struct i2c_client *)client, udata->offset);
	}
	else if (udata->len ==2)
	{
		val = i2c_smbus_read_word_swapped((struct i2c_client *)client, udata->offset);
		
	}

	if (udata->is_divisor)
		painfo->val.intval = udata->mult / (val >> 3);
	else
		painfo->val.intval = udata->mult * val;

	return status;
}


int sonic_i2c_get_fan_direction_default(void *client, FAN_DATA_ATTR *udata, void *info)
{
    int status = 0;
	uint32_t val = 0;
    struct fan_attr_info *painfo = (struct fan_attr_info *)info;

	val = i2c_smbus_read_byte_data((struct i2c_client *)client, udata->offset);
	painfo->val.intval = ((val & udata->mask) == udata->cmpval);
	/*fan_dbg(KERN_ERR "direction: val:0x%x, mask:0x%x, final val:0x%x\n", val, udata->mask, painfo->val.intval);*/

    return status;
}


int sonic_i2c_set_fan_pwm_default(struct i2c_client *client, FAN_DATA_ATTR *udata, void *info)
{
    int status = 0;
	uint32_t val = 0;
    struct fan_attr_info *painfo = (struct fan_attr_info *)info;

	val = painfo->val.intval & udata->mask;

	if (val > 255)
	{
	  return -EINVAL;
	}

	if (udata->len == 1)
		i2c_smbus_write_byte_data(client, udata->offset, val);
	else if (udata->len == 2)
	{
		uint8_t val_lsb = val & 0xFF;
		uint8_t val_hsb = (val >> 8) & 0xFF;
		/* TODO: Check this logic for LE and BE */
		i2c_smbus_write_byte_data(client, udata->offset, val_lsb);
		i2c_smbus_write_byte_data(client, udata->offset+1, val_hsb);
	}
	else
	{
		printk(KERN_DEBUG "%s: pwm should be of len 1/2 bytes. Not setting the pwm as the length is %d\n", __FUNCTION__, udata->len);
	}
	
	return status;
}


int sonic_i2c_get_fan_pwm_default(void *client, FAN_DATA_ATTR *udata, void *info)
{
    int status = 0;
	uint32_t val = 0;
    struct fan_attr_info *painfo = (struct fan_attr_info *)info;

	if (udata->len == 1)
	{
		val = i2c_smbus_read_byte_data((struct i2c_client *)client, udata->offset);
	}
	else if (udata->len ==2)
	{
		val = i2c_smbus_read_word_swapped((struct i2c_client *)client, udata->offset);
		
	}

	val = val & udata->mask;
	painfo->val.intval = val;
    return status;
}

int sonic_i2c_get_fan_fault_default(void *client, FAN_DATA_ATTR *udata, void *info)
{
    int status = 0;
	uint32_t val = 0;
    struct fan_attr_info *painfo = (struct fan_attr_info *)info;

	/*Assuming fan fault to be denoted by 1 byte only*/
	val = i2c_smbus_read_byte_data((struct i2c_client *)client, udata->offset);

	val = val & udata->mask;
	painfo->val.intval = val;
    return status;
}


int pddf_fan_post_probe_default(struct i2c_client *client, const struct i2c_device_id *dev_id)
{

	/*Dummy func for now - check the respective platform modules*/
    return 0;
}
