#include "rwlock.h"

void InitalizeReadWriteLock(struct read_write_lock * rw)
{
  //	Write the code for initializing your read-write lock.
  pthread_mutex_init(&rw->lock, NULL);
  pthread_cond_init(&rw->reader, NULL);
  pthread_cond_init(&rw->writer, NULL);
  rw->reader_count = 0;
  rw->writer_count = 0;
}

void ReaderLock(struct read_write_lock * rw)
{
  //	Write the code for aquiring read-write lock by the reader.
  pthread_mutex_lock(&rw->lock);
  while(rw->writer_count > 0)
  {
    pthread_cond_wait(&rw->reader, &rw->lock);
  }
  rw->reader_count++;
  pthread_mutex_unlock(&rw->lock);
}

void ReaderUnlock(struct read_write_lock * rw)
{
  //	Write the code for releasing read-write lock by the reader.
  pthread_mutex_lock(&rw->lock);
  rw->reader_count--;
  if(rw->reader_count == 0)
  {
    pthread_cond_broadcast(&rw->writer);
  }
  pthread_mutex_unlock(&rw->lock);
}

void WriterLock(struct read_write_lock * rw)
{
  //	Write the code for aquiring read-write lock by the writer.
  pthread_mutex_lock(&rw->lock);
  while(rw->reader_count > 0){
    pthread_cond_wait(&rw->writer, &rw->lock);
  }
  while(rw->writer_count > 0){
    pthread_cond_wait(&rw->writer, &rw->lock);
  }
  rw->writer_count++;
  pthread_mutex_unlock(&rw->lock);
}

void WriterUnlock(struct read_write_lock * rw)
{
  //	Write the code for releasing read-write lock by the writer.
  pthread_mutex_lock(&rw->lock);
  rw->writer_count--;
  pthread_cond_broadcast(&rw->writer);
  pthread_cond_broadcast(&rw->reader);
  pthread_mutex_unlock(&rw->lock);
}
