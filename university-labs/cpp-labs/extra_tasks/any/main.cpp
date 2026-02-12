#include "task.h"

#if defined(_GLIBCXX_ANY) || defined(_LIBCPP_ANY)
#error "include any forbidden"
#else
#include <any>
#endif

#include <cassert>
#include <string>
#include <typeinfo>
#include <vector>

#define ASSERT_THROW(expr, exception) \
do {                                  \
    try {                             \
        expr;                         \
        assert(!"no exception");      \
    } catch (const exception&) {      \
    } catch (...) {                   \
        assert(!"other exception");   \
    }                                 \
} while (false)

struct SimpleStruct {
    int IntField;
};

static_assert(sizeof(Any) == sizeof(void*));

void SimpleTest() {
    {
        Any any(5);
        assert(!any.Empty());
        assert(any.Value<int>() == 5);
    }

    {
        Any any(std::string("abcde"));
        assert(any.Value<std::string>() == "abcde");
    }

    {
        Any any;
        any = 5.5;
        assert(!any.Empty());
        assert(any.Value<double>() == 5.5);
    }

    {
        Any any;
        any = nullptr;
        assert(!any.Empty());
        any.Reset();
        assert(any.Empty());
        int* ptr = nullptr;
        any = ptr;
        assert(!any.Empty());
    }
}

void EmptyTest() {
    Any first;
    assert(first.Empty());

    std::vector<int> vec{1, 2, 3, 4, 5};
    Any second(vec);
    assert(vec.size() == 5);
    assert(!second.Empty());

    first.Swap(second);
    assert(first.Value<std::vector<int>>() == vec);
    assert(!first.Empty());
    assert(second.Empty());

    first.Reset();
    assert(first.Empty());
}

void CopyTest() {
    {
        Any a(5);
        Any b = a;

        assert(a.Value<int>() == 5);
        assert(a.Value<int>() == b.Value<int>());

        Any c;
        c = b;

        assert(b.Value<int>() == c.Value<int>());
        b.Reset();
        assert(c.Value<int>() == 5);

        Any e = Any(std::string("abcde"));
        e = [&e]() -> const Any& { return e; }();

        assert(e.Value<std::string>() == "abcde");

        a.Swap(e);
        assert(e.Value<int>() == 5);
        assert(a.Value<std::string>() == "abcde");
    }

    {
        Any any(SimpleStruct{5});
        assert(any.Value<SimpleStruct>().IntField == 5);

        any = std::string("abcde");
        assert(any.Value<std::string>() == "abcde");
    }
}

void MoveTest() {
    Any a(SimpleStruct{5});
    assert(a.Value<SimpleStruct>().IntField == 5);

    Any b(std::string("abcde"));
    a = std::move(b);

    assert(b.Empty());
    assert(a.Value<std::string>() == "abcde");

    Any c(std::move(a));
    assert(a.Empty());
    assert(c.Value<std::string>() == "abcde");
}

void ThrowTest() {
    {
        Any any(std::string("abcde"));
        ASSERT_THROW(any.Value<int>(), std::bad_cast);
    }

    {
        Any any(std::vector<int>{1, 2, 3, 4, 5});
        assert(any.Value<std::vector<int>>().size() == 5);
        ASSERT_THROW(any.Value<std::string>(), std::bad_cast);
    }
}

int main() {
    SimpleTest();
    EmptyTest();
    CopyTest();
    MoveTest();
    ThrowTest();

    return 0;
}
