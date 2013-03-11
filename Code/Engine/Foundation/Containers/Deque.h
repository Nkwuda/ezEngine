#pragma once

#include <Foundation/Algorithm/Sorting.h>

#define ezInvalidIndex 0xFFFFFFFF

template <typename T, bool Construct>
class ezDequeBase
{
protected:

  /// No memory is allocated during construction.
  ezDequeBase(ezIAllocator* pAllocator);

  /// No memory is allocated during construction.
  ezDequeBase(const ezDequeBase<T, Construct>& rhs, ezIAllocator* pAllocator);

  ~ezDequeBase();

  void operator= (const ezDequeBase<T, Construct>& rhs);

public:
  /// Destructs all elements and sets the count to zero. Does not deallocate any data.
  void Clear();

  /// Rearranges the internal data structures such that the amount of reserved elements can be appended with as few allocations, as possible.
  void Reserve(ezUInt32 uiCount);

  /// This function deallocates as much memory as possible to shrink the deque to the bare minimum size that it needs to work.
  /// This function is never called internally. It is only for the user to trigger a size reduction when he feels like it.
  /// The index array data is not reduced as much as possible, a bit spare memory is keept to allow for scaling the deque
  /// up again, without immediate reallocation of all data structures.
  /// If the deque is completely empty, ALL data is completely deallocated.
  void Compact();

  /// Sets the number of active elements in the deque. All new elements are default constructed. If the deque is shrunk, elements at the end of the deque are destructed.
  void SetCount(ezUInt32 uiCount);

  /// Accesses the n-th element in the deque.
  T& operator[](ezUInt32 uiIndex);

  /// Accesses the n-th element in the deque.
  const T& operator[](ezUInt32 uiIndex) const;

  /// Adds one default constructed element to the back of the deque.
  void PushBack();

  /// Adds one element to the back of the deque.
  void PushBack(const T& element);

  /// Removes the last element from the deque.
  void PopBack();

  /// Adds one element to the front of the deque.
  void PushFront(const T& element);

  /// Adds one default constructed element to the front of the deque.
  void PushFront();

  /// Removes the first element from the deque.
  void PopFront();

  /// Checks whether no elements are active in the deque.
  bool IsEmpty() const;

  /// Returns the number of active elements in the deque.
  ezUInt32 GetCount() const;

  /// Returns the first element.
  const T& PeekFront() const;

  /// Returns the first element.
  T& PeekFront();

  /// Returns the last element.
  const T& PeekBack() const;

  /// Returns the last element.
  T& PeekBack();

  /// Checks whether there is any element in the deque with the given value.
  bool Contains(const T& value) const;

  /// Returns the first index at which an element with the given value could be found.
  ezUInt32 IndexOf(const T& value, ezUInt32 uiStartIndex = 0) const;

  /// Returns the last index at wich an element with the given value could be found.
  ezUInt32 LastIndexOf(const T& value, ezUInt32 uiStartIndex = ezInvalidIndex) const;

  /// Removes the element at the given index and fills the gap with the last element in the deque.
  void RemoveAtSwap(ezUInt32 uiIndex);

  /// Removes the element at index and fills the gap by shifting all following elements
  void RemoveAt(ezUInt32 uiIndex);

  /// Removes the first occurrence of value and fills the gap by shifting all following elements
  bool Remove(const T& value);

  /// Inserts value at index by shifting all following elements. Valid insert positions are [0; GetCount].
  void Insert(const T& value, ezUInt32 uiIndex);

  /// Sort with explicit comparer
  template <typename C>
  void Sort();

  /// Sort with default comparer
  void Sort();

  /// Returns the allocator that is used by this instance.
  ezIAllocator* GetAllocator() const { return m_pAllocator; }

private:

  /// A common constructor function.
  void Constructor(ezIAllocator* pAllocator);

  /// Reduces the index array to take up less memory.
  void CompactIndexArray(ezUInt32 uiMinChunksToKeep);

  /// Moves the index chunk array to the left by swapping all elements from right to left
  void MoveIndexChunksLeft(ezUInt32 uiChunkDiff);

  /// Moves the index chunk array to the right by swapping all elements from left to right
  void MoveIndexChunksRight(ezUInt32 uiChunkDiff);

