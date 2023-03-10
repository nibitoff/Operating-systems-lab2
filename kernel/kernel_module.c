#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/namei.h>
#include <linux/netdevice.h>
#include <linux/path.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/sched.h>
#include <linux/dcache.h>
#include <linux/pci.h>


#define PROC_FILE_MAX_SIZE 256
#define BUF_INFO_MAX_SIZE 1000
#define PROC_FILE "file_to_read"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Danil Sabitov");
MODULE_DESCRIPTION("Kernel module Lab2");
MODULE_VERSION("0.01");

static struct proc_dir_entry *proc_file;
static struct dentry *dentry;

struct user_dentry u_dentry;
struct user_dentry{
    unsigned int d_flags;
    unsigned long d_time;
    unsigned long s_blocksize;
    unsigned long inode_ino;
    unsigned char d_iname;
};

struct user_pci_dev u_pci;
struct user_pci_dev {
    unsigned int	devfn;
    unsigned short	vendor;
    unsigned short	device;
    unsigned short	subsystem_vendor;
    unsigned short	subsystem_device;
};

struct pci_dev *dev;

static ssize_t proc_file_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t proc_file_write(struct file *, const char __user *, size_t, loff_t *);

static struct file_operations proc_file_ops = {
        .read = proc_file_read,
        .write = proc_file_write,
};

static int __init proc_info_mod_init(void) {
    proc_file = proc_create(PROC_FILE, 0777, NULL, &proc_file_ops);
    if (proc_file == NULL) {
        proc_remove(proc_file);
        printk("ERROR: could not initialize /proc/%s\n", PROC_FILE);
        return -ENOMEM;
    }
    printk("Module for lab2 started working!");
    printk("/proc/%s\n", PROC_FILE);
    return 0;
}

static void __exit proc_info_mod_exit(void) {
    remove_proc_entry(PROC_FILE, NULL);
    printk("Goodbye!!! /proc/%s removed.", PROC_FILE);
}

static ssize_t proc_file_read(struct file *file, char __user *user, size_t length, loff_t *offset) {
    loff_t new_len;
    char kern_info[BUF_INFO_MAX_SIZE];
    char check;
    check = 0;
    new_len = 0;

    if (dentry == NULL) {
        printk("Error! Can't find dentry from path.");
    }
    else {
        u_dentry = (struct user_dentry){
                .d_flags = dentry->d_flags,
                .d_time = dentry->d_time,
                .s_blocksize = dentry->d_sb->s_blocksize,
                .inode_ino = dentry->d_inode->i_ino,
                .d_iname = dentry->d_iname[DNAME_INLINE_LEN]
        };
        check = 1;
        memcpy(kern_info, &check, sizeof(char));
        new_len += sizeof(char);
        memcpy(kern_info + new_len, &u_dentry, sizeof(struct user_dentry));
        new_len += sizeof(u_dentry);
        printk("Dentry wrote.");
    }

    dev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, NULL);
    if (dev == NULL) {
        printk(KERN_INFO "Pci device is not found!\n");
    } else {
        printk(KERN_INFO "Pci device found!\n");
        u_pci = (struct user_pci_dev){
                .devfn = dev->devfn,
                .vendor = dev->vendor,
                .device = dev->device,
                .subsystem_vendor = dev->subsystem_vendor,
                .subsystem_device = dev->subsystem_device
        };
        memcpy(kern_info + new_len, &u_pci, sizeof(struct user_pci_dev));
        new_len += sizeof(u_pci);
        printk("Pci_device info wrote.");
}
    pci_dev_put(dev);
    return simple_read_from_buffer(user, new_len, offset, kern_info, BUF_INFO_MAX_SIZE);
}

static ssize_t proc_file_write(struct file *file, const char __user *user, size_t length, loff_t *offset) {
    struct path path;
    int error;
    char path_ch[BUF_INFO_MAX_SIZE] = {0};
    ssize_t try_to_write;

    try_to_write = simple_write_to_buffer(path_ch, BUF_INFO_MAX_SIZE, offset, user, length);
    if (try_to_write < 0){
        printk("ERROR! Failed to write to buffer!");
        return -EFAULT;
        }
    error = kern_path(path_ch, LOOKUP_FOLLOW, &path);
    if (error) {
        printk("PATH ERROR!");
        dentry = NULL;
        return error;
    }
    printk("Path = %s, Path length =  %zu", path_ch, strlen(path_ch));
    dentry = file->f_path.dentry;
    return strlen(path_ch);
}

module_init(proc_info_mod_init);
module_exit(proc_info_mod_exit);