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
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/init_task.h>

#define NETLINK_TEST 17 
#define BUFFER_SIZE 1024
#define PATH_BUFFER_SIZE 50
#define UTIL_THRESHOLD 1250 // 80/100
#define UTIL_PRECISION 1000
#define FORK_NUM_THRESHOLD 200
#define PATH "/home/pi/final_project/force_run"

static unsigned long period_sec = 1;
static unsigned long period_nsec = 0;
static unsigned long test = UTIL_THRESHOLD; //for test
static unsigned long test2 = FORK_NUM_THRESHOLD; //for test

long prev_idle = 0;
long prev_total = 0;

static struct hrtimer hr_timer;
static ktime_t interval;

static struct task_struct *main_thread;

static struct sock *socket_ptr = NULL;

struct sched_param {
	int sched_priority;
};

struct proc_stat {
	int pid_n;
	int num_children;
};

struct proc_stat* children_num_array;
char* read_force_run_buffer;
char* read_proc_info_buffer;
char* read_cmdline_buffer;
char* path_buffer;

module_param(period_sec, ulong, 0);
module_param(period_nsec, ulong, 0);
module_param(test, ulong, 0);
module_param(test2, ulong, 0);

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

/*
input is utilization threshold
return three stats: 
	-1 	func running error
	0	utilization is low
	1	utilization is high
*/
//https://supportcenter.checkpoint.com/supportcenter/portal?eventSubmit_doGoviewsolutiondetails=&solutionid=sk65143
//https://stackoverflow.com/questions/1184274/read-write-files-within-a-linux-kernel-module
static int do_analysis_proc_stat(int threshold) {
	struct file *f;
	mm_segment_t fs;
	long idle = 0;
	long idlecpu = 0;
	long iowait = 0;
	long total = 0;
	long totald = 0;
	long idled = 0;
	long split;
	int i = 0;
	int ret;
	char* token;
	char *cur;

	for (i=0; i<BUFFER_SIZE; i++) {
		read_proc_info_buffer[i] = '\0';
	}

	f = filp_open("/proc/stat", O_RDONLY, 0);
	if(IS_ERR(f)){
		printk(KERN_ALERT "do_analysis_proc_stat filp_open error!!");
		filp_close(f, NULL);
		return -1;
	} else {
		fs = get_fs();
		set_fs(get_ds());
		kernel_read(f, read_proc_info_buffer, BUFFER_SIZE, &f->f_pos);
		set_fs(fs);
		filp_close(f, NULL);

		cur = read_proc_info_buffer;
		i = 0;
		while( (token = strsep(&cur, "  ")) != NULL &&i<9){
			if(i==0||i==1){
				i++;
				continue;
			} else {
				//printk(KERN_INFO "token: %s %d", token, i);
				ret = kstrtol(token, 10, &split);
				if(ret!=0) {
					printk(KERN_ALERT "Conversion1 error!!");
					return -1;
				}
				total = total + split;

				if(i==5){ //idle cpu time
					ret = kstrtol(token, 10, &idlecpu);
					if(ret!=0) {
						printk(KERN_ALERT "Conversion2 error!!");
						return -1;
					}
				}
				if(i==6){ //i/o wait
					ret = kstrtol(token, 10, &iowait);
					if(ret!=0) {
						printk(KERN_ALERT "Conversion3 error!!");
						return -1;
					}
				}
				i++;
			}
		}
		idle = idlecpu + iowait;
		totald = total - prev_total;
		idled = idle - prev_idle;

		printk(KERN_INFO "utilization = %lu/%lu", totald-idled, totald);

		prev_total = total;
		prev_idle = idle;

		if ((totald-idled)==0) {
			return 0;
		} else {
			if ((totald)*UTIL_PRECISION/(totald-idled) < threshold) {
				return 1;
			} else {
				return 0;
			}	
		}
	}
}

