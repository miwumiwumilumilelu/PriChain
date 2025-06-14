#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/rtmutex.h>
#include <linux/sched.h>
#include <linux/sched/rt.h>
#include <linux/delay.h>

struct sched_param {
    int sched_priority;
};

static struct rt_mutex test_lock;
static struct task_struct *task_l, *task_h, *task_m;

int low_prio_thread(void *data)
{
    struct sched_param sp;
    int i;
    sp.sched_priority = 10;
    sched_setscheduler_nocheck(current, SCHED_FIFO, &sp);

    pr_alert("rtmutex L: START, prio: %d, normal_prio: %d\n", current->prio, current->normal_prio);
    pr_alert("rtmutex L: try to acquire rtmutex, prio: %d, normal_prio: %d\n", current->prio, current->normal_prio);
    rt_mutex_lock(&test_lock);
    pr_alert("rtmutex L: GOT rtmutex, prio: %d, normal_prio: %d\n", current->prio, current->normal_prio);
    for (i = 0; i < 6; i++) {
        pr_alert("rtmutex L: time=%lu, current prio: %d, normal_prio: %d\n",
                jiffies, current->prio, current->normal_prio);
        ssleep(1);
    }
    pr_alert("rtmutex L: RELEASE rtmutex, prio: %d, normal_prio: %d\n", current->prio, current->normal_prio);
    rt_mutex_unlock(&test_lock);
    pr_alert("rtmutex L: END, prio: %d, normal_prio: %d\n", current->prio, current->normal_prio);
    return 0;
}

int mid_prio_thread(void *data)
{
    struct sched_param sp;
    int i;
    sp.sched_priority = 15;
    sched_setscheduler_nocheck(current, SCHED_FIFO, &sp);

    pr_alert("rtmutex M: START, prio: %d, normal_prio: %d\n", current->prio, current->normal_prio);
    ssleep(2); // 确保L和H先执行
    pr_alert("rtmutex M: try to acquire rtmutex, prio: %d, normal_prio: %d\n", current->prio, current->normal_prio);
    rt_mutex_lock(&test_lock);
    pr_alert("rtmutex M: GOT rtmutex, prio: %d, normal_prio: %d\n", current->prio, current->normal_prio);
    for (i = 0; i < 2; i++) {
        pr_alert("rtmutex M: time=%lu, prio: %d, normal_prio: %d\n", jiffies, current->prio, current->normal_prio);
        ssleep(1);
    }
    pr_alert("rtmutex M: RELEASE rtmutex, prio: %d, normal_prio: %d\n", current->prio, current->normal_prio);
    rt_mutex_unlock(&test_lock);
    pr_alert("rtmutex M: END, prio: %d, normal_prio: %d\n", current->prio, current->normal_prio);
    return 0;
}

int high_prio_thread(void *data)
{
    struct sched_param sp;
    sp.sched_priority = 20;
    sched_setscheduler_nocheck(current, SCHED_FIFO, &sp);

    pr_alert("rtmutex H: START, prio: %d, normal_prio: %d\n", current->prio, current->normal_prio);
    ssleep(1); // 确保L先获得锁
    pr_alert("rtmutex H: try to acquire rtmutex, prio: %d, normal_prio: %d\n", current->prio, current->normal_prio);
    rt_mutex_lock(&test_lock);
    pr_alert("rtmutex H: GOT rtmutex, prio: %d, normal_prio: %d\n", current->prio, current->normal_prio);
    rt_mutex_unlock(&test_lock);
    pr_alert("rtmutex H: END, prio: %d, normal_prio: %d\n", current->prio, current->normal_prio);
    return 0;
}

static int __init rtmutex_test_init(void)
{
    pr_info("rtmutex test module loaded\n");
    rt_mutex_init(&test_lock);

    task_l = kthread_run(low_prio_thread, NULL, "low_prio_thread");
    task_m = kthread_run(mid_prio_thread, NULL, "mid_prio_thread");
    task_h = kthread_run(high_prio_thread, NULL, "high_prio_thread");

    return 0;
}

static void __exit rtmutex_test_exit(void)
{
    pr_info("rtmutex test module unloaded\n");
}

module_init(rtmutex_test_init);
module_exit(rtmutex_test_exit);
MODULE_LICENSE("GPL"); 