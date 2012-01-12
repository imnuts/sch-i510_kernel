/**
 * Samsung Virtual Network driver using OneDram device
 *
 * Copyright (C) 2010 Samsung Electronics. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

//#define DEBUG

#if defined(DEBUG)
#define PERF_DEBUG
#endif

#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/jiffies.h>

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/if.h>
#include <linux/if_arp.h>

#include <linux/if_phonet.h>
#include <linux/phonet.h>
#include <net/phonet/phonet.h>

#include <linux/kernel_sec_common.h>

#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>

#define DEFAULT_WAKE_TIME (2*HZ)
#endif

#if defined(PERF_DEBUG)
#define _pdbg(dev, format, arg...) dev_dbg(dev, format, ## arg)
#else
#define _pdbg(dev, format, arg...) do { } while (0)
#endif

#include "pdp.h"
#include "sipc.h"
#include "sipc4.h"

#define dbg_loga(s, args...) printk(KERN_ERR "[SVNet] <%s:%d> " s, __func__, __LINE__,  ##args)
#define dbg_loge(s, args...) printk(KERN_ERR "[SVNet/Err] <%s:%d> " s, __func__, __LINE__,  ##args)
#define dbg_log(s, args...)  printk(s, ##args)


#define SVNET_DEV_ADDR 0xa0

//#define PARSE_IP_PACKET


enum {
	SVNET_NORMAL = 0,
	SVNET_RESET,
	SVNET_EXIT,
	SVNET_MAX,
};

struct svnet_stat {
	unsigned int st_wq_state;
	unsigned long st_recv_evt;
	unsigned long st_recv_pkt_ph;
	unsigned long st_recv_pkt_pdp;
	unsigned long st_do_write;
	unsigned long st_do_read;
	unsigned long st_do_rx;
};
static struct svnet_stat stat;

struct svnet_evt {
	struct list_head list;
	u32 event;
};

struct svnet_evt_head {
	struct list_head list;
	u32 len;
	spinlock_t lock;
};

struct svnet {
	struct net_device *ndev;
	const struct attribute_group *group;

	struct workqueue_struct *wq;
	struct work_struct work_read;
	struct delayed_work work_write;
	struct delayed_work work_rx;

	struct work_struct work_exit;
	int exit_flag;

	struct sk_buff_head txq;
	struct svnet_evt_head rxq;

	struct sipc *si;
#ifdef CONFIG_HAS_WAKELOCK
	struct wake_lock wlock;
	long wake_time; /* jiffies */
#endif
};


static char tcp_flag_str[32];


#ifdef CONFIG_HAS_WAKELOCK
static inline void _wake_lock_init(struct svnet *sn)
{
	wake_lock_init(&sn->wlock, WAKE_LOCK_SUSPEND, "svnet");
	sn->wake_time = DEFAULT_WAKE_TIME;
}

static inline void _wake_lock_destroy(struct svnet *sn)
{
	wake_lock_destroy(&sn->wlock);
}

static inline void _wake_lock_timeout(struct svnet *sn)
{
	wake_lock_timeout(&sn->wlock, sn->wake_time);
}

static inline void _wake_lock_settime(struct svnet *sn, long time)
{
	if (sn)
		sn->wake_time = time;
}

static inline long _wake_lock_gettime(struct svnet *sn)
{
	return sn?sn->wake_time:DEFAULT_WAKE_TIME;
}
#else
#define _wake_lock_init(sn) do { } while(0)
#define _wake_lock_destroy(sn) do { } while(0)
#define _wake_lock_timeout(sn) do { } while(0)
#define _wake_lock_settime(sn, time) do { } while(0)
#define _wake_lock_gettime(sn) (0)
#endif

static struct svnet *svnet_dev;

static unsigned long long tmp_itor;
static unsigned long long tmp_xtow;
static unsigned long long time_max_itor;
static unsigned long long time_max_xtow;
static unsigned long long time_max_read;
static unsigned long long time_max_write;

