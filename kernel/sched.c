/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <kernel.h>
#include <ksched.h>
#include <spinlock.h>
#include <sched_priq.h>
#include <wait_q.h>
#include <kswap.h>
#include <kernel_arch_func.h>
#include <syscall_handler.h>

#ifdef CONFIG_SCHED_DUMB
#define _priq_run_add		_priq_dumb_add
#define _priq_run_remove	_priq_dumb_remove
#define _priq_run_best		_priq_dumb_best
#else
#define _priq_run_add		_priq_rb_add
#define _priq_run_remove	_priq_rb_remove
#define _priq_run_best		_priq_rb_best
#endif

#ifdef CONFIG_WAITQ_FAST
#define _priq_wait_add		_priq_rb_add
#define _priq_wait_remove	_priq_rb_remove
#define _priq_wait_best		_priq_rb_best
#else
#define _priq_wait_add		_priq_dumb_add
#define _priq_wait_remove	_priq_dumb_remove
#define _priq_wait_best		_priq_dumb_best
#endif

/* the only struct _kernel instance */
struct _kernel _kernel;

static struct k_spinlock sched_lock;

#define LOCKED(lck) for (k_spinlock_key_t __i = {},			\
					  __key = k_spin_lock(lck);	\
			!__i.key;					\
			k_spin_unlock(lck, __key), __i.key = 1)

static inline int _is_preempt(struct k_thread *thread)
{
#ifdef CONFIG_PREEMPT_ENABLED
	/* explanation in kernel_struct.h */
	return thread->base.preempt <= _PREEMPT_THRESHOLD;
#else
	return 0;
#endif
}

static inline int _is_thread_dummy(struct k_thread *thread)
{
	return !!(thread->base.thread_state & _THREAD_DUMMY);
}

static inline int _is_idle(struct k_thread *thread)
{
#ifdef CONFIG_SMP
	return thread->base.is_idle;
#else
	extern struct k_thread * const _idle_thread;

	return thread == _idle_thread;
#endif
}

#ifdef _NON_OPTIMIZED_TICKS_PER_SEC
s32_t _ms_to_ticks(s32_t ms)
{
	s64_t ms_ticks_per_sec = (s64_t)ms * sys_clock_ticks_per_sec;

	return (s32_t)ceiling_fraction(ms_ticks_per_sec, MSEC_PER_SEC);
}
#endif

static struct k_thread *next_up(void)
{
#ifndef CONFIG_SMP
	/* In uniprocessor mode, we can leave the current thread in
	 * the queue (actually we have to, otherwise the assembly
	 * context switch code for all architectures would be
	 * responsible for putting it back in _Swap and ISR return!),
	 * which makes this choice simple.
	 */
	struct k_thread *th = _priq_run_best(&_kernel.ready_q.runq);

	return th ? th : _current_cpu->idle_thread;
#else
	/* Under SMP, the "cache" mechanism for selecting the next
	 * thread doesn't work, so we have more work to do to test
	 * _current against the best choice from the queue.
	 */
	int active = !_is_thread_prevented_from_running(_current);
	int queued = _is_thread_queued(_current);

	struct k_thread *th = _priq_run_best(&_kernel.ready_q.runq);

	/* Idle thread if nothing else */
	if (!th) {
		th = _current_cpu->idle_thread;
	}

	/* Stay with current unless it's already been put back in the
	 * queue and something better is available (c.f. timeslicing,
	 * yield)
	 */
	if (active && !queued && !_is_t1_higher_prio_than_t2(th, _current)) {
		th = _current;
	}

	/* Put _current back into the queue if necessary */
	if (th != _current && !queued) {
		_priq_run_add(&_kernel.ready_q.runq, _current);
	}

	/* Remove the thread we're about to run from the queue (which
	 * potentially might not be there, but that's OK)
	 */
	_priq_run_remove(&_kernel.ready_q.runq, th);

	return th;
#endif
}