/*
return pid for any potential bomb
return -1 for no bomb 
*/
//https://blog.csdn.net/qq_24521983/article/details/53582462
static int find_potential_fork_bomb(int threshold) {
	struct task_struct *task, *p;
	struct list_head *pos;
	int count = 0;
	int tmp;
	int i;
	int uid_n = -1;
	int pid_n = -1;
	
	for (i=0; i<BUFFER_SIZE; i++) { //refresh
		children_num_array[i].pid_n = -1;
		children_num_array[i].num_children = 0;
	}
	task = &init_task;
	list_for_each(pos, &task->tasks) {
		p = list_entry(pos, struct task_struct, tasks);
		count++;
		while (1) { //go through ancestors
			tmp = task_ppid_nr(p);
			if (tmp==0) {
				break;
			} else {
				p = pid_task(find_vpid(tmp), PIDTYPE_PID); //only consider the node from its parent to root
				uid_n = __kuid_val(task_uid(p));
				pid_n = p->pid;
				if (uid_n==0) {
					continue; //Administrator and do nothing
				} else if (uid_n>=1000) {
					if (strcmp(p->comm, "systemd")==0) {
						continue; //system service daemon and do nothing
					} else if (strcmp(p->comm, "lxsession")==0) {
						continue; //system service daemon and do nothing
					} else if (strcmp(p->comm, "lxpanel")==0) {
						continue; //system service daemon and do nothing
					} else if (strcmp(p->comm, "lxterminal")==0) {
						continue; //system service daemon and do nothing
					} else if (strcmp(p->comm, "pcmanfm")==0) {
						continue; //system service daemon and do nothing
					} else if (strcmp(p->comm, "openbox")==0) {
						continue; //system service daemon and do nothing
					} else {
						//printk("%d-->%d: %s (%d)", task_ppid_nr(p), pid_n, p->comm, uid_n);
						for (i=0; i<BUFFER_SIZE; i++) {
							if (children_num_array[i].num_children!=0) {
								if (pid_n==children_num_array[i].pid_n) {
									children_num_array[i].num_children++;
									if (children_num_array[i].num_children>threshold) {
										return children_num_array[i].pid_n; //return bomb_pid
									}
									break;
								}
							} else {
								children_num_array[i].pid_n = pid_n;
								children_num_array[i].num_children++;
								break;
							}
						}
					}
				} else {
					printk(KERN_ALERT "Unknown User: %d", uid_n);
					printk(KERN_ALERT "%d-->%d: %s (%d)\n", task_ppid_nr(p), pid_n, p->comm, uid_n);
				}
			}
		}
	}
	//test
	/*for (i=0; i<BUFFER_SIZE; i++) {
		if (children_num_array[i].num_children==0) {
			break;
		} 
		printk("%d has %d children", children_num_array[i].pid_n, children_num_array[i].num_children);
	}
	printk("the number of process is:%d",count);*/
	return -1;
}

/*
is force_run return 1
else return 0
*/
static int check_if_force_run(char* input, char * config) {
	char* token;
	while( (token = strsep(&config, " ")) != NULL) {
		if (strcmp(token, input)==0) {
			return 1;
		}
	}
	return 0;
}

static char* get_cmdline_by_pidn(int pid_n) {
	struct file *f;
	mm_segment_t fs;
	char* cur = NULL;
	int i = 0;
	for (i=0; i<PATH_BUFFER_SIZE; i++) { //refresh
		path_buffer[i] = '\0';
	}

	sprintf(path_buffer, "/proc/%d/cmdline", pid_n);
	printk(KERN_INFO "read cmdline from %s", path_buffer);
	f = filp_open(path_buffer, O_RDONLY, 0);
	if(IS_ERR(f)){
		printk(KERN_ALERT "killer filp_open error!!");
		filp_close(f, NULL);
		return NULL;
	} else {
		fs = get_fs();
		set_fs(get_ds());
		kernel_read(f, read_cmdline_buffer, PATH_BUFFER_SIZE, &f->f_pos);
		set_fs(fs);
		filp_close(f, NULL);
		cur = read_cmdline_buffer;
		return cur;
	}
}

