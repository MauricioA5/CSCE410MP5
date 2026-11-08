/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() : ready_head(0), ready_tail(0) {
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  if (!ready_head) return;

  Node * node = ready_head;
  Thread * next = node->thread;
  ready_head = node->next;
  if (!ready_head) ready_tail = 0;
  delete node;

  if (next != Thread::CurrentThread()) {
    Thread::dispatch_to(next);
  }
}

void Scheduler::resume(Thread * _thread) {
  /* A thread may only appear in the ready queue once. */
  assert(_thread);
  for (Node * queued = ready_head; queued; queued = queued->next) {
    assert(queued->thread != _thread);
  }
  Node * node = new Node{_thread, 0};
  if (ready_tail) {
    ready_tail->next = node;
  } else {
    ready_head = node;
  }
  ready_tail = node;
}

void Scheduler::add(Thread * _thread) {
  resume(_thread);
}

void Scheduler::terminate(Thread * _thread) {
  assert(false);
}
