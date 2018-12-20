#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#define PROC_NAME	"hw1"
static void *my_seq_start(struct seq_file *s, loff_t *pos)
{
	static unsigned long counter =0;
	if(*pos==0)
	{
		return &counter;
	}
	else
	{
		*pos=0;
		return NULL;
	}
}
static void *my_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	unsigned long *tmp_v = (unsigned long *)v;
	(*tmp_v)++;
	(*pos)++;
	return NULL;
}
static void my_seq_stop(struct seq_file *s, void *v)
{
}
static int my_seq_show(struct seq_file *s, void *v)
{
	size_t process_counter;
	struct task_struct* task_list;
	int i, i2;
	void getnstimeofday(struct timespec *ts);
	long int tempint1;
	long int tempint2, ts;
	long int ti1, ti2, ti3, ti11,ti22,ti33;
	long int init=0, cat=0, init_n=0, cat_n=0, uptime;
	for (i=0; i<120; i++){
		seq_printf(s, "-");
	}
	//count the number of process
	process_counter = 0;
	for_each_process(task_list){
		++process_counter;
	}
	seq_printf(s, "\nCURRENT SYSTEM INFORMATION >\nTotal %zu task\n", process_counter);
	//get second and nanosecond respectively
	for_each_process(task_list){
		init = task_list->start_time.tv_sec;
		init_n = task_list->start_time.tv_nsec;
		break;
	}
	for_each_process(task_list){
		cat = task_list->start_time.tv_sec;
		cat_n = task_list->start_time.tv_nsec;
	}
	uptime = (cat - init)*1000 + (cat_n-init_n)/1000000;
	seq_printf(s, "%dHz, %ld ms after system boot time\n", task_list->time_slice, uptime);	

	
	for (i2=0; i2<120; i2++){
		seq_printf(s, "-");
	}
	seq_printf(s, "\n%19s%8s%13s%13s%13s%13s\n", "command", "pid", "start(s)", "total(s)", "user(s)", "kernel(s)");
	for (i2=0; i2<120; i2++){
		seq_printf(s, "-");
	}
	seq_printf(s,"\n");
	// process stats
	for_each_process(task_list){
		tempint1 = task_list->start_time.tv_nsec/1000000;
		tempint2 = task_list->utime + task_list->stime;
		ts = task_list->time_slice; //time slice to ts
		ti1 = tempint2 / ts;
		ti2 = task_list->utime / ts;
		ti3 = task_list->stime / ts;
		ti11 = (tempint2 - ti1*ts)*1000/ts;
		ti22 = (task_list->utime - ti2*ts)*1000/ts;
		ti33 = (task_list->stime - ti3*ts)*1000/ts;
		seq_printf(s, "%19s%8d%9lld.%03ld%9ld.%03ld%9ld.%03ld%9ld.%03ld\n", task_list->comm, task_list->pid, (long long)task_list->start_time.tv_sec,tempint1,ti1,ti11, ti2,ti22, ti3,ti33);
	}

	return 0;
}
static struct seq_operations my_seq_ops = {
	.start = my_seq_start,
	.next = my_seq_next,
	.stop = my_seq_stop,
	.show = my_seq_show
};
static int my_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &my_seq_ops);
};
static struct file_operations my_file_ops ={
	.owner = THIS_MODULE,
	.open = my_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};
int init_module(void)
{	
	struct proc_dir_entry *entry;
	entry = create_proc_entry(PROC_NAME, 0644, NULL);
	if(entry)
	{
		entry->proc_fops =  &my_file_ops;
	}
	return 0;
}
void cleanup_module(void)
{
	remove_proc_entry(PROC_NAME, NULL);
}
