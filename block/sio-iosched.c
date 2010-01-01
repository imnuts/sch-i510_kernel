/*
 * Simple IO scheduler
 *
 * Copyright (C) 2010 Miguel Boton <mboton@gmail.com>
 */

#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/init.h>

/* tunables */
static const int sync_expire = HZ / 2;
static const int async_expire = 5 * HZ;

/* scheduler data */
struct sio_data {
	/* fifo queues */
	struct list_head fifo_list[2];

	/* fifo expire */
	int fifo_expire[2];
};

/* constants */
#define SYNC	0
#define ASYNC	1

static void
sio_merged_requests(struct request_queue *q, struct request *rq,
		    struct request *next)
{
	list_del_init(&next->queuelist);
}

static void
sio_add_request(struct request_queue *q, struct request *rq)
{
	struct sio_data *sd = q->elevator->elevator_data;
	int sync = rq_is_sync(rq);

	rq_set_fifo_time(rq, jiffies + sd->fifo_expire[sync]);
	list_add_tail(&rq->queuelist, &sd->fifo_list[sync]);
}

static int
sio_queue_empty(struct request_queue *q)
{
	struct sio_data *sd = q->elevator->elevator_data;

	return list_empty(&sd->fifo_list[SYNC]) &&
	       list_empty(&sd->fifo_list[ASYNC]);
}

static struct request *
sio_expired_request(struct sio_data *sd, int sync)
{
	struct request *rq;

	if (list_empty(&sd->fifo_list[sync]))
		return NULL;

	rq = rq_entry_fifo(sd->fifo_list[sync].next);

	/* expired request */
	if (time_after(jiffies, rq_fifo_time(rq)))
		return rq;

	return NULL;
}

static struct request *
sio_choose_expired_request(struct sio_data *sd)
{
	struct request *sync = sio_expired_request(sd, SYNC);
	struct request *async = sio_expired_request(sd, ASYNC);

	/* check expired requests */
	if (sync && async)
		return async;
	else if (sync)
		return sync;

	return async;

}

static struct request *
sio_choose_request(struct sio_data *sd)
{
	if (!list_empty(&sd->fifo_list[SYNC]))
		return rq_entry_fifo(sd->fifo_list[SYNC].next);

	if (!list_empty(&sd->fifo_list[ASYNC]))
		return rq_entry_fifo(sd->fifo_list[ASYNC].next);

	return NULL;
}

static int
sio_dispatch_requests(struct request_queue *q, int force)
{
	struct sio_data *sd = q->elevator->elevator_data;
	struct request *rq;

	rq = sio_choose_expired_request(sd);
	if (!rq) {
		rq = sio_choose_request(sd);
		if (!rq)
			return 0;
	}

	list_del_init(&rq->queuelist);
	elv_dispatch_sort(q, rq);

	return 1;
}

static struct request *
sio_former_request(struct request_queue *q, struct request *rq)
{
	struct sio_data *sd = q->elevator->elevator_data;
	struct list_head *queue = &sd->fifo_list[rq_is_sync(rq)];

	if (rq->queuelist.prev == queue)
		return NULL;

	return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static struct request *
sio_latter_request(struct request_queue *q, struct request *rq)
{
	struct sio_data *sd = q->elevator->elevator_data;
	struct list_head *queue = &sd->fifo_list[rq_is_sync(rq)];

	if (rq->queuelist.next == queue)
		return NULL;

	return list_entry(rq->queuelist.next, struct request, queuelist);
}

static void *
sio_init_queue(struct request_queue *q)
{
	struct sio_data *sd;

	sd = kmalloc_node(sizeof(*sd), GFP_KERNEL, q->node);
	if (!sd)
		return NULL;

	INIT_LIST_HEAD(&sd->fifo_list[SYNC]);
	INIT_LIST_HEAD(&sd->fifo_list[ASYNC]);

	sd->fifo_expire[SYNC] = sync_expire;
	sd->fifo_expire[ASYNC] = async_expire;

	return sd;
}

static void
sio_exit_queue(struct elevator_queue *e)
{
	struct sio_data *sd = e->elevator_data;

	BUG_ON(!list_empty(&sd->fifo_list[SYNC]));
	BUG_ON(!list_empty(&sd->fifo_list[ASYNC]));

	kfree(sd);
}

static struct elevator_type elevator_sio = {
	.ops = {
		.elevator_merge_req_fn		= sio_merged_requests,
		.elevator_dispatch_fn		= sio_dispatch_requests,
		.elevator_add_req_fn		= sio_add_request,
		.elevator_queue_empty_fn	= sio_queue_empty,
		.elevator_former_req_fn		= sio_former_request,
		.elevator_latter_req_fn		= sio_latter_request,
		.elevator_init_fn		= sio_init_queue,
		.elevator_exit_fn		= sio_exit_queue,
	},
	.elevator_name = "sio",
	.elevator_owner = THIS_MODULE,
};

static int __init sio_init(void)
{
	elv_register(&elevator_sio);

	return 0;
}

static void __exit sio_exit(void)
{
	elv_unregister(&elevator_sio);
}

module_init(sio_init);
module_exit(sio_exit);


MODULE_AUTHOR("Miguel Boton");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple IO scheduler");
