# TraCR examples

The following examples are provided:

- `thread_markers.*`: This is the most commonly used example. The so-called thread markers are stored on the main thread.
If your application uses multiple threads (e.g., with Pthreads), see `pthread_example.*`.

- `pthread_example.*`: Similar to `thread_markers.*`, but runs across multiple threads using Pthreads.
In this case, threads must be explicitly initialized.

- `vmarkers_push_pop.*`: Equivalent to `thread_markers.*`, but uses VMARKS (short for vanilla markers).
These are lightweight markers that are pushed using a color ID instead of a marker ID. Useful when working with many different markers.

- `vmarkers_set.*`: Another VMARKS example, this time using the SET method instead of PUSH/POP.
The PUSH/POP method relies on each marker being explicitly pushed and later popped, whereas the SET method simply sets the marker without requiring an end point (i.e. a pop).