extern unsigned long long time_max_semlat;

static int _queue_evt(struct svnet_evt_head *h, u32 event);

static ssize_t show_version(struct device *d,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "Samsung IPC version %s\n", sipc_version);
}

static ssize_t show_waketime(struct device *d,
		struct device_attribute *attr, char *buf)
{
	char *p = buf;
	unsigned int msec;
	unsigned long j;

	if (!svnet_dev)
		return 0;

	j = _wake_lock_gettime(svnet_dev);
	msec = jiffies_to_msecs(j);
	p += sprintf(p, "%u\n", msec);

	return p - buf;
}

static ssize_t store_waketime(struct device *d,
		struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long msec;
	unsigned long j;
	int r;

	if (!svnet_dev)
		return count;

	r = strict_strtoul(buf, 10, &msec);
	if (r)
		return count;

	j = msecs_to_jiffies(msec);
	_wake_lock_settime(svnet_dev, j);

	return count;
}

static inline int _show_stat(char *buf)
{
	char *p = buf;

	p += sprintf(p, "Stat -------- \n");
	p += sprintf(p, "\twork state: %d\n", stat.st_wq_state);
	p += sprintf(p, "\trecv mailbox: %lu\n", stat.st_recv_evt);
	p += sprintf(p, "\trecv phonet: %lu\n", stat.st_recv_pkt_ph);
	p += sprintf(p, "\trecv packet: %lu\n", stat.st_recv_pkt_pdp);
	p += sprintf(p, "\twrite count: %lu\n", stat.st_do_write);
	p += sprintf(p, "\tread count: %lu\n", stat.st_do_read);
	p += sprintf(p, "\trx count: %lu\n", stat.st_do_rx);
	p += sprintf(p, "\n");

	return p - buf;
}

static ssize_t show_latency(struct device *d,
		struct device_attribute *attr, char *buf)
{
	char *p = buf;

	p += sprintf(p, "Max read latency:  %12llu ns\n", time_max_itor);
	p += sprintf(p, "Max read time:     %12llu ns\n", time_max_read);
	p += sprintf(p, "Max write latency: %12llu ns\n", time_max_xtow);
	p += sprintf(p, "Max write time:    %12llu ns\n", time_max_write);
	p += sprintf(p, "Max sem. latency:  %12llu ns\n", time_max_semlat);

	return p - buf;
}

static ssize_t store_latency(struct device *d,
		struct device_attribute *attr, const char *buf, size_t count)
{
	if (!svnet_dev)
		return count;

	time_max_itor = 0;
	time_max_read = 0;
	time_max_xtow = 0;
	time_max_write = 0;
	time_max_semlat = 0;

	return count;
}

static ssize_t show_debug(struct device *d,
		struct device_attribute *attr, char *buf)
{
	char *p = buf;

	if (!svnet_dev)
		return 0;

	p += _show_stat(p);

	p += sprintf(p, "Event queue ----- \n");
	p += sprintf(p, "\tTX queue\t%u\n", skb_queue_len(&svnet_dev->txq));
	p += sprintf(p, "\tRX queue\t%u\n", svnet_dev->rxq.len);

	p += sipc_debug_show(svnet_dev->si, p);

	return p - buf;
}

static ssize_t store_debug(struct device *d,
		struct device_attribute *attr, const char *buf, size_t count)
{
	if (!svnet_dev)
		return count;

	switch (buf[0]) {
	case 'R':
		sipc_debug(svnet_dev->si, buf);
		break;
	default:

		break;
	}

	return count;
}

