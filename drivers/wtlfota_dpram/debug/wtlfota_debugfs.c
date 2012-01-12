/****************************************************************************
**
** COPYRIGHT(C) : Samsung Electronics Co.Ltd, 2006-2010 ALL RIGHTS RESERVED
**
** AUTHOR       : Song Wei  			@LDK@
** DESCRIPTION: wtlfota_debugfs.c: debugfs support. This is the major interface for the userspace to interact with wtlfota_dpram driver
*/


#include <asm/uaccess.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include "wtlfota_dpram_cmd.h"
#define CMD_SIZE 100

struct debug_command_info{
  ///input parameter: the string written to the command file in debugfs
  char  command [CMD_SIZE];
  /// command string used to match input (string written to the command file in debugfs)
  void (*exec_command)(char *);
  char description [CMD_SIZE];
};

static struct dentry *debugfs_wtlfota;
static struct dentry *debugfs_wtlfota_dpram;


static void echo_cmd(char *input);
//static void write_magic_cmd(char *input);

const int CMD_LIST_LEN = 3;
struct debug_command_info CMD_LIST[] = {
  {"echo", echo_cmd, "echo: printk echo\n"},
  {"reset", reset_cmd, "reset: reset phone\n"},
  //{"write_magic", write_magic_cmd, "write_magic: write magic bytes to dpram\n"},
};

static void echo_cmd(char *input){
  printk("wtlfota debugfs:%s\n", input);
}


/* static void write_magic_cmd(char *input){ */
/*   static unsigned long magic; */
/*   int ret; */
/*   printk("writing :%lu ", magic); */
/*   ret = WRITE_TO_WTLFOTA_DPRAM_VERIFY(0, (void *)(&magic), sizeof(magic)); */
/*   if(ret != 0){ */
/*     printk("failed!\n"); */
/*   }else{ */
/*     printk("succeeded!\n"); */
/*   } */
/*   magic++; */
/* } */



static int wtlfota_debugfs_open(struct inode *inode, struct file *file)
{
  file->private_data = inode->i_private;
  return 0;
}


static ssize_t read_file_command(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
  char buf[CMD_SIZE * CMD_LIST_LEN];
  unsigned int len = 0;
  int i=0;
  for(i=0; i<CMD_LIST_LEN; i++){
    len += snprintf(buf+len, sizeof(buf)-len, "%s",
		    CMD_LIST[i].description);
  }
  return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_command(struct file *file,
				  const char __user *userbuf,
				  size_t count, loff_t *ppos)
{
  char buf[CMD_SIZE];

  int i=0;
  if (copy_from_user(buf, userbuf, min(count, sizeof(buf))))
    return -EFAULT;
  for(i=0; i<CMD_SIZE && buf[i] != 10 && buf[i]!=' ' && buf[i]!=0; i++){
    ;
  }
  buf[i]=0;

  for(i=0; i<CMD_LIST_LEN; i++){
    if (strcmp(buf, CMD_LIST[i].command) == 0) {
      //dprintk("fire!\n");
      CMD_LIST[i].exec_command((char *)&buf);
    }
  }

  return count;
}

static const struct file_operations fops_command = {
  .read = read_file_command,
  .write = write_file_command,
  .open = wtlfota_debugfs_open,
  .owner = THIS_MODULE,
};


void debugfs_init_wtlfota(void)
{
  printk("debugfs_init_wtlfota\n");
  debugfs_wtlfota = debugfs_create_dir("wtlfota", NULL);
  debugfs_wtlfota_dpram  = debugfs_create_file("dpram", S_IWUSR | S_IRUGO,
							    debugfs_wtlfota, NULL, &fops_command);
}


void debugfs_finish_wtlfota(void)
{
  debugfs_remove(debugfs_wtlfota_dpram);
  debugfs_remove(debugfs_wtlfota);
}