  /// Computes which chunk contains the element at index 0
  ezUInt32 GetFirstUsedChunk() const;

  /// Computes which chunk would contain the last element if the deque had 'uiAtSize' elements (m_uiCount), returns the chunk of element 0, if the deque is currently empty.
  ezUInt32 GetLastUsedChunk(ezUInt32 uiAtSize) const;

  /// Returns which chunk contains the currently last element.
  ezUInt32 GetLastUsedChunk() const;

  /// Computes how many chunks would be required if the deque had a size of 'uiAtSize' (the value of m_uiFirstElement) affects the result.
  ezUInt32 GetRequiredChunks(ezUInt32 uiAtSize) const;

  /// Goes through all the unused chunks and deallocates chunks until no more than 'uiMaxChunks' are still allocated (in total).
  void DeallocateUnusedChunks(ezUInt32 uiMaxChunks);

  /// Resets the counter when the next size reduction will be done.
  void ResetReduceSizeCounter();

  /// This function will periodically deallocate unused chunks to prevent unlimited memory usage. 
  /// However it tries not to deallocate too much, so it does not immediately deallocate 'everything'.
  /// Instead the deque will track its upper limited of used elements in between size reductions and keep enough
  /// chunks allocated to fulfill those requirements.
  /// If after a large amount of data was in use, the deque's size decreases drastically, the amount of chunks
  /// is reduced gradually (but not immediately).
  /// Size reductions are mostly triggered by PopBack and PopFront (but also by SetCount).
  /// The worst case scenario should be this:
  /// * SetCount(very large amount) -> allocates lots of chunks
  /// * PopBack / PopFront until the deque is empty -> every once in a while a reduction is triggered, this will deallocate a few chunks every time
  /// * PushBack / PopBack for a long time -> Size does not change, but the many PopBack calls will trigger reductions 
  ///    -> The number of allocated chunks will shrink over time until no more than the required number of chunks (+2) remains
  /// * SetCount(very large amount) -> lots of chunks need to be allocated again
  void ReduceSize(ezInt32 iReduction);

  /// Computes how many elements could be handled without rearranging the index array
  ezUInt32 GetCurMaxCount() const;

  /// Searches through the unused chunks for an allocated chunk and returns it. Allocates a new chunk if necessary.
  T* GetUnusedChunk();

  /// Returns a reference to the element at the given index. Makes sure the chunk that should contain that element is allocated. Used before elements are constructed.
  T& ElementAt(ezUInt32 uiIndex);

  /// Deallocates all data, resets the deque to the state after construction.
  void DeallocateAll();

  ezIAllocator* m_pAllocator;
  T** m_pChunks;             ///< The chunk index array for redirecting accesses. Not all chunks must be allocated.
  ezUInt32 m_uiChunks;          ///< The size of the m_pChunks array. Determines how many elements could theoretically be stored in the deque.
  ezUInt32 m_uiFirstElement;    ///< Which element (across all chunks) is considered to be the first.
  ezUInt32 m_uiCount;           ///< How many elements are actually active at the moment.
  ezUInt32 m_uiAllocatedChunks; ///< How many entries in the m_pChunks array are allocated at the moment.
  ezInt32 m_iReduceSizeTimer;   ///< Every time this counter reaches zero, a 'garbage collection' step is performed, which might deallocate chunks.
  ezUInt32 m_uiMaxCount;        ///< How many elements were maximally active since the last 'garbage collection' to prevent deallocating too much memory.
};


template <typename T, typename AllocatorWrapper = ezDefaultAllocatorWrapper, bool Construct = true>
class ezDeque : public ezDequeBase<T, Construct>
{
public:
  ezDeque();
  ezDeque(ezIAllocator* pAllocator);

  ezDeque(const ezDeque<T, AllocatorWrapper, Construct>& other);
  ezDeque(const ezDequeBase<T, Construct>& other);

  void operator=(const ezDeque<T, AllocatorWrapper, Construct>& rhs);
  void operator=(const ezDequeBase<T, Construct>& rhs);
};

#include <Foundation/Containers/Implementation/Deque_inl.h>

