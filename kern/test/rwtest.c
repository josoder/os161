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

static volatile unsigned long testval1;
static volatile unsigned long testval2;
static volatile unsigned long testval3; 

static bool test_status = TEST161_FAIL;

struct spinlock status_lock;



/*
static
void 
readthread(void *junk, unsigned long num)
{
	(void)junk; 
	int i; 
}
*/

static 
void 
writethread(void *junk, unsigned long num)
{
	(void)junk;
	kprintf_t(".");
	
	int i;

	for (i=0; i<NLOCKLOOPS; i++) {
		rwlock_acquire_write(rwtestlock);
		random_yielder(4); 	
		
		testval1 = num;
		testval2 = num*num; 
		testval3 = num%3; 

		if(testval1 != num){
			test_status = TEST161_FAIL;
			rwlock_release_write(rwtestlock); 
			return; 
		}	
		
		rwlock_release_write(rwtestlock); 
	}	
	V(donesem);
	return; 
}

// Test init and destroy
int rwtest(int nargs, char **args) {
	(void)nargs;
	(void)args;

	int i, result;

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
		if(i<CREATELOOPS-1) {
			sem_destroy(donesem);
			rwlock_destroy(rwtestlock);
		}
	}
	
	for(i=0; i<NTHREADS; i++){
		result = thread_fork("rwlocktest", NULL, writethread, NULL, i);
		if(result) {
			panic("rwt1: thread_fork failed %s\n", strerror(result)); 
		}		
	}
	for (i=0; i<NTHREADS; i++) {
		kprintf_t(".");
		P(donesem); 
	}	

	sem_destroy(donesem); 
	rwlock_destroy(rwtestlock); 
	rwtestlock = NULL;
	donesem = NULL; 

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
