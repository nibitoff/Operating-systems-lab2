#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>

#define BUF_SIZE 2048
#define PROC_FILE "/proc/file_to_read"

struct user_pci_dev {
    unsigned int	devfn;
    unsigned short	vendor;
    unsigned short	device;
    unsigned short	subsystem_vendor;
    unsigned short	subsystem_device;
};

struct user_dentry{
    unsigned int d_flags;
    unsigned long d_time;
    unsigned long s_blocksize;
    unsigned long inode_ino;
    unsigned char d_iname;
};

void print_dentry(struct user_dentry* dentry){
    printf("----DENTRY INFO----\n");
    printf("d_iname: %u\n", dentry->d_iname);
    printf("d_flags: %u\n", dentry->d_flags);
    printf("d_time: %lu\n", dentry->d_time);
    printf("the root of the dentry tree superblock size: %lu\n", dentry->s_blocksize);
    printf("-------------\n");
}

void print_pci_dev(struct user_pci_dev* u_dev){
    printf("----PCI_DEVICE INFO----\n");
    printf("vendor: %u\n", u_dev->vendor);
    printf("device: %hu\n", u_dev->device);
    printf("devfn: %u\n", u_dev->devfn);
    printf("subsystem_vendor: %hu\n", u_dev->subsystem_vendor);
    printf("subsystem_device: %hu\n", u_dev->subsystem_device);
    printf("-------------\n");
}

int main(int args, char **argv) {
    if (args != 2) {
        printf("Wrong number of arguments: %d. Needed: 1\n", args);
        return 1;
    }

    char input[BUF_SIZE];
    char output[BUF_SIZE];

    sprintf(input, "%s", argv[1]);

    FILE *wfl;
    FILE *rfl;
    wfl = fopen(PROC_FILE, "w");
    if (wfl == NULL) {
        printf("ERROR! Problems with opening proc.");
        return 1;
    }
    fprintf(wfl, "%s", input);

    fclose(wfl);

    rfl = fopen(PROC_FILE, "r");
    if (rfl == NULL) {
        printf("ERROR! Problems with opening proc.");
        return 1;
    }
    char check;
    if (fread(&check, sizeof(char), 1, rfl) == 0 || check != 1) {
        printf("Can't read structure from file. The checking byte is broken!\n");
    } else{
        printf("File found!\n");
        struct user_dentry dentry;
        struct user_pci_dev u_pci;
        if (fread(&dentry, sizeof(struct user_dentry), 1, rfl) == 0) {
            printf("Error! Can't read dentry structure from file\n");
        } else print_dentry(&dentry);
        if (fread(&u_pci, sizeof(struct user_pci_dev), 1, rfl) == 0) {
            printf("Error! Can't read pci_device structure from file\n");
        } else print_pci_dev(&u_pci);
    }
    fclose(rfl);
    return 0;
}