/**
 *  procfs4.c -  create a "file" in /proc
 * 	This program uses the seq_file library to manage the /proc file.
 *
 */

#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */
#include <linux/proc_fs.h>	/* Necessary because we use proc fs */
#include <linux/seq_file.h>	/* for seq_file */
#include <linux/uaccess.h>

#define PROC_NAME	"throttlectl"
#define MAX_SIZE  1024

MODULE_AUTHOR("Philippe Reynes");
MODULE_LICENSE("GPL");

static char wrbuffer[MAX_SIZE];
static unsigned long wrbuffer_size = 0;

int readV(void){
	int val = 0; 
	asm (
		".intel_syntax noprefix\n"
		"mov ecx, 508\n"
		"rdmsr\n"
		"mov %0, eax\n"
		".att_syntax noprefix\n"
		:"=r" ( val ));
	return val;
}

void writeV(int val){
	asm volatile (
		".intel_syntax noprefix\n"
		"mov ecx, 508\n"
		"mov edx, 0\n"
		"mov eax, %0\n"
		"wrmsr\n"
		".att_syntax noprefix\n"
		:
		: "r" (val));
}

static void *my_seq_start(struct seq_file *s, loff_t *pos){
	static unsigned long counter = 0;

	if ( *pos == 0 ) {	
		return &counter;
	}
	else{
		*pos = 0;
		return NULL;
	}
}

static void *my_seq_next(struct seq_file *s, void *v, loff_t *pos){
	unsigned long *tmp_v = (unsigned long *)v;
	(*tmp_v)++;
	(*pos)++;
	return NULL;
}

static void my_seq_stop(struct seq_file *s, void *v){
}

static int my_seq_show(struct seq_file *s, void *v){
	int val;
	val = readV();
	seq_printf(s, "%d\n", val);

	return 0;
}

static struct seq_operations my_seq_ops = {
	.start = my_seq_start,
	.next  = my_seq_next,
	.stop  = my_seq_stop,
	.show  = my_seq_show
};

static int my_open(struct inode *inode, struct file *file){
	return seq_open(file, &my_seq_ops);
};

void parseInstruction(char *wrbuffer, int size){
	char command[10];
	int val;
	sscanf(wrbuffer, "%s", command);
	if(strcmp(command, "thon") == 0){
		val = readV();
		val = val | 0x1;
		writeV(val);
	}
	else if(strcmp(command, "thoff") == 0){
		val = readV();
		val = val & 0xfffffffe;
		writeV(val);
	}
}
ssize_t procfile_write(struct file *file, const char *buffer, unsigned long count,
		   loff_t *data){

	/* get buffer size */
	wrbuffer_size = count;
	if (wrbuffer_size > MAX_SIZE ) {
		wrbuffer_size = MAX_SIZE;
	}
	/* write data to the buffer */
	if ( copy_from_user(wrbuffer, buffer, wrbuffer_size) ) {
		return -EFAULT;
	}

	parseInstruction(wrbuffer, wrbuffer_size);
	return wrbuffer_size;
}

static struct file_operations my_file_ops = {
	.owner   = THIS_MODULE,
	.open    = my_open,
	.read    = seq_read,
	.write 	 = procfile_write,
	.llseek  = seq_lseek,
	.release = seq_release
};
	
int init_module(void){
	struct proc_dir_entry *entry;

	entry = proc_create(PROC_NAME, 0, NULL, &my_file_ops);
	
	return 0;
}

void cleanup_module(void){
	remove_proc_entry(PROC_NAME, NULL);
}