#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/sched.h> 
#include <linux/fs.h>
#include <net/sock.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/netlink.h>
#include <linux/buffer_head.h>
#include <net/net_namespace.h>
#include <linux/slab.h>
#define NETLINK_TEST 17 
const char d[2] = " ";
static unsigned long period_sec= 1;
static unsigned long period_nsec=0;
static struct hrtimer hr_timer;
static ktime_t interval;
static struct task_struct *thread1;
static struct sock *socket_ptr = NULL;
long prev_idle = 0;
long prev_total = 0;

struct sched_param {
	int sched_priority;
};

struct global_data {
	int buffer_size;
};
typedef struct global_data Global_data;

module_param(period_sec,ulong, 0);
module_param(period_nsec, ulong, 0);

static void netlink_recv_msg(struct sk_buff * skb) {
    struct nlmsghdr * nlh = NULL;
    if (skb == NULL) {
        printk(KERN_INFO "skb is NULL \n");
        return;
    }
    nlh = (struct nlmsghdr *)skb->data;
    printk(KERN_INFO "%s: received netlink message payload: %s\n", __FUNCTION__, (char *)NLMSG_DATA(nlh));
}

struct netlink_kernel_cfg cfg = {
    .input = netlink_recv_msg,
};

static int init_global(Global_data* g) {
	g->buffer_size = 1024;
	return 0;
}

static int read_proc_stat(Global_data* g) {
	struct file *f;
	static char buffer[g->buffer_size];
	mm_segment_t fs;
	printk("read_proc_stat");
	f = filp_open("/proc/stat", O_RDONLY, 0);
	if(!f){
		printk(KERN_ALERT "filp_open error!!.\n");
		filp_close(f, NULL);
		return -1;
	} else {
		// Get current segment descriptor
		printk("1");
		fs = get_fs();
		printk("2");
		// Set segment descriptor associated to kernel space
		set_fs(get_ds());
		printk("3");
		// Read the file
		f->f_op->read(f, buffer, size, &f->f_pos);
		printk("4");
		// Restore segment descriptor
		set_fs(fs);
		// See what we read from file
		printk(KERN_INFO "buf:%s", buffer);
		filp_close(f, NULL);
		return 0;
	}
}

//https://stackoverflow.com/questions/1184274/read-write-files-within-a-linux-kernel-module
static int thread_fn(void * data) {
    Global_data* g = (Global_data*) data;
	while (!kthread_should_stop()){ 
		set_current_state(TASK_INTERRUPTIBLE);
  		schedule();
		if (read_proc_stat(g)==-1) {
			continue;
		} else {
	      	continue;
		}
	}
	return 0;
}


/*timer expiration*/
static enum hrtimer_restart timer_callback(struct hrtimer *timer_for_restart) {
	ktime_t currtime;
	wake_up_process(thread1);
  	currtime  = ktime_get();
  	hrtimer_forward(timer_for_restart, currtime , interval);
	return HRTIMER_RESTART;
}

/* init function - logs that initialization happened, returns success */
static int simple_init (void) {
	int ret;
	struct sched_param param= {.sched_priority=95};
	socket_ptr = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
    printk(KERN_INFO "link created");
    Global_data g;
    if (init_global(&g) == -1) {
    	return 1;
    }
    printk(KERN_INFO "init_global");
	
	interval=ktime_set(period_sec,period_nsec);
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer.function=&timer_callback;

	thread1 = kthread_create(thread_fn, (void *)(&g) ,"main_thread");
	kthread_bind(thread1, 1);
	ret=sched_setscheduler(thread1, SCHED_FIFO, &param);
	if(ret==-1){
  		printk(KERN_ALERT "sched_setscheduler failed");
		return 1;  
	}
	wake_up_process(thread1);
	hrtimer_start(&hr_timer,interval, HRTIMER_MODE_REL);

    printk(KERN_ALERT "simple module initialized\n");
    return 0;
}

/* exit function - logs that the module is being removed */
static void simple_exit (void) {
	int ret;
	sock_release(socket_ptr->sk_socket);
	hrtimer_cancel(&hr_timer);
 	ret = kthread_stop(thread1);
 	if(!ret)
  		printk(KERN_INFO "Thread stopped");
    printk(KERN_ALERT "simple module is being unloaded\n");
}

module_init (simple_init);
module_exit (simple_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Jiangnan Liu, Qitao Xu, Zhe Wang");
MODULE_DESCRIPTION ("Enforcing Real Time Behavior");