static DEVICE_ATTR(version, S_IRUGO, show_version, NULL);
static DEVICE_ATTR(latency, S_IRUGO | S_IWUSR, show_latency, store_latency);
static DEVICE_ATTR(waketime, S_IRUGO | S_IWUSR, show_waketime, store_waketime);
static DEVICE_ATTR(debug, S_IRUGO | S_IWUSR, show_debug, store_debug);
#ifdef SUSPEND_RESUME_BRIDGE
static DEVICE_ATTR(resume, S_IRUGO | S_IWUSR | S_IWGRP, NULL, store_bridge_resume);
static DEVICE_ATTR(suspend, S_IRUGO | S_IWUSR | S_IWGRP, NULL, store_bridge_suspend);
#endif
#ifdef SET_INTERFACE_ID
static DEVICE_ATTR(interfaceid, S_IRUGO | S_IWUGO, NULL, store_intf_id);
#endif

static struct attribute *svnet_attributes[] = {
	&dev_attr_version.attr,
	&dev_attr_waketime.attr,
	&dev_attr_debug.attr,
	&dev_attr_latency.attr,
#ifdef SUSPEND_RESUME_BRIDGE
	&dev_attr_resume.attr,
	&dev_attr_suspend.attr,
#endif
#ifdef SET_INTERFACE_ID
	&dev_attr_interfaceid.attr,
#endif
	NULL
};

static const struct attribute_group svnet_group = {
	.attrs = svnet_attributes,
};

