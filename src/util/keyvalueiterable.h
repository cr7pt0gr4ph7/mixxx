#pragma once

#include <QKeyValueIterator>
#include <type_traits>

/// Wrapper type that allows range-based for loops to be used with
/// QHash, QJsonDocument etc. Use it like this:
///
///     QHash<QString, int> myHash;
///     for (auto [key, value] : asKeyValueIterable(myHash)) {
///         // ...
///     }
template<typename T>
class KeyValueIterable final {
  public:
    inline KeyValueIterable(T& data)
            : m_data{data} {
    }

    typedef std::remove_cvref_t<T> WrappedType;
    typedef typename WrappedType::key_type key_type;
    typedef typename WrappedType::mapped_type mapped_type;
    typedef typename WrappedType::size_type size_type;

    typedef class QKeyValueIterator<key_type,
            mapped_type,
            typename WrappedType::const_iterator>
            const_iterator;

    typedef class QKeyValueIterator<
            key_type,
            mapped_type,
            typename WrappedType::iterator>
            iterator;

    inline auto begin() {
        // Return type is either iterator or const_iterator,
        // depending on the return type of m_data.begin()
        // (which may depend on the const-ness of T).
        return maybe_const_iterator(m_data.begin());
    }

    inline auto end() {
        // Return type is either iterator or const_iterator,
        // depending on the return type of m_data.end()
        // (which may depend on the const-ness of T).
        return maybe_const_iterator(m_data.end());
    }

    inline const_iterator cbegin() const {
        return const_iterator(m_data.cbegin());
    }

    inline const_iterator cend() const {
        return const_iterator(m_data.cend());
    }

    inline size_type size() {
        return m_data.size();
    }

  private:
    inline iterator maybe_const_iterator(WrappedType::iterator i) {
        return iterator(i);
    }

    inline const_iterator maybe_const_iterator(WrappedType::const_iterator i) {
        return const_iterator(i);
    }

  private:
    T& m_data;
};

template<typename WrappedType>
KeyValueIterable<const WrappedType> asKeyValueIterable(const WrappedType& data) {
    return KeyValueIterable<const WrappedType>(data);
}

template<typename WrappedType>
KeyValueIterable<WrappedType> asMutableKeyValueIterable(WrappedType& data) {
    return KeyValueIterable<WrappedType>(data);
}
