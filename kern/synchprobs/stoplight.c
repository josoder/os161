/*
 * Copyright (c) 2001, 2002, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Driver code is in kern/tests/synchprobs.c We will replace that file. This
 * file is yours to modify as you see fit.
 *
 * You should implement your solution to the stoplight problem below. The
 * quadrant and direction mappings for reference: (although the problem is, of
 * course, stable under rotation)
 *
 *   |0 |
 * -     --
 *    01  1
 * 3  32
 * --    --
 *   | 2|
 *
 * As way to think about it, assuming cars drive on the right: a car entering
 * the intersection from direction X will enter intersection quadrant X first.
 * The semantics of the problem are that once a car enters any quadrant it has
 * to be somewhere in the intersection until it call leaveIntersection(),
 * which it should call while in the final quadrant.
 *
 * As an example, let's say a car approaches the intersection and needs to
 * pass through quadrants 0, 3 and 2. Once you call inQuadrant(0), the car is
 * considered in quadrant 0 until you call inQuadrant(3). After you call
 * inQuadrant(2), the car is considered in quadrant 2 until you call
 * leaveIntersection().
 *
 * You will probably want to write some helper functions to assist with the
 * mappings. Modular arithmetic can help, e.g. a car passing straight through
 * the intersection entering from direction X will leave to direction (X + 2)
 * % 4 and pass through quadrants X and (X + 3) % 4.  Boo-yah.
 *
 * Your solutions below should call the inQuadrant() and leaveIntersection()
 * functions in synchprobs.c to record their progress.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>


int getdirectionleft(int fromposition);
int getdirectionstraight(int fromposition); 
void enterintersection(uint32_t fromposition, uint32_t index); 
void exit(uint32_t quad, uint32_t index);
void switchquad(uint32_t current, uint32_t next, uint32_t index);


struct lock *quad_locks[4];  
static struct cv *icv; 

volatile int left_turns; 
volatile int cars_in_intersection; 

struct lock *wait_lock; 



/*
 * Called by the driver during initialization.
 */

void
stoplight_init() {
	quad_locks[0] = lock_create("qlock0");
	quad_locks[1] = lock_create("qlock1");
	quad_locks[2] = lock_create("qlock2");
	quad_locks[3] = lock_create("qlock3");
	
	wait_lock = lock_create("wait lock");
	
	if ( wait_lock == NULL ) {
		panic("failed to init lock"); 
	}	

	for(int i=0; i<4; i++) {
		if(quad_locks[i] == NULL) {
			panic("failed to init lock");
		}
	}
	
	icv = cv_create("icv"); 
	if(icv == NULL) {
		panic("failed to init cv");
	}

	left_turns = 0; 
	cars_in_intersection = 0; 

	return;
}

// Helper methods
int getdirectionleft(int fromposition){
	return (fromposition + 2) % 4; 
}

int getdirectionstraight(int fromposition){
	return (fromposition + 3) % 4; 
}

void
exit(uint32_t quad, uint32_t index) {
	leaveIntersection(index); 
	lock_release(quad_locks[quad]);	
	
	
	lock_acquire(wait_lock);

	cv_signal(icv, wait_lock);

	lock_release(wait_lock);
	
	cars_in_intersection--; 
}

void
enterintersection(uint32_t direction, uint32_t index){
	
	lock_acquire(wait_lock); 

	while(cars_in_intersection>=2){
		cv_wait(icv, wait_lock); 
	}
	
	lock_release(wait_lock); 
	
	lock_acquire(quad_locks[direction]);
       	cars_in_intersection++;
	inQuadrant(direction, index); 
}

void 
switchquad(uint32_t current ,uint32_t next, uint32_t index){ 
	lock_acquire(quad_locks[next]); 
	inQuadrant(next, index); 
	lock_release(quad_locks[current]);
}

/*
 * Called by the driver during teardown.
 */ 

void stoplight_cleanup() {
	for(int i=0; i<4; i++){
		lock_destroy(quad_locks[i]); 
	}	
	
	cv_destroy(icv); 

	return;
}

void
turnright(uint32_t direction, uint32_t index)
{
	// will only need to get pass through 1 quadrant
	
	enterintersection(direction, index); 

	exit(direction, index);
	return;
}
 	


void
gostraight(uint32_t direction, uint32_t index)
{
	enterintersection(direction, index); 
	
	int straight = getdirectionstraight(direction);
	
	switchquad(direction, straight, index);

	// Leave intersection 
	exit(straight, index);

	return;

}
void
turnleft(uint32_t direction, uint32_t index)
{
	// need to first go straight then to the left, so passing x, (x+3) % 4 and (x+2) % 3 
	int straight = getdirectionstraight(direction);
	int left = getdirectionleft(direction);
	
	// enter quad x
	enterintersection(direction, index);
	
	// enter the quad straight ahead
	switchquad(direction, straight, index);
	// enter the final quad 
	switchquad(straight, left, index);
	
	// exit
	exit(left, index);
	return;
}