void parse_ip_packet(u8 *ip_pkt)
{
    u8   ip_ver, ip_hdr_len;
    u16  ip_pkt_len;
    u8   protocol;
    u32  src_ip_addr;
    u32  dst_ip_addr;
    u8  *tcp_pkt;
    u8  *udp_pkt;
    u16  src_port, dst_port;
    u32  seq_num;
    u32  ack_num;
    u8   tcp_hdr_len;
    u8   tcp_flag;

/*---------------------------------------------------------------------------
   
                               IP header format

       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |Version|  IHL  |Type of Service|          Total Length         |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |         Identification        |Flags|      Fragment Offset    |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |  Time to Live |    Protocol   |         Header Checksum       |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                       Source Address                          |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                    Destination Address                        |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                    Options                    |    Padding    |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

       IHL - Header Length 
       Flags - Consist of 3 bits
               First bit is kept 0
	       Second bit is Dont Fragment bit.
	       Third bit is More Fragments bit.

---------------------------------------------------------------------------*/
    ip_ver     = (ip_pkt[0] & 0xF0) >> 4;
    ip_hdr_len = (ip_pkt[0] & 0x0F) << 2;
    ip_pkt_len = (((u16)ip_pkt[2]) << 8) + ((u16)ip_pkt[3]);
    protocol   = ip_pkt[9];
    src_ip_addr = (((u16)ip_pkt[12]) << 24) + (((u16)ip_pkt[13]) << 16) +
                  (((u16)ip_pkt[14]) << 8) + ((u16)ip_pkt[15]);
    dst_ip_addr = (((u16)ip_pkt[16]) << 24) + (((u16)ip_pkt[17]) << 16) +
                  (((u16)ip_pkt[18]) << 8) + ((u16)ip_pkt[19]);

    dbg_loga("IP:: version = %d, IP packet len = %d, IP header len = %d\n",
                ip_ver, ip_pkt_len, ip_hdr_len);

    if (protocol != 6 && protocol != 17)
        dbg_loga("IP:: UL protocol == %d\n", protocol);

    dbg_loga("IP:: src addr = %d.%d.%d.%d, dst addr = %d.%d.%d.%d\n",
		ip_pkt[12], ip_pkt[13], ip_pkt[14], ip_pkt[15],
		ip_pkt[16], ip_pkt[17], ip_pkt[18], ip_pkt[19]);

    if (protocol == 6)  // TCP
    {
/*-------------------------------------------------------------------------

                           TCP Header Format

       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |          Source Port          |       Destination Port        |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                        Sequence Number                        |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                    Acknowledgment Number                      |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |  Data |           |U|A|P|R|S|F|                               |
       | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
       |       |           |G|K|H|T|N|N|                               |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |           Checksum            |         Urgent Pointer        |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                    Options                    |    Padding    |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                             data                              |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


-------------------------------------------------------------------------*/
        tcp_pkt = ip_pkt + ip_hdr_len;

        src_port = (((u16)tcp_pkt[0]) << 8) + ((u16)tcp_pkt[1]);
        dst_port = (((u16)tcp_pkt[2]) << 8) + ((u16)tcp_pkt[3]);
        seq_num  = (((u16)tcp_pkt[4]) << 24) + (((u16)tcp_pkt[5]) << 16) +
                   (((u16)tcp_pkt[6]) << 8) + ((u16)tcp_pkt[7]);
        ack_num  = (((u16)tcp_pkt[8]) << 24) + (((u16)tcp_pkt[9]) << 16) +
                   (((u16)tcp_pkt[10]) << 8) + ((u16)tcp_pkt[11]);
        tcp_hdr_len = (tcp_pkt[12] & 0xF0) >> 2;
        tcp_flag = tcp_pkt[13];

		memset(tcp_flag_str, 0, sizeof(tcp_flag_str));
        if (tcp_flag & 0x01)
			strcat(tcp_flag_str, "FIN ");
        if (tcp_flag & 0x02)
			strcat(tcp_flag_str, "SYN ");
        if (tcp_flag & 0x04)
			strcat(tcp_flag_str, "RST ");
        if (tcp_flag & 0x08)
			strcat(tcp_flag_str, "PSH ");
        if (tcp_flag & 0x10)
			strcat(tcp_flag_str, "ACK ");
        if (tcp_flag & 0x20)
			strcat(tcp_flag_str, "URG ");

        dbg_loga("TCP:: src port = %d, dst port = %d\n", src_port, dst_port);
        dbg_loga("TCP:: seq# = 0x%08x(%u), ack# = 0x%08x(%u)\n", seq_num, seq_num, ack_num, ack_num);
		dbg_loga("TCP:: flag = %s\n", tcp_flag_str);
    }
    else if (protocol == 17)    // UDP
    {
/*-------------------------------------------------------------------------

                           UDP Header Format

       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |          Source Port          |       Destination Port        |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |            Length             |           Checksum            |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                             data                              |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


-------------------------------------------------------------------------*/
        udp_pkt = ip_pkt + ip_hdr_len;

        src_port = (((u16)udp_pkt[0]) << 8) + ((u16)udp_pkt[1]);
        dst_port = (((u16)udp_pkt[2]) << 8) + ((u16)udp_pkt[3]);

        dbg_loga("UDP:: src port = %d, dst port = %d\n", src_port, dst_port);

        if (dst_port == 53) // DNS
            dbg_loga("UDP:: DNS query!!!\n");
        else if (src_port == 53)
            dbg_loga("UDP:: DNS response!!!\n");
    }
}

int vnet_start_xmit(struct sk_buff *skb, struct net_device *ndev)
{
	struct svnet *sn;
	struct pdp_priv *priv;

#ifdef SVNET_PDP_ETHER
	skb_pull(skb,14);
#endif
	dev_dbg(&ndev->dev, "recv inet packet %p: %d bytes\n", skb, skb->len);
	stat.st_recv_pkt_pdp++;

	priv = netdev_priv(ndev);
	if (!priv)
		goto drop;

	sn = netdev_priv(priv->parent);
	if (!sn)
		goto drop;

	if (!tmp_xtow)
		tmp_xtow = cpu_clock(smp_processor_id());

#ifdef PARSE_IP_PACKET
	{
		struct net_device *ndev = skb->dev;
		struct pdp_priv *priv;
		int len = skb->len;
		int res;

		if (skb->protocol != __constant_htons(ETH_P_PHONET)) {
			priv = netdev_priv(ndev);
			res = PN_PDP(priv->channel);
			if (CHID(res) >= CHID_PSD_DATA1 && CHID(res) <= CHID_PSD_DATA4) {
				dbg_loga("\n");
				dbg_loga(">>>>>>>>>> Outgoing IP packet >>>>>>>>>>\n");
				parse_ip_packet(skb->data);
			}
		}

	}
#endif

	skb_queue_tail(&sn->txq, skb);

	_wake_lock_timeout(sn);
	queue_delayed_work(sn->wq, &sn->work_write, 0);

	return NETDEV_TX_OK;

drop:
	ndev->stats.tx_dropped++;

	return NETDEV_TX_OK;
}

