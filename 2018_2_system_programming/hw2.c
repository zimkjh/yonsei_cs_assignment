#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/moduleparam.h>
#include <linux/sysinfo.h>
#include <linux/time.h>



#include <linux/init.h>
#include <linux/timekeeping.h>
#include <linux/fs.h>
#include <linux/kernel_stat.h>
#include <linux/cputime.h>
#include <linux/ktime.h>

#include <linux/slab.h>
#include <linux/err.h>
#include <linux/ptrace.h>

#include <asm/pgtable_64_types.h>
#include <asm/pgtable_types.h>

#define PROC_NAME "hw2"
#define STUDENT_NAME "Kim Jo Hyun"
#define STUDENT_ID "2016147571"

static int period = 0;


module_param(period, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(period, "An integer"); //get period parameter
void my_tasklet_function(int data) // tasklet function

{ 
	printk("tasklet1");

 return ;

}


DECLARE_TASKLET( my_tasklet, my_tasklet_function, 2);

void procs_info_print(void)
{
}
void printf_bar(struct seq_file *s)
{
	int i;
	for(i=0; i<71; i++)
	{
		seq_printf(s, "*");
	}
	seq_printf(s, "\n"); 
}
static void *my_seq_start(struct seq_file *s, loff_t *pos) //copy from assignment 1
{
	static unsigned long counter = 0;
	if(*pos == 0)
	{
		return &counter;
	}
	else
	{
		*pos=0;
		return NULL;
	}
}
static void *my_seq_next(struct seq_file *s, void *v, loff_t *pos) //copy from assignment 1
{
	unsigned long *tmp_v = (unsigned long *)v;
	(*tmp_v)++;
	(*pos)++;
	return NULL;
}
static void my_seq_stop(struct seq_file *s, void *v)
{
}


void my_seq_show(struct seq_file *s, void *v){

	struct task_struct* task_list;
	struct sysinfo sysinfo_local;



	printk("period : %d\n", period); // check period paramter came
	int task_num = 0;
	for_each_process(task_list) // count process number (excluding kernel thread process)
	{
		if(task_list->parent->pid == 1 || task_list->pid == 1){
			
			task_num+=1 ;
		}
	}
	printk("taksk_num : %d\n", task_num);

//-----------uptime
	struct timespec idle;
	u64 idletime;
	u64 nsec;
	u32 rem;
	int i;

	idletime = 0;
	for_each_possible_cpu(i)
		idletime += (__force u64) kcpustat_cpu(i).cpustat[CPUTIME_IDLE];

	nsec = cputime64_to_jiffies64(idletime) * TICK_NSEC;
	idle.tv_sec = div_u64_rem(nsec, NSEC_PER_SEC, &rem);
	idle.tv_nsec = rem;
////////uptime

	int random_num = idle.tv_sec % task_num;//get random number using uptime
	printk("rand : %d\n", random_num); 

	struct task_struct *p = &init_task;
	int temp = 0;
	while(1){ // get random task_struct
		p = next_task(p);
		if(p->parent->pid == 1 || p->pid == 1){
			temp+=1;
			printk("pid : %d\n", p->pid); 
			if(temp == random_num-2){
				break;
			}
		}
	}
	task_list = p; // get the task_list we will use.
	printk("%lu : \n", task_list -> pid);
	printk("rand done : %lu\n", task_list->pid);
	

    printf_bar(s);
    seq_printf(s, "Student ID: %s    Name: %s\n", STUDENT_ID, STUDENT_NAME);
    seq_printf(s, "Virtual Memory Address Information\n");
    seq_printf(s, "Process (%15s:%lu)\n", "vi", task_list->pid);


	seq_printf(s, "Last update time %lu%03lu ms\n", // print uptime
			(unsigned long) idle.tv_sec,
			(idle.tv_nsec / (NSEC_PER_SEC / 1000)));
    printf_bar(s);

    // print info about each area

    seq_printf(s, "0x%08lx - 0x%08lx : Code Area, %lu page(s)\n",task_list->mm->mmap->vm_start, task_list->mm->mmap->vm_end, (task_list->mm->mmap->vm_end - task_list->mm->mmap->vm_start) / PAGE_SIZE);
    seq_printf(s, "0x%08lx - 0x%08lx : Data Area, %lu page(s)\n",task_list->mm->mmap->vm_next->vm_start, task_list->mm->mmap->vm_next->vm_end, (task_list->mm->mmap->vm_next->vm_end - task_list->mm->mmap->vm_next->vm_start)/ PAGE_SIZE);
    seq_printf(s, "0x%08lx - 0x%08lx : BSS Area, %lu page(s)\n",task_list->mm->mmap->vm_next->vm_next->vm_start, task_list->mm->mmap->vm_next->vm_next->vm_end, ( task_list->mm->mmap->vm_next->vm_next->vm_end - task_list->mm->mmap->vm_next->vm_next->vm_start)/ PAGE_SIZE);
    seq_printf(s, "0x%08lx - 0x%08lx : Heap Area, %lu page(s)\n",task_list->mm->start_brk, task_list->mm->brk, ( task_list->mm->brk - task_list->mm->start_brk)/PAGE_SIZE);


	struct vm_area_struct* ss; 
	ss = task_list->mm->mmap->vm_next->vm_next->vm_next->vm_next;
	unsigned long sum = 0, sum_s, sum_e;
	sum_s = ss->vm_start;
	sum_e = ss->vm_start;

	while(ss->vm_end <= task_list->mm->mmap_base){ // to get end of the shared library
		 
		sum += (ss->vm_end - ss->vm_start);
		sum_e = ss->vm_end;
		ss = ss->vm_next;
	}

    seq_printf(s, "0x%08lx - 0x%08lx : Shared Libraries Area, %lu page(s)\n",sum_s, sum_e, (sum_e-sum_s)/PAGE_SIZE);
    seq_printf(s, "0x%08lx - 0x%08lx : Stack Area, %lu page(s)\n",
            task_list->mm->start_stack, task_list->mm->start_stack - task_list->mm->stack_vm * PAGE_SIZE, task_list->mm->stack_vm);

    // 1 level paging (PGD Info)
    printf_bar(s);
    seq_printf(s, "1 Level Paging: Page Directory Entry Information \n");
    printf_bar(s);
    
    seq_printf(s, "PGD     Base Address            : 0x%08lx\n", task_list->mm->pgd);
	unsigned long baseAddr = task_list->mm->pgd;


	seq_printf(s, "code    PGD Address             : 0x%08lx \n", pgd_offset(task_list->mm, task_list->mm->mmap->vm_start));
	seq_printf(s, "        PGD Value               : 0x%08lx \n", pgd_val(*pgd_offset(task_list->mm, task_list->mm->mmap->vm_start)));
	seq_printf(s, "        +PFN Address            : 0x%08lx \n", pgd_val(*pgd_offset(task_list->mm, task_list->mm->mmap->vm_start)) / 0x1000);

    
    seq_printf(s, "        +Page Size              : %dKB\n", PAGE_SIZE/1024); /// PRINT BY STRING?

	unsigned long ptee = pgd_val(*pgd_offset(task_list->mm, task_list->mm->mmap->vm_start));
	//calculate binary bits from the last 3 numbers of pgd value
	long ptee_2 = (ptee-(ptee/0x1000*0x1000));
	ptee_2 = 0x025;
	long ptee_1 = (ptee_2 - (ptee_2/0x100 * 0x100));
	long ptee_0 = ptee_1 - (ptee_1/0x10 * 0x10);
	long p_2 = ptee_2/0x100;
	long p_1 = ptee_1/0x10;
	long p_0 = ptee_0;

	seq_printf(s, "        +Accessed Bit           : %d \n", (p_1-(p_1/4)*4) /2);
	if((p_1-(p_1/2)*2) == 1){
		seq_printf(s, "        +Cache Disable Bit      : true\n" );
	}else{
		seq_printf(s, "        +Cache Disable Bit      : false\n" );
	}
	if( (p_0/8) ==1){
		seq_printf(s, "        +Page Write-Through     : write-through\n");
	}else{
		seq_printf(s, "        +Page Write-Through     : write-back\n");
	}
	if((p_0-(p_0/8)*8) /4 ==1){
		seq_printf(s, "        +User/Supervisor Bit    : user\n");
	}else{
		seq_printf(s, "        +User/Supervisor Bit    : supervisor\n");
	}
	if((p_0-(p_0/4)*4) /2 ==1){
		seq_printf(s, "        +Read/Write Bit         : read/write \n");
	}else{
		seq_printf(s, "        +Read/Write Bit         : read-only\n");
	}
	seq_printf(s, "        +Page Present Bit       : %d \n", (p_0-(p_0/2)*2) );


    // 2 level paging (PUD Info)
    printf_bar(s);
    seq_printf(s, "2 Level Paging: Page Upper Directory Entry Information \n");
    printf_bar(s);
    
    seq_printf(s, "code    PUD Address             : 0x%08lx\n",  pud_offset(pgd_offset(task_list->mm, task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start));
    seq_printf(s, "        PUD Value               : 0x%08lx\n", pud_val(*pud_offset(pgd_offset(task_list->mm, task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start)));
    seq_printf(s, "        +PFN Address            : 0x%08lx\n", pud_val(*pud_offset(pgd_offset(task_list->mm, task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start))/0x1000);
    
    // 3 level paging (PMD Info)
    printf_bar(s);
    seq_printf(s, "3 Level Paging: Page Middle Directory Entry Information \n");
    printf_bar(s);

    seq_printf(s, "code    PMD Address             : 0x%08lx\n", pmd_offset(pud_offset(pgd_offset(task_list->mm, task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start));
    seq_printf(s, "        PMD Value               : 0x%08lx\n", pmd_val(*pmd_offset(pud_offset(pgd_offset(task_list->mm, task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start)));
    seq_printf(s, "        +PFN Address            : 0x%08lx\n", pmd_val(*pmd_offset(pud_offset(pgd_offset(task_list->mm, task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start))/0x1000);

    // 4 level paging (PTE Info)
    printf_bar(s);
    seq_printf(s, "4 Level Paging: Page Table Entry Information \n");
    printf_bar(s);

    seq_printf(s, "code    PTE Address             : 0x%08lx\n", pte_offset_kernel(pmd_offset(pud_offset(pgd_offset(task_list->mm, task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start));
    seq_printf(s, "        PTE Value               : 0x%08lx\n", pte_val(*pte_offset_kernel(pmd_offset(pud_offset(pgd_offset(task_list->mm, task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start)));
    seq_printf(s, "        +Page Base Address      : 0x%08lx\n", pte_val(*pte_offset_kernel(pmd_offset(pud_offset(pgd_offset(task_list->mm, task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start))/0x1000);    


	ptee = pte_val(*pte_offset_kernel(pmd_offset(pud_offset(pgd_offset(task_list->mm, baseAddr), baseAddr), baseAddr), baseAddr));
	ptee_2 = (ptee-(ptee/0x1000*0x1000));
	ptee_2 = 0x025;
	ptee_1 = (ptee_2 - (ptee_2/0x100 * 0x100));
	ptee_0 = ptee_1 - (ptee_1/0x10 * 0x10);
	p_2 = ptee_2/0x100;
	p_1 = ptee_1/0x10;
	p_0 = ptee_0;

	seq_printf(s,  "        +Dirty Bit              : %lx \n", (p_1-(p_1/8)*8) /4 );
	seq_printf(s, "        +Accessed Bit           : %lx\n", (p_1-(p_1/4)*4) /2);
	if((p_1-(p_1/2)*2) == 1){
		seq_printf(s,  "        +Cache Disable Bit      : true\n" );
	}else{
		seq_printf(s,  "        +Cache Disable Bit      : false\n" );
	}
	if( (p_0/8) ==1){
		seq_printf(s, "        +Page Write-Through     : write-through\n");
	}else{
		seq_printf(s, "        +Page Write-Through     : write-back\n");
	}
	if((p_0-(p_0/8)*8) /4 ==1){
		seq_printf(s, "        +User/Supervisor        : user\n");
	}else{
		seq_printf(s, "        +User/Supervisor        : supervisor\n");
	}
	if((p_0-(p_0/4)*4) /2 ==1){
		seq_printf(s, "        +Read/Write Bit         : read/write \n");
	}else{
		seq_printf(s, "        +Read/Write Bit         : read-only\n");
	}
	seq_printf(s, "        +Page Present Bit       : %d \n", (p_0-(p_0/2)*2) );
			

    printf_bar(s);
    seq_printf(s, "Start of Physical Address       : 0x%08lx\n",( pte_val(*pte_offset_kernel(pmd_offset(pud_offset(pgd_offset(task_list->mm, task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start))/0x1000)*0x1000);
    printf_bar(s);
	unsigned long virt = phys_to_virt(pte_val(*pte_offset_kernel(pmd_offset(pud_offset(pgd_offset(task_list->mm, task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start), task_list->mm->mmap->vm_start)));
    seq_printf(s, "Start of Virtual Address        : 0x%08lx\n", virt - (virt/0x100000000)*0x100000000);
    printf_bar(s);


	return 0;
}
//from hw1
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
static struct file_operations my_file_ops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};
int init_module(void)
{
	struct proc_dir_entry *entry;
	entry = proc_create(PROC_NAME,0, NULL,&my_file_ops);
	printk(KERN_INFO "[ INIT ==\n");
	procs_info_print();
	return 0;
}
void cleanup_module(void)
{
	remove_proc_entry(PROC_NAME, NULL);
}