static void update_cache(int preempt_ok)
{
#ifndef CONFIG_SMP
	struct k_thread *th = next_up();

	if (_current && !_is_idle(_current) && !_is_thread_dummy(_current)) {
		/* Don't preempt cooperative threads unless the caller allows
		 * it (i.e. k_yield())
		 */
		if (!preempt_ok && !_is_preempt(_current)) {
			th = _current;
		}
	}

	_kernel.ready_q.cache = th;
#endif
}

void _add_thread_to_ready_q(struct k_thread *thread)
{
	LOCKED(&sched_lock) {
		_priq_run_add(&_kernel.ready_q.runq, thread);
		_mark_thread_as_queued(thread);
		update_cache(0);
	}
}

void _move_thread_to_end_of_prio_q(struct k_thread *thread)
{
	LOCKED(&sched_lock) {
		_priq_run_remove(&_kernel.ready_q.runq, thread);
		_priq_run_add(&_kernel.ready_q.runq, thread);
		_mark_thread_as_queued(thread);
		update_cache(0);
	}
}

void _remove_thread_from_ready_q(struct k_thread *thread)
{
	LOCKED(&sched_lock) {
		if (_is_thread_queued(thread)) {
			_priq_run_remove(&_kernel.ready_q.runq, thread);
			_mark_thread_as_not_queued(thread);
			update_cache(thread == _current);
		}
	}
}

static void pend(struct k_thread *thread, _wait_q_t *wait_q, s32_t timeout)
{
	_remove_thread_from_ready_q(thread);
	_mark_thread_as_pending(thread);

	/* The timeout handling is currently synchronized external to
	 * the scheduler using the legacy global lock.  Should fix
	 * that.
	 */
	if (timeout != K_FOREVER) {
		s32_t ticks = _TICK_ALIGN + _ms_to_ticks(timeout);
		int key = irq_lock();

		_add_thread_timeout(thread, wait_q, ticks);
		irq_unlock(key);
	}

	if (wait_q) {
#ifdef CONFIG_WAITQ_FAST
		thread->base.pended_on = wait_q;
#endif
		_priq_wait_add(&wait_q->waitq, thread);
	}

#ifdef CONFIG_KERNEL_EVENT_LOGGER_THREAD
	_sys_k_event_logger_thread_pend(thread);
#endif
}

void _pend_thread(struct k_thread *thread, _wait_q_t *wait_q, s32_t timeout)
{
	__ASSERT_NO_MSG(thread == _current || _is_thread_dummy(thread));
	pend(thread, wait_q, timeout);
}

static _wait_q_t *pended_on(struct k_thread *thread)
{
#ifdef CONFIG_WAITQ_FAST
	__ASSERT_NO_MSG(thread->base.pended_on);

	return thread->base.pended_on;
#else
	ARG_UNUSED(thread);
	return NULL;
#endif
}

struct k_thread *_find_first_thread_to_unpend(_wait_q_t *wait_q,
					      struct k_thread *from)
{
	ARG_UNUSED(from);

	struct k_thread *ret = NULL;

	LOCKED(&sched_lock) {
		ret = _priq_wait_best(&wait_q->waitq);
	}

	return ret;
}

void _unpend_thread_no_timeout(struct k_thread *thread)
{
	LOCKED(&sched_lock) {
		_priq_wait_remove(&pended_on(thread)->waitq, thread);
		_mark_thread_as_not_pending(thread);
	}

#if defined(CONFIG_ASSERT) && defined(CONFIG_WAITQ_FAST)
	thread->base.pended_on = NULL;
#endif
}

int _pend_current_thread(int key, _wait_q_t *wait_q, s32_t timeout)
{
	pend(_current, wait_q, timeout);
	return _Swap(key);
}

struct k_thread *_unpend_first_thread(_wait_q_t *wait_q)
{
	struct k_thread *t = _unpend1_no_timeout(wait_q);

	if (t) {
		_abort_thread_timeout(t);
	}

	return t;
}

void _unpend_thread(struct k_thread *thread)
{
	_unpend_thread_no_timeout(thread);
	_abort_thread_timeout(thread);
}