static int svnet_xmit(struct sk_buff *skb, struct net_device *ndev)
{
	struct svnet *sn;

	if (skb->protocol != __constant_htons(ETH_P_PHONET)) {
		dev_err(&ndev->dev, "recv not a phonet message\n");
		goto drop;
	}

	stat.st_recv_pkt_ph++;
	dev_dbg(&ndev->dev, "recv packet %p: %d bytes\n", skb, skb->len);

	sn = netdev_priv(ndev);

	if (sipc_check_skb(sn->si, skb)) {
		sipc_do_cmd(sn->si, skb);
		return NETDEV_TX_OK;
	}

	if (!tmp_xtow)
		tmp_xtow = cpu_clock(smp_processor_id());

#ifdef PARSE_IP_PACKET
	{
		struct net_device *ndev = skb->dev;
		struct pdp_priv *priv;
		int len = skb->len;
		int res;

		if (skb->protocol != __constant_htons(ETH_P_PHONET)) {
			priv = netdev_priv(ndev);
			res = PN_PDP(priv->channel);
			if (CHID(res) >= CHID_PSD_DATA1 && CHID(res) <= CHID_PSD_DATA4) {
				dbg_loga("\n");
				dbg_loga(">>>>>>>>>> Outgoing IP packet >>>>>>>>>>\n");
				parse_ip_packet(skb->data);
			}
		}

	}
#endif

	skb_queue_tail(&sn->txq, skb);

	_wake_lock_timeout(sn);
	queue_delayed_work(sn->wq, &sn->work_write, 0);

	return NETDEV_TX_OK;

drop:
	dev_kfree_skb(skb);
	ndev->stats.tx_dropped++;
	return NETDEV_TX_OK;
}

static int _queue_evt(struct svnet_evt_head *h, u32 event)
{
	unsigned long flags;
	struct svnet_evt *e;

	e = kmalloc(sizeof(struct svnet_evt), GFP_ATOMIC);
	if (!e)
		return -ENOMEM;

	e->event = event;

	spin_lock_irqsave(&h->lock, flags);
	list_add_tail(&e->list, &h->list);
	h->len++;
	spin_unlock_irqrestore(&h->lock, flags);

	return 0;
}

static void _queue_purge(struct svnet_evt_head *h)
{
	unsigned long flags;
	struct svnet_evt *e, *next;

	spin_lock_irqsave(&h->lock, flags);
	list_for_each_entry_safe(e, next, &h->list, list) {
		list_del(&e->list);
		h->len--;
		kfree(e);
	}
	spin_unlock_irqrestore(&h->lock, flags);
}

static u32 _dequeue_evt(struct svnet_evt_head *h)
{
	unsigned long flags;
	struct list_head *p;
	struct svnet_evt *e;
	u32 event;

	spin_lock_irqsave(&h->lock, flags);
	p = h->list.next;
	if (p == &h->list) {
		e = NULL;
		event = 0;
	} else {
		e = list_entry(p, struct svnet_evt, list);
		list_del(&e->list);
		h->len--;
		event = e->event;
	}
	spin_unlock_irqrestore(&h->lock, flags);

	if (e)
		kfree(e);

	return event;
}

