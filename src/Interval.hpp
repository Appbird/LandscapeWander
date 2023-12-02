#include <assert.h>

template<typename T>
struct Interval {
    T begin;
    T end;
    Interval(const T& begin_, const T& end_):
        begin(begin_),
        end(end_)
    {
        assert(begin < end);
    }
    T length() const    { return end - begin; }
    T last() const      { return end - 1; }
};