namespace M3D {
namespace Math {
template<typename T, size_t N>
class Matrix {
public:
    static constexpr size_t order = N;
    using value_type = T;

    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    Matrix() = default;
    Matrix(Matrix&&) = default; // move

    Matrix& operator=(Matrix&&) = default;
    Matrix(Matrix const&) = default; // copy
    Matrix& operator=(Matrix const&) = default;

    ˜Matrix() = default;

    template<typename U>
    Matrix(const Matrix_ref<U, N>&); // construct from Matrix_ref

    template<typename U>
    Matrix& operator=(const Matrix_ref<U, N>&); // assign from Matrix_ref

    template<typename... Exts> // specify the extents
    explicit Matrix(Exts... exts);

    Matrix(Matrix_initializer<T, N>); // initialize from list

    Matrix& operator=(Matrix_initializer<T, N>); // assign from list

    template<typename U>
    Matrix(initializer_list<U>) = delete; // don’t use {} except for elements

    template<typename U>
    Matrix& operator=(initializer_list<U>) = delete;

    static constexpr size_t order() { return N; } // number of dimensions

    size_t extent(size_t n) const { return desc.extents[n]; } // #elements in the nth dimension

    size_t size() const { return elems.size(); } // total number of elements

    const Matrix_slice<N>& descriptor() const { return desc; } // the slice defining subscripting

    T∗ data() { return elems.data(); } // ‘‘flat’’ element access

    const T∗ data() const { return elems.data(); }

    /*
     * Subscripting and slicing
     */
    template<typename... Args> // m(i,j,k) subscripting with integers
    Enable_if<Matrix_impl::Requesting_element<Args...>(), T&>
    operator()(Args... args);

    template<typename... Args>
    Enable_if<Matrix_impl::Requesting_element<Args...>(), const T&>
    operator()(Args... args) const;

    template<typename... Args> // m(s1,s2,s3) subscripting with slices
    Enable_if<Matrix_impl::Requesting_slice<Args...>(), Matrix_ref<T, N>>
            operator()(const Args&... args);

    template<typename... Args>
    Enable_if<Matrix_impl::Requesting_slice<Args...>(), Matrix_ref<const T, N>>
            operator()(const Args&... args) const;

    Matrix_ref < T, N−1 > operator[](size_t i) { return row(i); } // m[i] row access
    Matrix_ref < const T, N−1 > operator[](size_t i) const { return row(i); }
    Matrix_ref < T, N−1 > row(size_t n); // row access
    Matrix_ref < const T, N−1 > row(siz e_t n) const;
    Matrix_ref < T, N−1 > col(size_t n); // column access
    Matrix_ref < const T, N−1 > col(size_t n) const;

    /* Arithmetic Operation */
    template<typename F>
    Matrix& apply(F f); // f(x) for every element x
    template<typename M, typename F>
    Matrix& apply(const M& m, F f); // f(x,mx) for corresponding elements
    Matrix& operator=(const T& value); // assignment with scalar
    Matrix& operator+=(const T& value); // scalar addition
    Matrix& operator−=(const T& value); // scalar subtraction
    Matrix& operator∗=(const T& value); // scalar multiplication
    Matrix& operator/=(const T& value); // scalar division
    Matrix& operator%=(const T& value); // scalar modulo
    template<typename M> // matrix addition
    Matrix& operator+=(const M& x);
    template<typename M> // matrix subtraction
    Matrix& operator−=(const M& x);

private:
    Matrix_slice<N> desc; // slice defining extents in the N dimensions
    vector<T> elems; // the elements
};

} // namespace Math
} // namespace M3D