/*
Do not directly use f->ops->read and use kernel_read instead, see
https://stackoverflow.com/questions/1184274/read-write-files-within-a-linux-kernel-module
*/
static int do_kill_processes(void) {
	struct file *f;
	mm_segment_t fs;
	char *cur = NULL;
	int i = 0;
	int bomb_pid = -1;
	char *bomb_cmdline = NULL;

	for (i=0; i<BUFFER_SIZE; i++) {
		read_force_run_buffer[i] = '\0';
	}

	f = filp_open(PATH, O_RDONLY, 0); //read config
	if(IS_ERR(f)){
		printk(KERN_ALERT "killer filp_open error!!");
		filp_close(f, NULL);
		return -1;
	} else {
		fs = get_fs();
		set_fs(get_ds());
		kernel_read(f, read_force_run_buffer, BUFFER_SIZE, &f->f_pos);
		set_fs(fs);
		filp_close(f, NULL);
		cur = read_force_run_buffer;
		printk(KERN_INFO "force_run procs are: %s", cur);
		bomb_pid = find_potential_fork_bomb(test2);
		if (bomb_pid==-1) {
			printk("no bomb");
			return 0; //no bomb and do nothing
		} else {
			printk("bomb pid is %d", bomb_pid);
			bomb_cmdline = get_cmdline_by_pidn(bomb_pid);
			if (bomb_cmdline==NULL) {
				printk(KERN_ALERT "no cmdline");
				return -1;
			} else {
				printk("cmdline is %s", bomb_cmdline);
				if (check_if_force_run(bomb_cmdline, cur)==1) {
					printk(KERN_ALERT "force_run, can not kill");
					//TODO: write report
				} else {
					printk(KERN_ALERT "not force_run, kill it");
					kill_pid(find_vpid(bomb_pid), SIGTERM, 1);
				}
			}
		}
		return 0;
	}
}

static int thread_fn(void * data) {
	int system_util_stat = -1;
	children_num_array = (struct proc_stat*) kmalloc_array(BUFFER_SIZE, sizeof(struct proc_stat), GFP_KERNEL);
	read_force_run_buffer = (char*) kmalloc_array(BUFFER_SIZE, sizeof(char), GFP_KERNEL);
	read_proc_info_buffer = (char*) kmalloc_array(BUFFER_SIZE, sizeof(char), GFP_KERNEL);
	read_cmdline_buffer = (char*) kmalloc_array(BUFFER_SIZE, sizeof(char), GFP_KERNEL);
	path_buffer = (char*) kmalloc_array(BUFFER_SIZE, sizeof(char), GFP_KERNEL);
	while (!kthread_should_stop()){ 
		set_current_state(TASK_INTERRUPTIBLE);
  		schedule();
		system_util_stat = do_analysis_proc_stat(test);
		if (system_util_stat==-1) { //func running error
			continue;
		} else if (system_util_stat==0) {
			printk(KERN_INFO "low utilization");
			continue;
		} else if (system_util_stat==1) {
			printk(KERN_INFO "high utilization");
			if (do_kill_processes()==-1) {
				return 1;
			} else {
				continue;
			}
		} else {
			printk(KERN_ALERT "wrong system_util_stat: %d", system_util_stat);
		}
	}
	return 0;
}

/*timer expiration*/
static enum hrtimer_restart timer_callback(struct hrtimer *timer_for_restart) {
	ktime_t currtime;
	wake_up_process(main_thread);
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
	interval=ktime_set(period_sec,period_nsec);
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer.function=&timer_callback;

	main_thread = kthread_create(thread_fn, NULL ,"main_thread");
	kthread_bind(main_thread, 1);
	ret=sched_setscheduler(main_thread, SCHED_FIFO, &param);
	if(ret==-1){
  		printk(KERN_ALERT "sched_setscheduler failed");
		return 1;  
	}
	wake_up_process(main_thread);
	hrtimer_start(&hr_timer,interval, HRTIMER_MODE_REL);

    printk(KERN_ALERT "simple module initialized");
    return 0;
}

/* exit function - logs that the module is being removed */
static void simple_exit (void) {
	int ret;
	sock_release(socket_ptr->sk_socket);
	hrtimer_cancel(&hr_timer);
 	ret = kthread_stop(main_thread);
 	if(!ret) {
  		printk(KERN_INFO "Thread stopped");
 	}
  	kfree(children_num_array);
	kfree(read_force_run_buffer);
	kfree(read_proc_info_buffer);
	kfree(read_cmdline_buffer);
	kfree(path_buffer);
    printk(KERN_ALERT "simple module is being unloaded");
}

module_init (simple_init);
module_exit (simple_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Zhe Wang, Jiangnan Liu, Qitao Xu");
MODULE_DESCRIPTION ("Fork Bomb Killer");
