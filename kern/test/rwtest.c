/*
 * All the contents of this file are overwritten during automated
 * testing. Please consider this before changing anything in this file.
 */


#define CREATELOOPS		8
#define NTHREADS      32
#define NLOCKLOOPS    120

#include <types.h>
#include <lib.h>
#include <clock.h>
#include <thread.h>
#include <synch.h>
#include <test.h>
#include <kern/test161.h>
#include <spinlock.h>



static struct rwlock *rwtestlock = NULL;
static struct semaphore *donesem = NULL;
//static struct cv *testcv = NULL;

static bool test_status = TEST161_FAIL;


// Test init and destroy
int rwtest(int nargs, char **args) {
	(void)nargs;
	(void)args;

	int i;

	kprintf_n("Starting rwt1...");
	for (i=0; i<CREATELOOPS; i++) {
		kprintf_t(".");
		rwtestlock = rwlock_create("testlock");

		if (rwtestlock == NULL) {
			panic("rwt1: lock_create failed\n");
		}
		rwlock_acquire_read(rwtestlock); 
		if(rwtestlock->readers==0) {
			panic("rwt1: lock_acquire failed\n"); 
		}

		rwlock_release_read(rwtestlock); 
		
		if(rwtestlock->readers>0) {
			panic("rwt1: lock_release failed\n"); 
		}
		
		donesem = sem_create("donesem", 0);
		if (rwtestlock == NULL) {
			panic("rwt1: sem_create failed\n");
		}
		sem_destroy(donesem);
		rwlock_destroy(rwtestlock);
	}
	

	kprintf_t("\n");
	test_status = TEST161_SUCCESS;
	success(test_status, SECRET, "rwt1");


	return 0;

}

int rwtest2(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt2 unimplemented\n");
	success(TEST161_FAIL, SECRET, "rwt2");

	return 0;
}

int rwtest3(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt3 unimplemented\n");
	success(TEST161_FAIL, SECRET, "rwt3");

	return 0;
}

int rwtest4(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt4 unimplemented\n");
	success(TEST161_FAIL, SECRET, "rwt4");

	return 0;
}

int rwtest5(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt5 unimplemented\n");
	success(TEST161_FAIL, SECRET, "rwt5");

	return 0;
}
