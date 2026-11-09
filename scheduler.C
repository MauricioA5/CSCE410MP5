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

/* Use one scheduler for all kernel threads. */
static Scheduler * active_scheduler;

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

static void run_idle() {
  for (;;) active_scheduler->yield();
}

Scheduler::Scheduler() : ready_head(0), ready_tail(0), idle(0), finished(0) {
  active_scheduler = this;
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  collect_finished();
  if (!ready_head) {
    /* Keep the idle thread outside the FIFO queue. */
    if (!idle) idle = new Thread(run_idle, new char[1024], 1024);
    if (Thread::CurrentThread() != idle) Thread::dispatch_to(idle);
    return;
  }

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
  assert(_thread && _thread != idle);
  /* Unlink a ready thread before its caller destroys it. */
  Node * previous = 0;
  for (Node * node = ready_head; node; node = node->next) {
    if (node->thread == _thread) {
      if (previous) previous->next = node->next;
      else ready_head = node->next;
      if (node == ready_tail) ready_tail = previous;
      delete node;
      break;
    }
    previous = node;
  }

  if (_thread == Thread::CurrentThread()) {
    collect_finished();
    finished = _thread;
    yield();
  }
}

void Scheduler::collect_finished() {
  /* Only allow a thread to releas its predecessor only after switching stacks. */
  if (finished && finished != Thread::CurrentThread()) {
    delete finished;
    finished = 0;
  }
}

void Scheduler::terminate_current() {
  assert(active_scheduler);
  active_scheduler->terminate(Thread::CurrentThread());
}
