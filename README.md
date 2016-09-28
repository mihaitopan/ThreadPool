# ThreadPool
Thread pool dynamic library I created to use when programming multithreading.

It has a abstract class named IWorkItem and it's method Execute() which are to be overwritten in order to give the worker threads the desired job.
The library exports 2 functions: CreateObject() and DestroyObject(), which are use to instantiate the thread pool.
A tester and also an example of how to use the library is given by the ThreadPoolTester project.
