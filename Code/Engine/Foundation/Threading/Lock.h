#pragma once

/// Manages a lock (e.g. a mutex) and ensures that it is properly released as the lock object goes out of scope.
template <typename T>
class ezLock
{
public:
  EZ_FORCE_INLINE explicit ezLock(T& lock) :
    m_lock(lock)
  {
    m_lock.Acquire();
  }

  EZ_FORCE_INLINE ~ezLock()
  {
    m_lock.Release();
  }

private:
  T& m_lock;
};
