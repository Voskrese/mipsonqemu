/* drivers/mtd/maps/plat-ram.c
 *
 * (c) 2004-2005 Simtec Electronics
 *	http://www.simtec.co.uk/products/SWLINUX/
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * Generic platfrom device based RAM map
 *
 * $Id: plat-ram.c,v 1.7 2005/11/07 11:14:28 gleixner Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/kdev_t.h>
#include <linux/major.h>
#include <linux/root_dev.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/plat-ram.h>

#include <asm/io.h>

/* private structure for each mtd platform ram device created */

struct platram_info {
	struct device		*dev;
	struct mtd_info		*mtd;
	struct map_info		 map;
	struct mtd_partition	*partitions;
	struct platdata_mtd_ram	*pdata;
};

/* to_platram_info()
 *
 * device private data to struct platram_info conversion
*/

static inline struct platram_info *to_platram_info(struct platform_device *dev)
{
	return (struct platram_info *)platform_get_drvdata(dev);
}

/* platram_setrw
 *
 * call the platform device's set rw/ro control
 *
 * to = 0 => read-only
 *    = 1 => read-write
*/

static inline void platram_setrw(struct platram_info *info, int to)
{
	if (info->pdata == NULL)
		return;

	if (info->pdata->set_rw != NULL)
		(info->pdata->set_rw)(info->dev, to);
}

/* platram_remove
 *
 * called to remove the device from the driver's control
*/

static int platram_remove(struct platform_device *pdev)
{
	struct platram_info *info = to_platram_info(pdev);

	platform_set_drvdata(pdev, NULL);

	dev_dbg(&pdev->dev, "removing device\n");

	if (info == NULL)
		return 0;

	if (info->mtd) {
#ifdef CONFIG_MTD_PARTITIONS
		del_mtd_partitions(info->mtd);
		if (info->partitions)
			kfree(info->partitions);
#endif
		del_mtd_device(info->mtd);
		map_destroy(info->mtd);
	}

	/* ensure ram is left read-only */

	platram_setrw(info, PLATRAM_RO);

	/* release resources */

	if (info->map.virt != NULL)
		iounmap(info->map.virt);

	kfree(info);

	return 0;
}

/* platram_probe
 *
 * called from device drive system when a device matching our
 * driver is found.
*/

static int platram_probe(struct platform_device *pdev)
{
	struct platdata_mtd_ram	*pdata;
	struct platram_info *info;
	struct resource *res;
	int err = 0;

	dev_dbg(&pdev->dev, "probe entered\n");

	if (pdev->dev.platform_data == NULL) {
		dev_err(&pdev->dev, "no platform data supplied\n");
		err = -ENOENT;
		goto exit_error;
	}

	pdata = pdev->dev.platform_data;

	info = kmalloc(sizeof(*info), GFP_KERNEL);
	if (info == NULL) {
		dev_err(&pdev->dev, "no memory for flash info\n");
		err = -ENOMEM;
		goto exit_error;
	}

	memset(info, 0, sizeof(*info));
	platform_set_drvdata(pdev, info);

	info->dev = &pdev->dev;
	info->pdata = pdata;

	/* get the resource for the memory mapping */

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	if (res == NULL) {
		dev_err(&pdev->dev, "no memory resource specified\n");
		err = -ENOENT;
		goto exit_free;
	}

	dev_dbg(&pdev->dev, "got platform resource %p (0x%lx)\n", res, res->start);

	/* setup map parameters */

	info->map.phys = res->start;
	info->map.size = (res->end - res->start) + 1;
	info->map.name = pdata->mapname != NULL ? pdata->mapname : (char *)pdev->name;
	info->map.bankwidth = pdata->bankwidth;

	/* remap the memory area */

	info->map.virt = ioremap(res->start, info->map.size);
	dev_dbg(&pdev->dev, "virt %p, %lu bytes\n", info->map.virt, info->map.size);

	if (info->map.virt == NULL) {
		dev_err(&pdev->dev, "failed to ioremap() region\n");
		err = -EIO;
		goto exit_free;
	}

	simple_map_init(&info->map);

	dev_dbg(&pdev->dev, "initialised map, probing for mtd\n");

	/* probe for the right mtd map driver */

	info->mtd = do_map_probe("map_ram" , &info->map);
	if (info->mtd == NULL) {
		dev_err(&pdev->dev, "failed to probe for map_ram\n");
		err = -ENOMEM;
		goto exit_free;
	}

	info->mtd->owner = THIS_MODULE;

	platram_setrw(info, PLATRAM_RW);

	/* check to see if there are any available partitions, or wether
	 * to add this device whole */

#ifdef CONFIG_MTD_PARTITIONS
	if (pdata->nr_partitions > 0) {
		err = add_mtd_partitions(info->mtd, pdata->partitions,
					 pdata->nr_partitions);
		if (err < 0)
			goto exit_free;
	} else if (pdata->probes) {
		err = parse_mtd_partitions(info->mtd, pdata->probes,
					   &info->partitions, 0);
		if (err > 0) {
			err = add_mtd_partitions(info->mtd, info->partitions,
						 err);
		}
		if (err < 0)
			goto exit_free;
	}
#endif /* CONFIG_MTD_PARTITIONS */

	if (add_mtd_device(info->mtd)) {
		dev_err(&pdev->dev, "add_mtd_device() failed\n");
		err = -ENOMEM;
		goto exit_free;
	}

	dev_info(&pdev->dev, "registered mtd device\n");

	if (pdata->root_dev)
		ROOT_DEV = MKDEV(MTD_BLOCK_MAJOR, info->mtd->index);

	return 0;

 exit_free:
	platram_remove(pdev);
 exit_error:
	return err;
}

/* device driver info */

static struct platform_driver platram_driver = {
	.probe		= platram_probe,
	.remove		= platram_remove,
	.driver		= {
		.name	= "mtd-ram",
		.owner	= THIS_MODULE,
	},
};

/* module init/exit */

static int __init platram_init(void)
{
	printk("Generic platform RAM MTD, (c) 2004 Simtec Electronics\n");
	return platform_driver_register(&platram_driver);
}

static void __exit platram_exit(void)
{
	platform_driver_unregister(&platram_driver);
}

module_init(platram_init);
module_exit(platram_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ben Dooks <ben@simtec.co.uk>");
MODULE_DESCRIPTION("MTD platform RAM map driver");