/* FIXME: this API is glitchy when used in SMP.  If the thread is
 * currently scheduled on the other CPU, it will silently set it's
 * priority but nothing will cause a reschedule until the next
 * interrupt.  An audit seems to show that all current usage is to set
 * priorities on either _current or a pended thread, though, so it's
 * fine for now.
 */
void _thread_priority_set(struct k_thread *thread, int prio)
{
	int need_sched = 0;

	LOCKED(&sched_lock) {
		need_sched = _is_thread_ready(thread);

		if (need_sched) {
			_priq_run_remove(&_kernel.ready_q.runq, thread);
			thread->base.prio = prio;
			_priq_run_add(&_kernel.ready_q.runq, thread);
			update_cache(1);
		} else {
			thread->base.prio = prio;
		}
	}

	if (need_sched) {
		_reschedule(irq_lock());
	}
}

int _reschedule(int key)
{
	if (!_is_in_isr() &&
	    _is_preempt(_current) &&
	    _get_next_ready_thread() != _current) {
		return _Swap(key);
	}

	irq_unlock(key);
	return 0;
}

void k_sched_lock(void)
{
	LOCKED(&sched_lock) {
		_sched_lock();
	}
}

void k_sched_unlock(void)
{
#ifdef CONFIG_PREEMPT_ENABLED
	__ASSERT(_current->base.sched_locked != 0, "");
	__ASSERT(!_is_in_isr(), "");

	LOCKED(&sched_lock) {
		++_current->base.sched_locked;
		update_cache(1);
	}

	K_DEBUG("scheduler unlocked (%p:%d)\n",
		_current, _current->base.sched_locked);

	_reschedule(irq_lock());
#endif
}

#ifdef CONFIG_SMP
struct k_thread *_get_next_ready_thread(void)
{
	struct k_thread *ret = 0;

	LOCKED(&sched_lock) {
		ret = next_up();
	}

	return ret;
}
#endif

#ifdef CONFIG_USE_SWITCH
void *_get_next_switch_handle(void *interrupted)
{
	if (!_is_preempt(_current) &&
	    !(_current->base.thread_state & _THREAD_DEAD)) {
		return interrupted;
	}

	_current->switch_handle = interrupted;

	LOCKED(&sched_lock) {
		struct k_thread *next = next_up();

		if (next != _current) {
			_current = next;
		}
	}

	_check_stack_sentinel();

	return _current->switch_handle;
}
#endif

void _priq_dumb_add(sys_dlist_t *pq, struct k_thread *thread)
{
	struct k_thread *t;

	__ASSERT_NO_MSG(!_is_idle(thread));

	SYS_DLIST_FOR_EACH_CONTAINER(pq, t, base.qnode_dlist) {
		if (_is_t1_higher_prio_than_t2(thread, t)) {
			sys_dlist_insert_before(pq, &t->base.qnode_dlist,
						&thread->base.qnode_dlist);
			return;
		}
	}

	sys_dlist_append(pq, &thread->base.qnode_dlist);
}

void _priq_dumb_remove(sys_dlist_t *pq, struct k_thread *thread)
{
	__ASSERT_NO_MSG(!_is_idle(thread));

	sys_dlist_remove(&thread->base.qnode_dlist);
}

struct k_thread *_priq_dumb_best(sys_dlist_t *pq)
{
	return CONTAINER_OF(sys_dlist_peek_head(pq),
			    struct k_thread, base.qnode_dlist);
}

int _priq_rb_lessthan(struct rbnode *a, struct rbnode *b)
{
	struct k_thread *ta, *tb;

	ta = CONTAINER_OF(a, struct k_thread, base.qnode_rb);
	tb = CONTAINER_OF(b, struct k_thread, base.qnode_rb);

	if (_is_t1_higher_prio_than_t2(ta, tb)) {
		return 1;
	} else if (_is_t1_higher_prio_than_t2(tb, ta)) {
		return 0;
	} else {
		return ta->base.order_key < tb->base.order_key ? 1 : 0;
	}
}

