# TraCR examples

The following examples are provided:

- `thread_markers.cpp`: This is the most commonly used example. The so-called thread markers are stored on the main thread.
If your application uses multiple threads (e.g., with Pthreads), see `pthread_example.cpp`.

- `pthread_example.cpp`: Similar to `thread_markers.cpp`, but runs across multiple threads using Pthreads.
In this case, threads must be explicitly initialized.

- `vmarkers_push_pop.cpp`: Equivalent to `thread_markers.cpp`, but uses VMARKERS (short for vanilla markers).
These are lightweight markers that are pushed using a color ID instead of a marker ID. Useful when working with many different markers.

- `vmarkers_set.cpp`: Another VMARKERS example, this time using the SET method instead of PUSH/POP.
The PUSH/POP method relies on each marker being explicitly pushed and later popped, whereas the SET method simply sets the marker without requiring an end point (i.e. a pop).