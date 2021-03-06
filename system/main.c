/*  main.c  - main */

#include <xinu.h>
#include <ramdisk.h>
#include <stdio.h>

/************************************************************************/
/*	Just for testing: length and msgdump 				*/
/************************************************************************/
void	msgdump(char *s, int32 len)
{
	int	i;
	char	ch;

	for (i=0; i<len; i++) {
		ch = *s++;
		if (ch=='\n') {
			kprintf("\n\r");
		} else if (((ch>=0x20) && (ch<=0x7e)) ||
			    (ch=='\r')) {
			kprintf("%c",ch);
		} else if (ch == NULLCH) {
			kprintf("~");
		} else {
			kprintf("$");
		}
	}
	kprintf("\r\n");
}

int32	getlen(char *s)
{
	int32	count;

	count = 0;
	while(*s++ != NULLCH) {
		count++;
	}
	return count+1;
}
extern	int32	lfsckfmt(did32);

void	prdump(void)
{
	int32 	i;
	struct	procent	*pptr;

	kprintf(" pid         name        prio state\n\r");
	kprintf("----- ------------------ ---- -----\n\r");
	for (i=0; i<NPROC; i++) {
		pptr = &proctab[i];
		if (pptr->prstate == PR_FREE) {
			continue;
		}
		kprintf("%4d  %-17s %4d   %2d\n\r",i,pptr->prname, pptr->prprio, pptr->prstate);
	}
	kprintf("\n\r");
	return;
}

/************************************************************************/
/*	end declarations of items used for testing			*/
/************************************************************************/

extern	uint32	clktime;
/************************************************************************/
/*									*/
/* main - main program for testing Xinu					*/
/*									*/
/************************************************************************/

