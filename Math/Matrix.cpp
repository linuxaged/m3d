#include "Matrix.h"
#include <Array>
namespace M3D {
namespace Math {
namespace Matrix_Impl {

struct slice {
    slice() : start(−1), length(−1), stride(1) { }
    explicit slice(size_t s) : start(s), length(−1), stride(1) { }
    slice(size_t s, size_t l, size_t n = 1) : start(s), length(l), stride(n) { }
    size_t operator()(size_t i) const { return start + i∗stride; }
    static slice all;
    size_t start; // first index
    size_t length; // number of indices included (can be used for range checking)
    size_t stride; // distance between elements in sequence
};

template<size_t N>
struct Matrix_slice {
    Matrix_slice() = default; // an empty matrix: no elements
    Matrix_slice(size_t s, initializer_list<size_t> exts); // extents
    Matrix_slice(size_t s, initializer_list<siz e_t> exts, initializer_list<size_t> strs);// extents and strides
    template<typename... Dims> // N extents
    Matrix_slice(Dims... dims);
    template<typename... Dims,
             typename = Enable_if<All(Convertible<Dims, siz e_t>()...)>>
    size_t operator()(Dims... dims) const; // calculate index from a set of subscripts
    size_t size; // total number of elements
    size_t start; // star ting offset
    array<size_t, N> extents; // number of elements in each dimension
    array<size_t, N> strides; // offsets between elements in each dimension
};

template<size_t N>
template<typename... Dims>
size_t Matrix_slice<N>::operator()(Dims... dims) const
{
    static_assert(sizeof...(Dims) == N, "");
    size_t args[N] { size_t(dims)... }; // Copy arguments into an array
    return inner_product(args, args + N, strides.begin(), size_t(0));
}

template<>
struct Matrix_slice<1> {
    // ...
    size_t operator()(size_t i) const
    {
        return i;
    }
}
template<>
struct Matrix_slice<2> {
    // ...
    size_t operator()(size_t i, size_t j) const
    {
        return i∗stides[0] + j;
    }
}


template <typename T, size_t N>
class Matrix_ref {
public:
    Matrix_ref(const Matrix_slice<N>& s, T* p)
        : desc{ s }
    , ptr : { p }
    {
    }
    // TODO:
    // mostly like Matrix, derive from a common base

private:
    Matrix_slice<N> desc; // the shape of the matrix
    T* ptr; // first element in the matrix
};

template<typename T, size_t N>
struct Matrix_init {
    using type = initializer_list < typename Matrix_init < T, N−1 >::type >;
};

template<typename T>
struct Matrix_init<T, 1> {
    using type = initializer_list<T>;
};

template<typename T>
struct Matrix_init<T, 0>; // undefined on purpose

template<size_t N, typename List>
array<size_t, N> derive_extents(const List& list)
{
    array<siz e_t, N> a;
    auto f = a.begin();
    add_extents<N>(f, list); // put extents from list into f[]
    return a;
}

template<size_t N, typename I, typename List>
Enable_if < (N > 1), void > add_extents(I& first, const List& list)
{
    assert(check_non_jagged(list));
    ∗first = list.size();
    add_extents < N−1 > (++first, ∗list.begin());
}

template<size_t N, typename I, typename List>
Enable_if<(N == 1), void> add_extents(I& first, const List& list)
{
    ∗first++ = list.size(); // we reached the deepest nesting
}

template<typename List>
bool check_non_jagged(const List& list)
{
    auto i = list.begin();
    for (auto j = i + 1; j != list.end(); ++j)
        if (i− > size() != j− > size())
            return false;
    return true;
}

template<typename T, typename Vec>
void insert_flat(initializer_list<T> list, Vec& vec)
{
    add_list(list.begin(), list.end(), vec);
}

template<typename T, typename Vec> // nested initializer_lists
void add_list(const initializer_list<T>∗ first, const initializer_list<T>∗ last, Vec& vec)
{
    for (; first != last; ++first)
        add_list(first− > begin(), first− > end(), vec);
}

template<typename T, typename Vec>
void add_list(const T∗ first, const T∗ last, Vec& vec)
{
    vec.insert(vec.end(), first, last);
}

} // namespace Matrix_Impl


template<typename T, size_t N>
template<typename... Exts>
Matrix<T, N>::Matrix(Exts... exts)
    : desc{exts...}, // copy extents
      elems(desc.size) // allocate desc.size elements and default initialize them
{ }

template<typename T, size_t N>
using Matrix_initializer = typename Matrix_impl::Matrix_init<T, N>::type;

template<typename T, size_t N>
Matrix<T, N>::Matrix(Matrix_initializer<T, N> init)
{
    Matrix_impl::derive_extents(init, desc.extents); // deduce extents from initializer list (§29.4.4)
    elems.reserve(desc.size); // make room for slices
    Matrix_impl::insert_flat(init, elems); // initialize from initializer list (§29.4.4)
    assert(elems.size() == desc.size);
}

template<typename T, size_t N>
template<typename U>
Matrix<T, N>::Matrix(const Matrix_ref<U, N>& x)
    : desc{x.desc}, elems{x.begin(), x.end()} // copy desc and elements
{
    static_assert(Convertible<U, T>(), "Matrix constructor: incompatible element types");
}

template<typename T, size_t N>
template<typename U>
Matrix<T, N>& Matrix<T, N>::operator=(const Matrix_ref<U, N>& x)
{
    static_assert(Convertible<U, T>(), "Matrix =: incompatible element types");
    desc = x.desc;
    elems.assign(x.begin(), x.end());
    return ∗this;
}

/* Subscripting and slicing */
template<typename T, size_t N>
Matrix_ref < T, N−1 > Matrix<T, N>::operator[](size_t n)
{
    return row(n); // §29.4.5
}

/* Arithmetic Operation */
template<typename T, size_t N>
template<typename F>
Matrix<T, N>& Matrix<T, N>::apply(F f)
{
    for (auto& x : elems) f(x); // this loop uses stride iterators
    return ∗this;
}

template<typename T, size_t N>
Matrix<T, N>& Matrix<T, N>::operator+=(const T& val)
{
    return apply([&](T & a) { a += val; } ); // using a lambda (§11.4)
}

template<typename T, size_t N>
Matrix<T, N> operator+(const Matrix<T, N>& m, const T& val)
{
    Matrix<T, N> res = m;
    res += val;
    return res;
}

// Addition
template<typename T, size_t N>
template<typename M, typename F>
Enable_if<Matrix_type<M>(), Matrix<T, N>&> Matrix<T, N>::apply(M& m, F f)
{
    assert(same_extents(desc, m.descriptor())); // make sure sizes match
    for (auto i = begin(), j = m.begin(); i != end(); ++i, ++j)
        f(∗i, ∗j);
    return ∗this;
}

template<typename T, size_t N>
template<typename M>
Enable_if<Matrix_type<M>(), Matrix<T, N>&> Matrix<T, N>::operator+=(const M& m)
{
    static_assert(m.order() == N, "+=: mismatched Matrix dimensions");
    assert(same_extents(desc, m.descriptor())); // make sure sizes match
    return apply(m, [](T & a, Value_type<M>&b) { a += b; });
}

template<typename T, size_t N>
Matrix<T, N> operator+(const Matrix<T, N>& a, const Matrix<T, N>& b)
{
    Matrix<T, N> res = a;
    res += b;
    return res;
}

template < typename T, typename T2, size_t N,
           typename RT = Matrix<Common_type<Value_type<T>, Value_type<T2>>, N>
           Matrix<RT, N> operator+(const Matrix<T, N>& a, const Matrix<T2, N>& b)
{
    Matrix<RT, N> res = a;
    res += b;
    return res;
}

template<typename T, size_t N>
Matrix<T, N> operator+(const Matrix_ref<T, N>& x, const T& n)
{
    Matrix<T, N> res = x;
    res += n;
    return res;
}




} // namespace Math
} // namespace M3D