static int _proc_private_event(struct svnet *sn, u32 evt)
{
	switch(evt) {
	case SIPC_EXIT_MB:
		dev_err(&sn->ndev->dev, "Modem crash message received\n");
		sn->exit_flag = SVNET_EXIT;
		break;
	case SIPC_RESET_MB:
		dev_err(&sn->ndev->dev, "Modem reset message received\n");
		sn->exit_flag = SVNET_RESET;
		break;
	default:
		return 0;
	}

	queue_work(sn->wq, &sn->work_exit);

	return 1;
}

static void svnet_queue_event(u32 evt, void *data)
{
	struct net_device *ndev = (struct net_device *)data;
	struct svnet *sn;
	int r;

	if (!tmp_itor)
		tmp_itor = cpu_clock(smp_processor_id());

	stat.st_recv_evt++;

	if (!ndev)
		return;

	sn = netdev_priv(ndev);
	if (!sn)
		return;

	r = _proc_private_event(sn, evt);
	if (r)
		return;

#if 0
	r = _queue_evt(&sn->rxq, evt);
	if (r) {
		dev_err(&sn->ndev->dev, "Not enough memory: event skipped\n");
		return;
	}
#endif

	_wake_lock_timeout(sn);
	queue_work(sn->wq, &sn->work_read);
}

static int svnet_open(struct net_device *ndev)
{
	struct svnet *sn = netdev_priv(ndev);

	dev_dbg(&ndev->dev, "%s\n", __func__);

	/* TODO: check modem state */

	if (!sn->si) {
		sn->si = sipc_open(svnet_queue_event, ndev);
		if (IS_ERR(sn->si)) {
			dev_err(&ndev->dev, "IPC init error\n");
			return PTR_ERR(sn->si);
		}
		sn->exit_flag = SVNET_NORMAL;
	}

	netif_wake_queue(ndev);

	return 0;
}

static int svnet_close(struct net_device *ndev)
{
	struct svnet *sn = netdev_priv(ndev);

	dev_dbg(&ndev->dev, "%s\n", __func__);

	if(sn->wq)
		flush_workqueue(sn->wq);
	skb_queue_purge(&sn->txq);

	if (sn->si)
		sipc_close(&sn->si);

	netif_stop_queue(ndev);

	return 0;
}

static int svnet_ioctl(struct net_device *ndev, struct ifreq *ifr, int cmd)
{
	struct if_phonet_req *req = (struct if_phonet_req *)ifr;

	switch (cmd) {
	case SIOCPNGAUTOCONF:
		req->ifr_phonet_autoconf.device = SVNET_DEV_ADDR;
		return 0;
	}

	return -ENOIOCTLCMD;
}

static const struct net_device_ops svnet_ops = {
	.ndo_open = svnet_open,
	.ndo_stop = svnet_close,
	.ndo_start_xmit = svnet_xmit,
	.ndo_do_ioctl = svnet_ioctl,
};

static void svnet_setup(struct net_device *ndev)
{
	ndev->features = 0;
	ndev->netdev_ops = &svnet_ops;
	ndev->header_ops = &phonet_header_ops;
	ndev->type = ARPHRD_PHONET;
	ndev->flags = IFF_POINTOPOINT | IFF_NOARP;
	ndev->mtu = PHONET_MAX_MTU;
	ndev->hard_header_len = 1;
	ndev->dev_addr[0] = SVNET_DEV_ADDR;
	ndev->addr_len = 1;
	ndev->tx_queue_len = 1000;

//	ndev->destructor = free_netdev;
}

