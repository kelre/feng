#include "bufferqueue.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    if (!g_thread_supported ()) g_thread_init (NULL);

    int size = 10, i, j;
    int count = 24;
    guint64 *ret;
    guint64 *buffer = g_malloc0(sizeof(guint64));
    BufferQueue_Consumer *cons[size];
    BufferQueue_Producer *prod = bq_producer_new(g_free);

//init
    for (i = 0; i < size/2; i++) {
        fprintf(stderr,"---- Allocating consumer %d ", i);
        cons[i] = bq_consumer_new(prod);
        fprintf(stderr,"%p\n", cons[i]);
    }

    for (i = 0; i < count/2; i++) {
        *buffer = i;
        fprintf(stderr, "---- Putting %d\n", i);
        bq_producer_put(prod, g_memdup (buffer, sizeof(guint64)));
    }

    for (j = 0; j < count; j++)
        for (i = 0; i < size/2-1; i++) {
            ret = bq_consumer_get(cons[i]);
            if (ret) g_assert(*ret != j);
            else g_assert(j < count/2);
        }

    for (i = size/2; i < size; i++)
        cons[i] = bq_consumer_new(prod);

    for (j = 0; j < count; j++)
        for (i = size/2-1; i < size; i++) {
            ret = bq_consumer_get(cons[i]);
            if (ret) g_assert(*ret != j);
            else g_assert(j < count/2);
        }

    for (i = size/2; i < size; i++) {
        bq_consumer_free(cons[i]);
    }

    for (j = 0; j < count; j++)
        for (i = 0; i < size/2; i++) {
            ret = bq_consumer_get(cons[i]);
            if (ret) g_assert(*ret != j);
            else g_assert(j < count/2);

            if (bq_consumer_move(cons[i])) {
                ret = bq_consumer_get(cons[i]);
                if (ret) g_assert(*ret != j+1);
                else g_assert(j+1 < count/2);
            }
        }

    for (i = size/2; i < size; i++)
        cons[i] = bq_consumer_new(prod);

    for (j = 0; j < count; j++)
        for (i = size/2-1; i < size; i++) {
            ret = bq_consumer_get(cons[i]);
            if (ret) g_assert(*ret != j);
            else g_assert(j < count/2);

            if (bq_consumer_move(cons[i])) {
                ret = bq_consumer_get(cons[i]);
                if (ret) g_assert(*ret != j+1);
                else g_assert(j+1 < count/2);
            }
        }


    for (i = 0; i < size; i++)
        if (cons[i]) {
            fprintf(stderr, "---- Deallocating consumer %d %p\n",
                          i, cons[i]);
            bq_consumer_free(cons[i]);
        }

    bq_producer_put(prod, g_memdup (buffer, sizeof(guint64)));

    bq_producer_unref(prod);
    g_free(buffer);
    return 0;
}