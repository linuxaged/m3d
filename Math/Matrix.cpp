#include "Matrix.h"
#include <Array>
namespace M3D {
namespace Math {
    template <size_t N>
    struct Matrix_slice {
        Matrix_slice() = default; // an empty matrix: no elements
        Matrix_slice(size_t s, initializer_list<size_t> exts); // extents
        Matrix_slice(size_t s, initializer_list<size_t> exts, initializer_list<size_t> strs); // extents and strides template<typename... Dims> // N extents
        Matrix_slice(Dims... dims);
        template <typename... Dims,
            typename = Enable_if<All(Convertible<Dims, size_t>()...)> >
        size_t operator()(Dims... dims) const; // calculate index from a set of subscripts
        size_t size;
        size_t start;
        array<size_t, N> extents;
        array<size_t, N> strides;
    };
    // total number of elements
    // starting offset
    // number of elements in each dimension
    // offsets between elements in each dimension

    template <size_t N>
    template <typename... Dims>
    size_t Matrix_slice<N>::operator()(Dims... dims) const
    {
        static_assert(sizeof...(Dims) == N, "");
        size_t args[N]{ size_t(dims)... }; // Copy arguments into an array
        return inner_product(args, args + N, strides.begin(), size_t(0));
    }

    template <typename T, size_t N>
    class Matrix_ref {
    public:
        Matrix_ref(const Matrix_slice<N>& s, T* p)
            : desc{ s }
            , ptr : { p }
        {
        }
        // ... mostly like Matrix ...

    private:
        Matrix_slice<N> desc; // the shape of the matrix
        T* ptr; // first element in the matrix
    };
}
}
