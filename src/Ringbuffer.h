#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include <Arduino.h>

template <typename BUFFTYPE, uint16_t BUFFSIZE>
class Ringbuffer {
private:
    BUFFTYPE buffer[BUFFSIZE];
    uint16_t readPos, size;

public:
    Ringbuffer();
    bool push(const BUFFTYPE * const value) __attribute__ ((noinline));
    bool pull(BUFFTYPE &value) __attribute__ ((noinline));
    bool contains(const BUFFTYPE * const value) __attribute__ ((noinline));
    uint16_t counterEqual(const BUFFTYPE * const value);
    void clear()   { size = 0; }
    uint16_t currentSize() { return size; }
    uint16_t freeSize()  { return BUFFSIZE - size; }
    bool isFull() { return size == BUFFSIZE; }
    bool isEmpty() { return size == 0; }
    uint16_t maxSize() { return BUFFSIZE; }
};

template <typename BUFFTYPE, uint16_t BUFFSIZE>
Ringbuffer<BUFFTYPE, BUFFSIZE>::Ringbuffer()
{
    readPos = 0;
    size = 0;
}

template <typename BUFFTYPE, uint16_t BUFFSIZE>
bool Ringbuffer<BUFFTYPE, BUFFSIZE>::push(const BUFFTYPE * const value)
{
    if (size == BUFFSIZE) 
        return false;

    uint16_t writePos = readPos + size;
    if (writePos >= BUFFSIZE) 
        writePos -= BUFFSIZE;
    buffer[writePos] = *value;
    size++;
    return true;
}

template <typename BUFFTYPE, uint16_t BUFFSIZE>
bool Ringbuffer<BUFFTYPE, BUFFSIZE>::pull(BUFFTYPE &value)
{
  if (size == 0) 
    return false;
  value = buffer[readPos];
  readPos++;
  size--;
  if (readPos == BUFFSIZE) 
    readPos = 0;
  return true;
}

template <typename BUFFTYPE, uint16_t BUFFSIZE>
bool Ringbuffer<BUFFTYPE, BUFFSIZE>::contains(const BUFFTYPE * const value)
{
    if (size == 0) 
        return false;

    for (uint16_t i = 0, j = readPos; i < size; i++) {
      if (j == BUFFSIZE) 
        j = 0;
      if (buffer[j++] == *value)
        return true;
    }
    
    return false;
}

template <typename BUFFTYPE, uint16_t BUFFSIZE>
uint16_t Ringbuffer<BUFFTYPE, BUFFSIZE>::counterEqual(const BUFFTYPE * const value)
{
    if (size == 0) 
        return false;

    uint16_t count = 0;
    for (uint16_t i = 0, j = readPos; i < size; i++) {
      if (j == BUFFSIZE) 
        j = 0;
      if (buffer[j++] == *value)
        count++;
    }
    
    return count;
}

#endif /* __RINGBUFFER_H__*/