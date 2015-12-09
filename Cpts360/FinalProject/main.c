//
// Created by aschorsch on 11/13/15.
//

#include "mountroot.h"

int main()
{
    char usrname[32];

    // Initialize with mounting the root
    init();
    mount_root();

    // Get the username
    printf("Enter username : ");
    scanf(" %[^\n]s", usrname);

    while (1) {
        shell();
    }
    //printf("result: %d\n", getino(&dev, "/X/"));

    return 0;
}