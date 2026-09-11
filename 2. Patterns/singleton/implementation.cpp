#include <bits/stdc++.h>
using namespace std;

/*
Singleton ensures:
    Only ONE instance of a class exists.
    Provides a global access point to that instance, i.e. It essentially turns the object into a "Single Source of Truth" that is available everywhere.

“We make the constructor private, prevent copying, and expose a static method that returns the single instance.”
this is always a pinter to current object

*/

class SingleTon
{
private:
    // private constructor, prevents: SingleTon* object = new SingleTon(); or SingleTon newObject;
    SingleTon()
    {
        // initialization logic
    }

    // prevent copy
    SingleTon(const SingleTon &) = delete;            // no copy constructor
    SingleTon &operator=(const Singleton &) = delete; // no Copy Assignment Operator, u can return Singleton also instead Singleton&, Singleton& just prevent extra copy creation. Also u can skip return also, in that case, chaining like a=b=c; will not work

public:
    static SingleTon &getInstance() // & is very important here, otherwise copy constructor will be called
    {
        // Before C++11, this code was not thread-safe. If two threads called getInstance() at the exact same time, they could both try to initialize the instance variable, leading to a race condition.
        static Singleton instance; // runs only for first time getInstance got called, skips for all other times
        return instance;
    }
};

int main()
{
    Singleton &s1 = Singleton::getInstance();
    Singleton &s2 = Singleton::getInstance();

    // Both point to the same instance
    return 0;
}