void _priq_rb_add(struct _priq_rb *pq, struct k_thread *thread)
{
	struct k_thread *t;

	__ASSERT_NO_MSG(!_is_idle(thread));

	thread->base.order_key = pq->next_order_key++;

	/* Renumber at wraparound.  This is tiny code, and in practice
	 * will almost never be hit on real systems.  BUT on very
	 * long-running systems where a priq never completely empties
	 * AND that contains very large numbers of threads, it can be
	 * a latency glitch to loop over all the threads like this.
	 */
	if (!pq->next_order_key) {
		RB_FOR_EACH_CONTAINER(&pq->tree, t, base.qnode_rb) {
			t->base.order_key = pq->next_order_key++;
		}
	}

	rb_insert(&pq->tree, &thread->base.qnode_rb);
}

void _priq_rb_remove(struct _priq_rb *pq, struct k_thread *thread)
{
	__ASSERT_NO_MSG(!_is_idle(thread));

	rb_remove(&pq->tree, &thread->base.qnode_rb);

	if (!pq->tree.root) {
		pq->next_order_key = 0;
	}
}

struct k_thread *_priq_rb_best(struct _priq_rb *pq)
{
	struct rbnode *n = rb_get_min(&pq->tree);

	return CONTAINER_OF(n, struct k_thread, base.qnode_rb);
}

#ifdef CONFIG_TIMESLICING
extern s32_t _time_slice_duration;    /* Measured in ms */
extern s32_t _time_slice_elapsed;     /* Measured in ms */
extern int _time_slice_prio_ceiling;

void k_sched_time_slice_set(s32_t duration_in_ms, int prio)
{
	__ASSERT(duration_in_ms >= 0, "");
	__ASSERT((prio >= 0) && (prio < CONFIG_NUM_PREEMPT_PRIORITIES), "");

	_time_slice_duration = duration_in_ms;
	_time_slice_elapsed = 0;
	_time_slice_prio_ceiling = prio;
}

int _is_thread_time_slicing(struct k_thread *thread)
{
	int ret = 0;

	/* Should fix API.  Doesn't make sense for non-running threads
	 * to call this
	 */
	__ASSERT_NO_MSG(thread == _current);

	if (_time_slice_duration <= 0 || !_is_preempt(thread) ||
	    _is_prio_higher(thread->base.prio, _time_slice_prio_ceiling)) {
		return 0;
	}


	LOCKED(&sched_lock) {
		struct k_thread *next = _priq_run_best(&_kernel.ready_q.runq);

		if (next) {
			ret = thread->base.prio == next->base.prio;
		}
	}

	return ret;
}

/* Must be called with interrupts locked */
/* Should be called only immediately before a thread switch */
void _update_time_slice_before_swap(void)
{
#ifdef CONFIG_TICKLESS_KERNEL
	if (!_is_thread_time_slicing(_get_next_ready_thread())) {
		return;
	}

	u32_t remaining = _get_remaining_program_time();

	if (!remaining || (_time_slice_duration < remaining)) {
		_set_time(_time_slice_duration);
	} else {
		/* Account previous elapsed time and reprogram
		 * timer with remaining time
		 */
		_set_time(remaining);
	}

#endif
	/* Restart time slice count at new thread switch */
	_time_slice_elapsed = 0;
}
#endif /* CONFIG_TIMESLICING */

int _unpend_all(_wait_q_t *waitq)
{
	int need_sched = 0;
	struct k_thread *th;

	while ((th = _waitq_head(waitq))) {
		_unpend_thread(th);
		_ready_thread(th);
		need_sched = 1;
	}

	return need_sched;
}

void _sched_init(void)
{
#ifdef CONFIG_SCHED_DUMB
	sys_dlist_init(&_kernel.ready_q.runq);
#else
	_kernel.ready_q.runq = (struct _priq_rb) {
		.tree = {
			.lessthan_fn = _priq_rb_lessthan,
		}
	};
#endif
}

int _impl_k_thread_priority_get(k_tid_t thread)
{
	return thread->base.prio;
}

