/**
 * Copyright (c) 2014 Himanshu Chauhan
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * @file virtio_pci.c
 * @author Himanshu Chauhan <hschauhan@nulltrace.org>
 * @brief Virtio PCI Transport Layer Emulator
 */

#include <vmm_error.h>
#include <vmm_stdio.h>
#include <vmm_heap.h>
#include <vmm_modules.h>
#include <vmm_devemu.h>
#include <emu/virtio.h>
#include <emu/virtio_queue.h>
#include <emu/virtio_pci.h>
#include <emu/pci/pci_emu_core.h>


#define VIRTIO_PCI_EMU_IPRIORITY	(PCI_EMU_CORE_IPRIORITY + VIRTIO_IPRIORITY + 1)

#define MODULE_DESC			"Virtio PCI Transport Layer"
#define MODULE_AUTHOR			"Himanshu Chauhan"
#define MODULE_LICENSE			"GPL"
#define MODULE_IPRIORITY		VIRTIO_PCI_EMU_IPRIORITY
#define	MODULE_INIT			virtio_pci_emulator_init
#define	MODULE_EXIT			virtio_pci_emulator_exit

static int virtio_pci_notify(struct virtio_device *dev, u32 vq)
{
	struct virtio_pci_dev *m = dev->tra_data;

	m->config.interrupt_state |= VIRTIO_MMIO_INT_VRING;

	vmm_devemu_emulate_irq(m->guest, m->irq, 1);

	return VMM_OK;
}

int virtio_pci_config_read(struct virtio_pci_dev *m,
			   u32 offset, void *dst,
			   u32 dst_len)
{
	int rc = VMM_OK;

	switch (offset) {
	case VIRTIO_MMIO_MAGIC_VALUE:
	case VIRTIO_MMIO_VERSION:
	case VIRTIO_MMIO_DEVICE_ID:
	case VIRTIO_MMIO_VENDOR_ID:
	case VIRTIO_MMIO_STATUS:
	case VIRTIO_MMIO_INTERRUPT_STATUS:
		*(u32 *)dst = (*(u32 *)(((void *)&m->config) + offset));
		break;
	case VIRTIO_MMIO_HOST_FEATURES:
		*(u32 *)dst = m->dev.emu->get_host_features(&m->dev);
		break;
	case VIRTIO_MMIO_QUEUE_PFN:
		*(u32 *)dst = m->dev.emu->get_pfn_vq(&m->dev,
					     m->config.queue_sel);
		break;
	case VIRTIO_MMIO_QUEUE_NUM_MAX:
		*(u32 *)dst = m->dev.emu->get_size_vq(&m->dev,
					      m->config.queue_sel);
		break;
	default:
		break;
	}

	return rc;
}

#if 0
static int virtio_mmio_read(struct virtio_mmio_dev *m,
			    u32 offset, u32 *dst)
{
	/* Device specific config write */
	if (offset >= VIRTIO_MMIO_CONFIG) {
		offset -= VIRTIO_MMIO_CONFIG;
		return virtio_config_read(&m->dev, offset, dst, 4);
	}

	return virtio_mmio_config_read(m, offset, dst, 4);
}
#endif

static int virtio_pci_config_write(struct virtio_pci_dev *m,
				   physical_addr_t offset,
				   void *src, u32 src_len)
{
	int rc = VMM_OK;
	u32 val = *(u32 *)(src);

	switch (offset) {
	case VIRTIO_MMIO_HOST_FEATURES_SEL:
	case VIRTIO_MMIO_GUEST_FEATURES_SEL:
	case VIRTIO_MMIO_QUEUE_SEL:
	case VIRTIO_MMIO_STATUS:
		*(u32 *)(((void *)&m->config) + offset) = val;
		break;
	case VIRTIO_MMIO_GUEST_FEATURES:
		if (m->config.guest_features_sel == 0)  {
			m->dev.emu->set_guest_features(&m->dev, val);
		}
		break;
	case VIRTIO_MMIO_GUEST_PAGE_SIZE:
		m->config.guest_page_size = val;
		break;
	case VIRTIO_MMIO_QUEUE_NUM:
		m->config.queue_num = val;
		m->dev.emu->set_size_vq(&m->dev, 
					m->config.queue_sel,
					m->config.queue_num);
		break;
	case VIRTIO_MMIO_QUEUE_ALIGN:
		m->config.queue_align = val;
		break;
	case VIRTIO_MMIO_QUEUE_PFN:
		m->dev.emu->init_vq(&m->dev, 
				    m->config.queue_sel,
				    m->config.guest_page_size,
				    m->config.queue_align,
				    val);
		break;
	case VIRTIO_MMIO_QUEUE_NOTIFY:
		m->dev.emu->notify_vq(&m->dev, val);
		break;
	case VIRTIO_MMIO_INTERRUPT_ACK:
		m->config.interrupt_state &= ~val;
		vmm_devemu_emulate_irq(m->guest, m->irq, 0);
		break;
	default:
		break;
	};

	return rc;
}

