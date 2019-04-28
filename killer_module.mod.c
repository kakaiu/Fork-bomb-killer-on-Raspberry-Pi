#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xf6bedf2d, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x471916eb, __VMLINUX_SYMBOL_STR(param_ops_ulong) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x4aebb7ae, __VMLINUX_SYMBOL_STR(kthread_stop) },
	{ 0xe5785f57, __VMLINUX_SYMBOL_STR(hrtimer_cancel) },
	{ 0x76a3b03f, __VMLINUX_SYMBOL_STR(sock_release) },
	{ 0x45a280d4, __VMLINUX_SYMBOL_STR(init_net) },
	{ 0xe6932195, __VMLINUX_SYMBOL_STR(hrtimer_start_range_ns) },
	{ 0x60f07451, __VMLINUX_SYMBOL_STR(sched_setscheduler) },
	{ 0xb2815b65, __VMLINUX_SYMBOL_STR(kthread_bind) },
	{ 0xc7c1dbc6, __VMLINUX_SYMBOL_STR(kthread_create_on_node) },
	{ 0xce58dc69, __VMLINUX_SYMBOL_STR(hrtimer_init) },
	{ 0x62744ac5, __VMLINUX_SYMBOL_STR(__netlink_kernel_create) },
	{ 0xf5e34dcb, __VMLINUX_SYMBOL_STR(hrtimer_forward) },
	{ 0x7adeb8d4, __VMLINUX_SYMBOL_STR(ktime_get) },
	{ 0xa20d08e8, __VMLINUX_SYMBOL_STR(wake_up_process) },
	{ 0xdedfaefa, __VMLINUX_SYMBOL_STR(init_pid_ns) },
	{ 0x258dcb9f, __VMLINUX_SYMBOL_STR(init_task) },
	{ 0xe188bdc6, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x65112926, __VMLINUX_SYMBOL_STR(kernel_write) },
	{ 0x70fd6d30, __VMLINUX_SYMBOL_STR(kill_pid) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0xe2d5255a, __VMLINUX_SYMBOL_STR(strcmp) },
	{ 0x76f6c0a0, __VMLINUX_SYMBOL_STR(pid_task) },
	{ 0xfaa8dd31, __VMLINUX_SYMBOL_STR(find_vpid) },
	{ 0x2469810f, __VMLINUX_SYMBOL_STR(__rcu_read_unlock) },
	{ 0x16107fb2, __VMLINUX_SYMBOL_STR(__task_pid_nr_ns) },
	{ 0x8d522714, __VMLINUX_SYMBOL_STR(__rcu_read_lock) },
	{ 0x85df9b6c, __VMLINUX_SYMBOL_STR(strsep) },
	{ 0x11a13e31, __VMLINUX_SYMBOL_STR(_kstrtol) },
	{ 0xd8ca6629, __VMLINUX_SYMBOL_STR(filp_close) },
	{ 0x4647845a, __VMLINUX_SYMBOL_STR(kernel_read) },
	{  0xac04e, __VMLINUX_SYMBOL_STR(filp_open) },
	{ 0x1000e51, __VMLINUX_SYMBOL_STR(schedule) },
	{ 0xb3f7646e, __VMLINUX_SYMBOL_STR(kthread_should_stop) },
	{ 0x12e807b4, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x2196324, __VMLINUX_SYMBOL_STR(__aeabi_idiv) },
	{ 0x2e5810c6, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr1) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xb1ad28e0, __VMLINUX_SYMBOL_STR(__gnu_mcount_nc) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "0C18943D36568CCE07DDB8A");