int main(int argc, char **argv)
{
	int32	fd, fd2, nbytes;
	char	buff[2048];
	char	buff2[2048];
	int32	retval;
	bool8	err;
	int32	i, j;
	pid32	netpid, mypid;
	char	msg1[] = "this is a test\n";
	int32	len1;

	char	msg2[] = "Here's another test of the file system\n";
	int32	len2;

	char	msg3[] = \
"This is a test of the Xinu file system.  We will write this buffer to a\
file and then copy it to another file.  The trick is that that there are\
more than 512 characters in the text, meaning that it will span at least\
two bocks of the file system.  Of course, that doesn't really prove that\
the file system is working (as a matter of fact, it doesn't even prove a\
basic two-block file works completely).   However, it will exercise most\
pieces of code at least once.  We can also write this same message twice\
or translate it to upper case for the second write.\n";
	int32	len3;

	kprintf(" * * * * * *  MAIN PROGRAM STARTS SUCCESSFULLY  ! ! ! ! * * * * * * * *\n");

	/* Use DHCP to obtain: IP and router addresses and subnet mask	*/

	NetData.ipvalid = FALSE;
	kprintf("\nmain: Requesting an IP address\n");

	retval = getlocalip();

	if (retval == SYSERR) {
		panic("Stopping: cannot obtain an IP address\n\r");
	} else {
	    kprintf("IP address is %d.%d.%d.%d   %08x\n\r",
		(retval>>24)&0xff, (retval>>16)&0xff, (retval>>8)&0xff,
		 retval&0xff,retval);

	    kprintf("Subnet mask is %d.%d.%d.%d and router is %d.%d.%d.%d\n\r",
		(NetData.addrmask>>24)&0xff, (NetData.addrmask>>16)&0xff,
		(NetData.addrmask>> 8)&0xff,  NetData.addrmask&0xff,
		(NetData.routeraddr>>24)&0xff, (NetData.routeraddr>>16)&0xff,
		(NetData.routeraddr>> 8)&0xff, NetData.routeraddr&0xff);
	}

	/* Test sleep */

	kprintf("testing sleep of 2 seconds: clktime is %d\n", clktime);
	sleep(2);
	kprintf("the sleep interval ends and clktime is %d\n",clktime);

	/* creating a shell process */

	kprintf("main: creating a shell process\n");

	resume(create(shell, 8192, 50, "shell", 1, CONSOLE));

	/* Wait for shell to exit and recreate it */

	while (TRUE) {
		recvclr();
		retval = receive();
		kprintf("\n\nRecreating shell\n\n");
		resume(create(shell, 8192, 50, "shell", 1, CONSOLE));
	}


/*DEBUG*/ kprintf("\n\n\rMAIN PROCESS STOPPING AT THIS POINT\n\n");
/*DEBUG*/ kill(getpid()); //ABORT TO AVOID THE REST OF THIS STUFF

	len1 = getlen(msg1);
	len2 = getlen(msg2);
	len3 = getlen(msg3);


	kprintf("\n\r**********************************************************\n\r");

	/* creating an empty file system on device TESTDISK */

	fd = open(TESTDISK,"test_file","rw");
	lfscreate(TESTDISK, 20, RM_BLKS*RM_BLKSIZ);


	/* test the local file sytem */

kprintf("\r\nmsg lengths len1: %d   len2: %d   len3: %d\r\n",len1,len2,len3);

	fd = open(LFILESYS,"file1", "rw");
	kprintf("Opening 'file1' on 'LFILESYS' returns %d\n\r", fd);

	kprintf("message 1 is:\n\r\n\r");
	msgdump(msg1, len1);
	kprintf("\n\r");

	kprintf("message 2 is:\n\r\n\r");
	msgdump(msg2, len2);
	kprintf("\n\r");

	kprintf("message 3 is:\n\r");
	msgdump(msg3, len3);
	kprintf("\n\r");
	retval = open(RDISK,"ComerDisk","rw");

	kprintf("writing eight blocks to the disk\r\n");
	for (i=7; i>=0; i--) {
		memset(buff, (char)(i&0xff), RD_BLKSIZ);
		kprintf("\n\r*** writing block %d\n\r",i);
		retval = write(RDISK, buff, i);
		if (retval < 0) {
		   kprintf("write to block %d returns %d\r\n", i, retval);
		}
	}

	retval = write(RDISK, msg3, 1);
	if (retval < 0) {
	   kprintf("write to block 1 returns %d\r\n", retval);
	}
	kprintf("reading block 1\n\r");
	retval = read(RDISK, buff, 1);
	kprintf("read from block 1 returns %d\r\n", retval);

//	err = 0;
//	for (i=0; i<RD_BLKSIZ; i++) {
//		if (buff[i] != msg3[i]) {
//			err = 1;
//			break;
//		}
//	}
//	if (err == 0) {
//		kprintf("Got back identical results!!!\r\n");
//	} else {
//		kprintf("Sadly  :-( byte %d differs!!!\r\n", i);
//	}

	kprintf("reading block 6\n\r");
	retval = read(RDISK, buff, 6);
	err = 0;
	for (i=0; i<RD_BLKSIZ; i++) {
		if (buff[i] != (char) (0xff&6)) {
			err = 1;
			break;
		}
	}
	if (err == 0) {
		kprintf("Got back all sixes!!!\r\n");
	} else {
		kprintf("Sadly  :-( byte %d is not 6!!!\r\n", i);
	}

	j = 0;
	for (i=0; i<RD_BLKSIZ; i++) {
		buff2[i] = "abcdefghijklmnopqrstuvwxyz"[j++];
		j %= 5;
	}

//	kprintf("writing to block 6\n\r");
//	retval = write(RDISK, buff2, 6);
//	kprintf("write to block 6 returns %d\r\n", retval);

//	memset(buff, NULLCH, RD_BLKSIZ);

//	kprintf("reading block 6\n\r");
//	retval = read(RDISK, buff, 6);
//	kprintf("read from block 6 returns %d\r\n", retval);

//	err = 0;
//	for (i=0; i<RD_BLKSIZ; i++) {
//		if (buff[i] != buff2[i]) {
//	 		err = 1;
//			break;
//		}
//	}
//	if (err == 0) {
//		kprintf("Got back identical results!!!\r\n");
//	} else {
//		kprintf("Sadly  :-( byte %d differs!!!\r\n", i);
//	}

	retval = lfsckfmt(TESTDISK);
	if (retval == SYSERR) {
		kprintf("lfsckfmt bombs\n\r");
	}

	/* SHORT MESSAGE */

	kprintf("Writing a short message and reading it back\n\r");

	nbytes = write(fd, msg1, len1);
	kprintf("\r\nWrite msg1 of %d bytes to file returns %d\n\r", len1, nbytes);
	retval = seek(fd, 0);
	kprintf("Seek to 0 returns %d\n\r", retval);
	nbytes = read(fd, buff,20);
	kprintf("Read of 20 returns %d bytes:\n\r\n\r", nbytes);
	msgdump(buff, nbytes);
	kprintf("\n\r");

	/* APPEND A MESSAGE */

	nbytes = write(fd, msg2, len2);
	kprintf("Write msg2 of %d bytes returns %d\n\r", len2, nbytes);
	retval = seek(fd, 0);
	kprintf("Seek to 0 returns %d\n\r", retval);

	nbytes = read(fd, buff, len1);
	kprintf("\r\nReading %d bytes from file returns %d:\r\n\r\n", len1, nbytes);
	msgdump(buff, nbytes);
	kprintf("\r\n");
	if (memcmp(buff,msg1,nbytes) == 0) {
		kprintf("Matches msg1\n\r");
	} else {
		kprintf("Does not match msg1\n\r");
	}

	nbytes = read(fd, buff, len2);
	kprintf("\r\nRead of %d returns %d:\r\n\r\n", len2, nbytes);
	msgdump(buff, nbytes);
	kprintf("\r\n");

	if (memcmp(buff,msg2,nbytes) == 0) {
		kprintf("Matches msg2\n\r");
	} else {
		kprintf("Does not match msg1\n\r");
	}

	/* OPEN A SECOND FILE */

	fd2 = open(LFILESYS,"Another_file", "rw");
	kprintf("Opening 'Another_file' on 'LFILESYS' returns %d\n\r", fd2);

	kprintf("In main, address of msg3 is %08x  len3 is %d\n\r", msg3, len3);
	nbytes = write(fd2, msg3, len3);
	kprintf("\r\nWrite msg3 of %d bytes to file returns %d\n\r", len3, nbytes);
	retval = seek(fd2, 0);
	kprintf("Seek to 0 returns %d\n\r", retval);
	nbytes = read(fd2, buff,2048);
	kprintf("Read of 20 returns %d bytes:\n\r\n\r", nbytes);
	msgdump(buff, nbytes);
	kprintf("\n\r");

	if (memcmp(buff,msg3,nbytes) == 0) {
		kprintf("Matches msg3\n\r");
	} else {
		kprintf("Does not match msg3\n\r");
	}


// /* STOPPING HERE */

	kprintf("Killing netin process (ID = %d)\n\r", netpid);
	kill(netpid);

	mypid = getpid();
	kprintf("Killing main process (ID = %d)\n\r", mypid);
	kill(mypid);

	return OK;
}