#if 0
static int virtio_mmio_write(struct virtio_mmio_dev *m,
			     u32 offset, u32 src_mask, u32 src)
{
	src = src & ~src_mask;

	/* Device specific config write */
	if (offset >= VIRTIO_MMIO_CONFIG) {
		offset -= VIRTIO_MMIO_CONFIG;
		return virtio_config_write(&m->dev, (u32)offset, &src, 4);
	}

	return virtio_mmio_config_write(m, (u32)offset, &src, 4);
}
#endif

static struct virtio_transport mmio_tra = {
	.name = "virtio_mmio",
	.notify = virtio_mmio_notify,
};

static int virtio_pci_emulator_reset(struct pci_device *pdev)
{
	struct virtio_pci_dev *m = pdev->priv;

	m->config.interrupt_state = 0x0;
	vmm_devemu_emulate_irq(vdev->guest, vdev->irq, 0);

	return virtio_reset(&vdev->dev);
}

static int virtio_pci_emulator_probe(struct pci_device *pdev,
				     struct vmm_guest *guest,
				     const struct vmm_devtree_nodeid *eid)
{
	int rc = VMM_OK;
	struct virtio_pci_dev *vdev;

	vdev = vmm_zalloc(sizeof(struct virtio_pci_dev));
	if (!vdev) {
		rc = VMM_ENOMEM;
		goto virtio_pci_probe_done;
	}

	vdev->guest = guest;

	vmm_snprintf(vdev->dev.name, VIRTIO_DEVICE_MAX_NAME_LEN, 
		     "%s/%s", guest->name, edev->node->name); 
	vdev->dev.edev = edev;
	vdev->dev.tra = &mmio_tra;
	vdev->dev.tra_data = m;
	vdev->dev.guest = guest;

	vdev->config = (struct virtio_pci_config) {
		.vendor_id      = 0x52535658, /* XVSR */
		.queue_num_max  = 256,
	};

	rc = vmm_devtree_read_u32(pdev->node, "virtio_type",
				  &vdev->config.device_id);
	if (rc) {
		goto virtio_pci_probe_freestate_fail;
	}

	vdev->dev.id.type = vdev->config.device_id;

	rc = vmm_devtree_irq_get(pdev->node, &vdev->irq, 0);
	if (rc) {
		goto virtio_mmio_probe_freestate_fail;
	}

	if ((rc = virtio_register_device(&vdev->dev))) {
		goto virtio_mmio_probe_freestate_fail;
	}

	pdev->priv = m;

	goto virtio_mmio_probe_done;

virtio_mmio_probe_freestate_fail:
	vmm_free(m);
virtio_mmio_probe_done:
	return rc;
}

static int virtio_pci_emulator_remove(struct pci_device *pdev)
{
	struct virtio_pci_dev *vdev = edev->priv;

	if (vdev) {
		virtio_unregister_device((struct virtio_device *)&vdev->dev);
		vmm_free(vdev);
		pdev->priv = NULL;
	}

	return VMM_OK;
}

static struct vmm_devtree_nodeid virtio_pci_emuid_table[] = {
	{ .type = "virtio", 
	  .compatible = "virtio,pci", 
	},
	{ /* end of list */ },
};

static struct pci_dev_emulator virtio_pci_emulator = {
	.name = "virtio-pci",
	.match_table = virtio_pci_emuid_table,
	.probe = virtio_pci_emulator_probe,
	.reset = virtio_pci_emulator_reset,
	.remove = virtio_pci_emulator_remove,
};

static int __init virtio_pci_emulator_init(void)
{
	return pci_emu_register_device(&virtio_pci_emulator);
}

static void __exit virtio_pci_emulator_exit(void)
{
	pci_emu_unregister_device(&virtio_pci_emulator);
}

VMM_DECLARE_MODULE(MODULE_DESC,
		   MODULE_AUTHOR,
		   MODULE_LICENSE,
		   MODULE_IPRIORITY,
		   MODULE_INIT,
		   MODULE_EXIT);
