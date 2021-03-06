#include <linux/fs.h>
#include <linux/init.h>
#include <linux/pid_namespace.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/seqlock.h>
#include <linux/time.h>

#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1-1)) * 100)

static int loadavg_proc_show(struct seq_file *m, void *v)
{
	unsigned long avnrun[3];
	long running, threads;
	struct ve_struct *ve;

	ve = get_exec_env();
	if (ve_is_super(ve)) {
		get_avenrun(avnrun, FIXED_1/200, 0);
		running = nr_running();
		threads = nr_threads;
	} else {
		get_avenrun_ve(avnrun, FIXED_1/200, 0);
		running = nr_running_ve();
		threads = nr_threads_ve(ve);
	}

	seq_printf(m, "%lu.%02lu %lu.%02lu %lu.%02lu %ld/%ld %d\n",
		LOAD_INT(avnrun[0]), LOAD_FRAC(avnrun[0]),
		LOAD_INT(avnrun[1]), LOAD_FRAC(avnrun[1]),
		LOAD_INT(avnrun[2]), LOAD_FRAC(avnrun[2]),
		running, threads,
		task_active_pid_ns(current)->last_pid);
	return 0;
}

static int loadavg_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, loadavg_proc_show, NULL);
}

static const struct file_operations loadavg_proc_fops = {
	.open		= loadavg_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_loadavg_init(void)
{
	proc_create("loadavg", S_ISVTX, NULL, &loadavg_proc_fops);
	return 0;
}
module_init(proc_loadavg_init);
