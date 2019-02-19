#include <stdio.h>
#include <dirent.h>         /* opendir, readdir, closedir */ 
#include <string.h>         /* strerror() */
#include <errno.h>          /* errno */
#include <iostream>
#include <fcntl.h>          /* open() */
#include <unistd.h>         /* close() */
#include <sys/ioctl.h>      /* ioctl() */
#include <linux/input.h>    /* EVIOCGVERSION ++ */
#include <linux/input-event-codes.h>

#define EV_BUF_SIZE 	16

#define KEY_PRESS 0x01
#define KEY_RELEASE 0x00

using namespace std;

bool caps = false;
 
char scancodes[] = {00, 27, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 45, 61, 8, 9, 113, 119, 101, 114, 116, 121, 117, 105, 111, 112, 91, 93, 13, 00, 97, 115, 100, 102, 103, 104, 106, 107, 108, 59, 39, 96, 00, 92, 122, 120, 99, 118, 98, 110, 109, 44, 46, 47, 00, 00, 00, 32, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 55, 56, 57, 45, 52, 53, 54, 43, 49, 50, 51, 48, 46};
char capscodes[] = {00, 27, 33, 64, 35, 36, 37, 94, 38, 42, 40, 41, 95, 43, 8, 15, 81, 87, 69, 82, 84, 89, 85, 73, 79, 80, 123, 125, 13, 00, 65, 83, 68, 70, 71, 72, 74, 75, 76, 58, 34, 126, 00, 124, 90, 88, 67, 86, 66, 78, 77, 60, 62, 63, 00, 00, 00, 32, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 127};
char *devices[24];
int dev = 0;

string getDeviceInfo(int _fd)
{
    /* A few examples of information to gather */
    string deviceInfo;
    char tempInfo[512];
    unsigned version;
    unsigned short id[4];     /* or use struct input_id */
    char name[256] = "N/A";
    
    /* Error check here as well. */
    ioctl(_fd, EVIOCGVERSION, &version);
    ioctl(_fd, EVIOCGID, id); 
    ioctl(_fd, EVIOCGNAME(sizeof(name)), name);

    sprintf(tempInfo,
        "----------\n"
        "Name      : %s\n"
        "Version   : %d.%d.%d\n"
        "ID        : Bus=%04x Vendor=%04x Product=%04x Version=%04x\n"
        "----------\n"
        ,
        name,

        version >> 16,
        (version >> 8) & 0xff,
        version & 0xff,

        id[ID_BUS],
        id[ID_VENDOR],
        id[ID_PRODUCT],
        id[ID_VERSION]
    );
    deviceInfo = tempInfo;
    return deviceInfo;
}

int main(int argc, char *argv[])
{
    int fd, sz, deviceNumber;
    unsigned i;

    /* A few examples of information to gather */
    //unsigned version;
    //unsigned short id[4];     /* or use struct input_id */
    //char name[256] = "N/A";

    struct input_event ev[EV_BUF_SIZE]; /* Read up to N events ata time */
    
    if (auto dir = opendir("/dev/input/by-id/")) 
    {
        printf("List of devices:\r\n");
		while (auto f = readdir(dir)) {
			if (!f->d_name || f->d_name[0] == '.')
				continue; // Skip everything that starts with a dot

			printf("%d - %s\n", dev,f->d_name);
            //sprintf(devices[dev], "%s%s", "/dev/input/by-id/", f->d_name);
			devices[dev] = f->d_name;
			dev++;
		}
		closedir(dir);
	}
    
  select:  
    printf("Select a device: ");
	cin >> deviceNumber;
	if((deviceNumber >= 0) && (deviceNumber <= dev))
    {
        printf("%s\n", devices[deviceNumber]);
    }
    else
    {
        printf("Device not Found!");
        goto select;
    }

    char devSelected[512];
    sprintf(devSelected, "%s%s", "/dev/input/by-id/", devices[deviceNumber]);
    
    if ((fd = open(devSelected, O_RDONLY)) < 0) {
        fprintf(stderr,
            "ERR %d:\n"
            "Unable to open `%s'\n"
            "%s\n",
            errno, devSelected, strerror(errno)
        );
    } else {
        printf(getDeviceInfo(fd).c_str());
    }
    
    

    /* Loop. Read event file and parse result. */
    for (;;) {
        sz = read(fd, ev, sizeof(struct input_event) * EV_BUF_SIZE);

        if (sz < (int) sizeof(struct input_event)) {
            fprintf(stderr,
                "ERR %d:\n"
                "Reading of `%s' failed\n"
                "%s\n",
                errno, argv[1], strerror(errno)
            );
            goto fine;
        }

        for (i = 0; i < sz / sizeof(struct input_event); ++i) {
            /*
            fprintf(stderr,
                "%ld.%06ld: "
                "type=%02x "
                "code=%02x "
                "value=%02x \n",
                ev[i].time.tv_sec,
                ev[i].time.tv_usec,
                ev[i].type,
                ev[i].code,
                ev[i].value
            );
            */
            if((ev[i].type == EV_KEY) && (ev[i].value == KEY_PRESS) && (ev[i].code == 0x2a))
            {
                caps = true;
            }
            else if((ev[i].type == EV_KEY) && (ev[i].value == KEY_RELEASE) && (ev[i].code == 0x2a))
            {
                caps = false;
            }
            if((ev[i].type == 0x01) && (ev[i].value == KEY_PRESS))
            {
                if(caps == true)
                {
                    fprintf(stderr, "%c", capscodes[ev[i].code]);
                }
                else
                {
                    fprintf(stderr, "%c", scancodes[ev[i].code]);
                }
            }
        }
    }

fine:
    close(fd);

    return errno;
}

