# Google-s-Pagerank-Algorithm-w-POSIX-threads

```
pthread_mutex_lock(&pr_mutex); thread_sum++;
if (thread_sum < NUM_THREADS {pthread_cond_wait(&pr_threshold, &pr_mutex);} if (thread_sum == NUM_THREADS){ pthread_cond_broadcast(&pr_threshold);
thread_sum = 0;} pthread_mutex_unlock(&pr_mutex);
```