static void svnet_read_wq(struct work_struct *work)
{
	struct svnet *sn = container_of(work,
			struct svnet, work_read);
	u32 event;
	u32 event_total;
	int r = 0;
	int contd = 0;
	unsigned long long t, d;

	t = cpu_clock(smp_processor_id());
	if (tmp_itor) {
		d = t - tmp_itor;
		_pdbg(&sn->ndev->dev, "int_to_read %llu ns\n", d);
		tmp_itor = 0;
		if (time_max_itor < d)
			time_max_itor = d;
	}

	dev_dbg(&sn->ndev->dev, "%s\n", __func__);
	stat.st_do_read++;

	stat.st_wq_state = 1;
	event_total = 83;
#if 0
	event = _dequeue_evt(&sn->rxq);
	while (event) {
		event_total |= event;
		event = _dequeue_evt(&sn->rxq);
	}
#endif

	// isn't it possible that merge the events?
	dev_dbg(&sn->ndev->dev, "event %x\n", event_total);

	if (sn->si) {
		if (event_total)
			r = sipc_read(sn->si, event_total, &contd);
		if (r < 0) {
			dev_err(&sn->ndev->dev, "ret %d -> queue %x\n",
					r, event_total);
			_queue_evt(&sn->rxq, event_total);
		}
	} else {
		dev_err(&sn->ndev->dev,
				"IPC not work, skip event %x\n", event_total);
	}

	if (contd > 0)
		queue_delayed_work(sn->wq, &sn->work_rx, 0);

	switch (r) {
	case -EINVAL:
		dev_err(&sn->ndev->dev, "Invalid argument\n");
		break;
	case -EBADMSG:
		dev_err(&sn->ndev->dev, "Bad message, purge the buffer\n");
		break;
	case -ETIMEDOUT:
		dev_err(&sn->ndev->dev, "Timed out\n");
		break;
	default:

		break;
	}

	stat.st_wq_state = 2;

	d = cpu_clock(smp_processor_id()) - t;
	_pdbg(&sn->ndev->dev, "read_time %llu ns\n", d);
	if (d > time_max_read)
		time_max_read = d;
}

static void svnet_write_wq(struct work_struct *work)
{
	struct svnet *sn = container_of(work,
			struct svnet, work_write.work);
	int r;
	unsigned long long t, d;

	t = cpu_clock(smp_processor_id());
	if (tmp_xtow) {
		d = t - tmp_xtow;
		_pdbg(&sn->ndev->dev, "xmit_to_write %llu ns\n", d);
		tmp_xtow = 0;
		if (d > time_max_xtow)
			time_max_xtow = d;
	}

	dev_dbg(&sn->ndev->dev, "%s\n", __func__);
	stat.st_do_write++;

	stat.st_wq_state = 3;
	if (sn->si)
		r = sipc_write(sn->si, &sn->txq);
	else {
		skb_queue_purge(&sn->txq);
		dev_err(&sn->ndev->dev, "IPC not work, drop packet\n");
		r = 0;
	}

	switch (r) {
	case -ENOSPC:
		dev_err(&sn->ndev->dev, "buffer is full, wait...\n");
		queue_delayed_work(sn->wq, &sn->work_write, HZ/10);
		break;
	case -EINVAL:
		dev_err(&sn->ndev->dev, "Invalid arugment\n");
		break;
	case -ENXIO:
		dev_err(&sn->ndev->dev, "IPC not work, purge the queue\n");
		break;
	case -ETIMEDOUT:
		dev_err(&sn->ndev->dev, "Timed out\n");
		break;
	default:
		/* do nothing */
		break;
	}

	stat.st_wq_state = 4;
	d = cpu_clock(smp_processor_id()) - t;
	_pdbg(&sn->ndev->dev, "write_time %llu ns\n", d);
	if (d > time_max_write)
		time_max_write = d;
}

static void svnet_rx_wq(struct work_struct *work)
{
	struct svnet *sn = container_of(work,
			struct svnet, work_rx.work);
	int r = 0;

	dev_dbg(&sn->ndev->dev, "%s\n", __func__);
	stat.st_do_rx++;

	stat.st_wq_state = 5;
	if (sn->si)
		r = sipc_rx(sn->si);

	if (r > 0)
		queue_delayed_work(sn->wq, &sn->work_rx, HZ/10);

	stat.st_wq_state = 6;
}