#ifdef CONFIG_USERSPACE
Z_SYSCALL_HANDLER1_SIMPLE(k_thread_priority_get, K_OBJ_THREAD,
			  struct k_thread *);
#endif

void _impl_k_thread_priority_set(k_tid_t tid, int prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	_ASSERT_VALID_PRIO(prio, NULL);
	__ASSERT(!_is_in_isr(), "");

	struct k_thread *thread = (struct k_thread *)tid;

	_thread_priority_set(thread, prio);
}

#ifdef CONFIG_USERSPACE
Z_SYSCALL_HANDLER(k_thread_priority_set, thread_p, prio)
{
	struct k_thread *thread = (struct k_thread *)thread_p;

	Z_OOPS(Z_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	Z_OOPS(Z_SYSCALL_VERIFY_MSG(_is_valid_prio(prio, NULL),
				    "invalid thread priority %d", (int)prio));
	Z_OOPS(Z_SYSCALL_VERIFY_MSG((s8_t)prio >= thread->base.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->base.prio));

	_impl_k_thread_priority_set((k_tid_t)thread, prio);
	return 0;
}
#endif

void _impl_k_yield(void)
{
	__ASSERT(!_is_in_isr(), "");

	if (!_is_idle(_current)) {
		LOCKED(&sched_lock) {
			_priq_run_remove(&_kernel.ready_q.runq, _current);
			_priq_run_add(&_kernel.ready_q.runq, _current);
			update_cache(1);
		}
	}

	if (_get_next_ready_thread() != _current) {
		_Swap(irq_lock());
	}
}

#ifdef CONFIG_USERSPACE
Z_SYSCALL_HANDLER0_SIMPLE_VOID(k_yield);
#endif

void _impl_k_sleep(s32_t duration)
{
#ifdef CONFIG_MULTITHREADING
	/* volatile to guarantee that irq_lock() is executed after ticks is
	 * populated
	 */
	volatile s32_t ticks;
	unsigned int key;

	__ASSERT(!_is_in_isr(), "");
	__ASSERT(duration != K_FOREVER, "");

	K_DEBUG("thread %p for %d ns\n", _current, duration);

	/* wait of 0 ms is treated as a 'yield' */
	if (duration == 0) {
		k_yield();
		return;
	}

	ticks = _TICK_ALIGN + _ms_to_ticks(duration);
	key = irq_lock();

	_remove_thread_from_ready_q(_current);
	_add_thread_timeout(_current, NULL, ticks);

	_Swap(key);
#endif
}

#ifdef CONFIG_USERSPACE
Z_SYSCALL_HANDLER(k_sleep, duration)
{
	/* FIXME there were some discussions recently on whether we should
	 * relax this, thread would be unscheduled until k_wakeup issued
	 */
	Z_OOPS(Z_SYSCALL_VERIFY_MSG(duration != K_FOREVER,
				    "sleeping forever not allowed"));
	_impl_k_sleep(duration);

	return 0;
}
#endif

void _impl_k_wakeup(k_tid_t thread)
{
	int key = irq_lock();

	/* verify first if thread is not waiting on an object */
	if (_is_thread_pending(thread)) {
		irq_unlock(key);
		return;
	}

	if (_abort_thread_timeout(thread) == _INACTIVE) {
		irq_unlock(key);
		return;
	}

	_ready_thread(thread);

	if (_is_in_isr()) {
		irq_unlock(key);
	} else {
		_reschedule(key);
	}
}

#ifdef CONFIG_USERSPACE
Z_SYSCALL_HANDLER1_SIMPLE_VOID(k_wakeup, K_OBJ_THREAD, k_tid_t);
#endif

k_tid_t _impl_k_current_get(void)
{
	return _current;
}

#ifdef CONFIG_USERSPACE
Z_SYSCALL_HANDLER0_SIMPLE(k_current_get);
#endif

int _impl_k_is_preempt_thread(void)
{
	return !_is_in_isr() && _is_preempt(_current);
}

#ifdef CONFIG_USERSPACE
Z_SYSCALL_HANDLER0_SIMPLE(k_is_preempt_thread);
#endif
