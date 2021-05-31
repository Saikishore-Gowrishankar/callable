# callable
An implementation of a polymorphic function wrapper that works with std::packaged_task

# Explanation
The standard std::function in the standard library does not allow for move-only types to be used. Thus, this simple generic class solves the trick. 

# Patterns used
The generic template class implements traditional template type erasure to allow the wrapper to hold any type of callable object (unless it is a member function pointer). The class uses the bridge pattern for the stored callable object, and it includes a factory function to easily generate callable objects