static char *uevent_envs[SVNET_MAX] = {
	"",
	"MAILBOX=cp_reset", /* reset */
	"MAILBOX=cp_exit", /* exit */
};
static void svnet_exit_wq(struct work_struct *work)
{
	struct svnet *sn = container_of(work,
			struct svnet, work_exit);
	char *envs[2] = { NULL, NULL };

	dev_dbg(&sn->ndev->dev, "%s: %d\n", __func__, sn->exit_flag);

	if (sn->exit_flag == SVNET_NORMAL || sn->exit_flag >= SVNET_MAX)
		return;

	envs[0] = uevent_envs[sn->exit_flag];
	kobject_uevent_env(&sn->ndev->dev.kobj, KOBJ_OFFLINE, envs);

	_queue_purge(&sn->rxq);
	skb_queue_purge(&sn->txq);

	if(kernel_sec_get_debug_level() != KERNEL_SEC_DEBUG_LEVEL_LOW) {
		sipc_ramdump(sn->si);
	}

#if 0
	rtnl_lock();
	if (netif_running(sn->ndev))
		dev_close(sn->ndev);
	rtnl_unlock();
#endif
}

static inline void _init_data(struct svnet *sn)
{
	INIT_WORK(&sn->work_read, svnet_read_wq);
	INIT_DELAYED_WORK(&sn->work_write, svnet_write_wq);
	INIT_DELAYED_WORK(&sn->work_rx, svnet_rx_wq);
	INIT_WORK(&sn->work_exit, svnet_exit_wq);

	INIT_LIST_HEAD(&sn->rxq.list);
	spin_lock_init(&sn->rxq.lock);
	sn->rxq.len = 0;
	skb_queue_head_init(&sn->txq);
}

static void _free(struct svnet *sn)
{
	if (!sn)
		return;

	_wake_lock_destroy(sn);

	if (sn->group)
		sysfs_remove_group(&sn->ndev->dev.kobj, &svnet_group);

	if (sn->wq) {
		flush_workqueue(sn->wq);
		destroy_workqueue(sn->wq);
	}

	if (sn->si) {
		sipc_close(&sn->si);
		sipc_exit();
	}

	if (sn->ndev)
		unregister_netdev(sn->ndev);

	// sn is ndev's priv
	free_netdev(sn->ndev);
}

static int __init svnet_init(void)
{
	int r;
	struct svnet *sn = NULL;
	struct net_device *ndev;

	ndev = alloc_netdev(sizeof(struct svnet), "svnet%d", svnet_setup);
	if (!ndev) {
		r = -ENOMEM;
		goto err;
	}
	netif_stop_queue(ndev);
	sn = netdev_priv(ndev);

	_wake_lock_init(sn);

	r = register_netdev(ndev);
	if (r) {
		dev_err(&ndev->dev, "failed to register netdev\n");
		goto err;
	}
	sn->ndev = ndev;

	_init_data(sn);

//	sn->wq = create_workqueue("svnetd");
	sn->wq = create_rt_workqueue("svnetd");
	if (!sn->wq) {
		dev_err(&ndev->dev, "failed to create a workqueue\n");
		r = -ENOMEM;
		goto err;
	}

	r = sysfs_create_group(&sn->ndev->dev.kobj, &svnet_group);
	if (r) {
		dev_err(&ndev->dev, "failed to create sysfs group\n");
		goto err;
	}
	sn->group = &svnet_group;

	dev_dbg(&ndev->dev, "Svnet dev: %p\n", sn);
	svnet_dev = sn;

	return 0;

err:
	_free(sn);
	return r;
}

static void __exit svnet_exit(void)
{

	_free(svnet_dev);
	svnet_dev = NULL;
}

module_init(svnet_init);
module_exit(svnet_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Suchang Woo <suchang.woo@samsung.com>");
MODULE_DESCRIPTION("Samsung Virtual network interface");
