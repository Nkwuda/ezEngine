#pragma once

#include <Foundation/Containers/DynamicArray.h>

/// This class allows to propagate events to code that might be interested in them.
/// An event can be anything that "happens" that might be of interest for other code, such
/// that it can react on it in some way.
/// Just create an instance of ezEvent and call "Broadcast" on it. Other intersted code needs access to
/// the variable (or at least to AddEventHandler / RemoveEventHandler) such that it can
/// register itself as an interested party. To pass information to the handlers, create your own
/// custom struct to package that information and then pass a pointer to that data through Broadcast.
/// The handlers just need to cast the void-pointer to the proper struct and thus can get all the detailed
/// information about the event.
template <typename EventData, typename PassThrough>
class ezEventBase
{
protected:
  /// Constructor.
  ezEventBase(ezIAllocator* pAllocator);

public:
  /// Notification callback type for events.
  typedef void (*ezEventHandler)(EventData pEventData, PassThrough pPassThrough);

  /// This function will broadcast to all registered users, that this event has just happened.
  void Broadcast(EventData pEventData); // [tested]

  /// Adds a function as an event handler. All handlers will be notified in the order that they were registered.
  void AddEventHandler(ezEventHandler callback, PassThrough pPassThrough); // [tested]

  /// Removes a previously registered handler. It is an error to remove a handler that was not registered.
  /// The exact same data that was passed to AddEventHandler (including pPassThrough) must be given.
  void RemoveEventHandler(ezEventHandler callback, PassThrough pPassThrough); // [tested]

  EZ_DISALLOW_COPY_AND_ASSIGN(ezEventBase);

private:
  /// Used to detect recursive broadcasts and then throw asserts at you.
  bool m_bBroadcasting;

  struct ezEventReceiver
  {
    EZ_DECLARE_POD_TYPE();

    ezEventHandler m_Callback;
    PassThrough m_pPassThrough;
  };

  /// A dynamic array allows to have zero overhead as long as no event receivers are registered.
  ezDynamicArray<ezEventReceiver> m_EventHandlers;
};

template <typename EventData, typename PassThrough = void*, typename AllocatorWrapper = ezDefaultAllocatorWrapper>
class ezEvent : public ezEventBase<EventData, PassThrough>
{
public:
  ezEvent();
  ezEvent(ezIAllocator* pAllocator);
};

#include <Foundation/Communication/Implementation/Event_inl.h>
