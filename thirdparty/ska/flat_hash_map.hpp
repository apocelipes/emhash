//          Copyright Malte Skarupke 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <utility>
#include <type_traits>

#ifdef _MSC_VER
#define SKA_NOINLINE(...) __declspec(noinline) __VA_ARGS__
#else
#define SKA_NOINLINE(...) __VA_ARGS__ __attribute__((noinline))
#endif

namespace ska
{
struct prime_number_hash_policy;
struct power_of_two_hash_policy;
struct fibonacci_hash_policy;

template <typename T>
struct remove_cvref : public std::remove_cv<typename std::remove_reference<T>::type> {
};

template <typename T1, typename T2>
struct is_same_ex : public std::is_same<typename remove_cvref<T1>::type,
                                        typename remove_cvref<T2>::type> {
};

namespace detailv3
{
template<typename Result, typename Functor>
struct functor_storage : Functor
{
    functor_storage() = default;
    functor_storage(const Functor & functor)
        : Functor(functor)
    {
    }
    template<typename... Args>
    Result operator()(Args &&... args)
    {
        return static_cast<Functor &>(*this)(std::forward<Args>(args)...);
    }
    template<typename... Args>
    Result operator()(Args &&... args) const
    {
        return static_cast<const Functor &>(*this)(std::forward<Args>(args)...);
    }
};
template<typename Result, typename... Args>
struct functor_storage<Result, Result (*)(Args...)>
{
    typedef Result (*function_ptr)(Args...);
    function_ptr function;
    functor_storage(function_ptr function)
        : function(function)
    {
    }
    Result operator()(Args... args) const
    {
        return function(std::forward<Args>(args)...);
    }
    operator function_ptr &()
    {
        return function;
    }
    operator const function_ptr &()
    {
        return function;
    }
};
template<typename key_type, typename value_type, typename hasher>
struct KeyOrValueHasher : functor_storage<size_t, hasher>
{
    typedef functor_storage<size_t, hasher>         hasher_storage;
    typedef typename value_type::first_type         first_type;
    typedef typename value_type::second_type        second_type;
    typedef std::pair<first_type *, second_type>    value_type_2nd;

    hasher hasher_;

    KeyOrValueHasher() = default;
    KeyOrValueHasher(const hasher & hash)
        : hasher_storage(hash), hasher_(hash)
    {
    }

#if 0
    template <typename Arg, typename std::enable_if<!ska::is_same_ex<Arg, key_type>::value &&
                            std::is_constructible<key_type, typename ska::remove_cvref<Arg>::type
                            >::value>::type * = nullptr>
    size_t operator()(const Arg & arg)
    {
        key_type key(arg);
        return hasher_(key);
    }

    template <typename Arg, typename std::enable_if<!ska::is_same_ex<Arg, key_type>::value &&
                            std::is_constructible<key_type, typename ska::remove_cvref<Arg>::type
                            >::value>::type * = nullptr>
    size_t operator()(const Arg & arg) const
    {
        key_type key(arg);
        return hasher_(key);
    }
#endif

    template <typename KeyType, typename std::enable_if<
                                    ska::is_same_ex<key_type, KeyType>::value
                                  >::type * = nullptr>
    size_t operator()(const KeyType & key)
    {
        return static_cast<hasher_storage &>(*this)(key);
    }

    template <typename KeyType, typename std::enable_if<
                                    ska::is_same_ex<key_type, KeyType>::value
                                  >::type * = nullptr>
    size_t operator()(const KeyType & key) const
    {
        return static_cast<const hasher_storage &>(*this)(key);
    }

    template <typename KeyType, typename std::enable_if<
                                    ska::is_same_ex<key_type, KeyType>::value
                                  >::type * = nullptr>
    size_t operator()(const KeyType * key)
    {
        return static_cast<hasher_storage &>(*this)(key);
    }

    template <typename KeyType, typename std::enable_if<
                                    ska::is_same_ex<key_type, KeyType>::value
                                  >::type * = nullptr>
    size_t operator()(const KeyType * key) const
    {
        return static_cast<const hasher_storage &>(*this)(key);
    }

    template <typename FirstType, typename std::enable_if<
                                    !ska::is_same_ex<key_type, FirstType>::value
                                  >::type * = nullptr>
    size_t operator()(const FirstType & key)
    {
        return static_cast<hasher_storage &>(*this)(key);
    }

    template <typename FirstType, typename std::enable_if<
                                    !ska::is_same_ex<key_type, FirstType>::value
                                  >::type * = nullptr>
    size_t operator()(const FirstType & key) const
    {
        return static_cast<const hasher_storage &>(*this)(key);
    }

    template <typename FirstType, typename std::enable_if<
                                    !ska::is_same_ex<key_type, FirstType>::value
                                  >::type * = nullptr>
    size_t operator()(const FirstType * key)
    {
        return static_cast<hasher_storage &>(*this)(key);
    }

    template <typename FirstType, typename std::enable_if<
                                    !ska::is_same_ex<key_type, FirstType>::value
                                  >::type * = nullptr>
    size_t operator()(const FirstType * key) const
    {
        return static_cast<const hasher_storage &>(*this)(key);
    }

    size_t operator()(const value_type & value)
    {
        return static_cast<hasher_storage &>(*this)(value.first);
    }

    size_t operator()(const value_type & value) const
    {
        return static_cast<const hasher_storage &>(*this)(value.first);
    }

    size_t operator()(const value_type_2nd & value)
    {
        return static_cast<hasher_storage &>(*this)(value.first);
    }

    size_t operator()(const value_type_2nd & value) const
    {
        return static_cast<const hasher_storage &>(*this)(value.first);
    }

    template<typename F, typename S>
    size_t operator()(const std::pair<F, S> & value)
    {
        return static_cast<hasher_storage &>(*this)(value.first);
    }

    template<typename F, typename S>
    size_t operator()(const std::pair<F, S> & value) const
    {
        return static_cast<const hasher_storage &>(*this)(value.first);
    }
};
template<typename key_type, typename value_type, typename key_equal>
struct KeyOrValueEquality : functor_storage<bool, key_equal>
{
    typedef functor_storage<bool, key_equal>        equality_storage;
    typedef typename value_type::first_type         first_type;
    typedef typename value_type::second_type        second_type;
    typedef std::pair<first_type *, second_type>    value_type_2nd;

    KeyOrValueEquality() = default;
    KeyOrValueEquality(const key_equal & equality)
        : equality_storage(equality)
    {
    }

    template <typename KeyType, typename std::enable_if<
                                    ska::is_same_ex<key_type, KeyType>::value
                                  >::type * = nullptr>
    bool operator()(const KeyType & lhs, const KeyType & rhs)
    {
        return static_cast<equality_storage &>(*this)(lhs, rhs);
    }

    template <typename FirstType, typename std::enable_if<
                                    !ska::is_same_ex<key_type, FirstType>::value
                                  >::type * = nullptr>
    bool operator()(const FirstType & lhs, const FirstType & rhs)
    {
        return static_cast<equality_storage &>(*this)(lhs, rhs);
    }

    bool operator()(const key_type & lhs, const value_type & rhs)
    {
        return static_cast<equality_storage &>(*this)(lhs, rhs.first);
    }

    bool operator()(const value_type & lhs, const key_type & rhs)
    {
        return static_cast<equality_storage &>(*this)(lhs.first, rhs);
    }

    bool operator()(const value_type & lhs, const value_type & rhs)
    {
        return static_cast<equality_storage &>(*this)(lhs.first, rhs.first);
    }

    template<typename F, typename S>
    bool operator()(const key_type & lhs, const std::pair<F, S> & rhs)
    {
        return static_cast<equality_storage &>(*this)(lhs, rhs.first);
    }

    template<typename F, typename S>
    bool operator()(const std::pair<F, S> & lhs, const key_type & rhs)
    {
        return static_cast<equality_storage &>(*this)(lhs.first, rhs);
    }

    template<typename F, typename S>
    bool operator()(const value_type & lhs, const std::pair<F, S> & rhs)
    {
        return static_cast<equality_storage &>(*this)(lhs.first, rhs.first);
    }

    template<typename F, typename S>
    bool operator()(const std::pair<F, S> & lhs, const value_type & rhs)
    {
        return static_cast<equality_storage &>(*this)(lhs.first, rhs.first);
    }

    template<typename FL, typename SL, typename FR, typename SR>
    bool operator()(const std::pair<FL, SL> & lhs, const std::pair<FR, SR> & rhs)
    {
        return static_cast<equality_storage &>(*this)(lhs.first, rhs.first);
    }
};
static constexpr int8_t min_lookups = 4;
template<typename T>
struct sherwood_v3_entry
{
    sherwood_v3_entry()
    {
    }
    sherwood_v3_entry(int8_t distance_from_desired)
        : distance_from_desired(distance_from_desired)
    {
    }
    ~sherwood_v3_entry()
    {
    }
    static sherwood_v3_entry * empty_default_table()
    {
        static sherwood_v3_entry result[min_lookups] = { {}, {}, {}, {special_end_value} };
        return result;
    }

    bool has_value() const
    {
        return distance_from_desired >= 0;
    }
    bool is_empty() const
    {
        return distance_from_desired < 0;
    }
    bool is_at_desired_position() const
    {
        return distance_from_desired <= 0;
    }
    template<typename... Args>
    void emplace(int8_t distance, Args &&... args)
    {
        new (std::addressof(value)) T(std::forward<Args>(args)...);
        distance_from_desired = distance;
    }

    void destroy_value()
    {
        value.~T();
        distance_from_desired = -1;
    }

    int8_t distance_from_desired = -1;
    static constexpr int8_t special_end_value = 0;
    union { T value; };
};

inline int8_t log2(size_t value)
{
    static constexpr int8_t table[64] =
    {
        63,  0, 58,  1, 59, 47, 53,  2,
        60, 39, 48, 27, 54, 33, 42,  3,
        61, 51, 37, 40, 49, 18, 28, 20,
        55, 30, 34, 11, 43, 14, 22,  4,
        62, 57, 46, 52, 38, 26, 32, 41,
        50, 36, 17, 19, 29, 10, 13, 21,
        56, 45, 25, 31, 35, 16,  9, 12,
        44, 24, 15,  8, 23,  7,  6,  5
    };
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return table[((value - (value >> 1)) * 0x07EDD5E59A4E28C2) >> 58];
}

template<typename T, bool>
struct AssignIfTrue
{
    void operator()(T & lhs, const T & rhs)
    {
        lhs = rhs;
    }
    void operator()(T & lhs, T && rhs)
    {
        lhs = std::move(rhs);
    }
};
template<typename T>
struct AssignIfTrue<T, false>
{
    void operator()(T &, const T &)
    {
    }
    void operator()(T &, T &&)
    {
    }
};

inline size_t next_power_of_two(size_t i)
{
    --i;
    i |= i >> 1;
    i |= i >> 2;
    i |= i >> 4;
    i |= i >> 8;
    i |= i >> 16;
    i |= i >> 32;
    ++i;
    return i;
}

template<typename...> using void_t = void;

template<typename T, typename = void>
struct HashPolicySelector
{
    typedef fibonacci_hash_policy type;
};
template<typename T>
struct HashPolicySelector<T, void_t<typename T::hash_policy>>
{
    typedef typename T::hash_policy type;
};

template<typename T, typename FindKey, typename ArgumentHash, typename Hasher, typename ArgumentEqual, typename Equal, typename ArgumentAlloc, typename EntryAlloc>
class sherwood_v3_table : private EntryAlloc, private Hasher, private Equal
{
    using Entry = detailv3::sherwood_v3_entry<T>;
    using AllocatorTraits = std::allocator_traits<EntryAlloc>;
    using EntryPointer = typename AllocatorTraits::pointer;
    struct convertible_to_iterator;

public:

    using value_type = T;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using hasher = ArgumentHash;
    using key_equal = ArgumentEqual;
    using allocator_type = EntryAlloc;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;

    sherwood_v3_table()
    {
    }
    explicit sherwood_v3_table(size_type bucket_count, const ArgumentHash & hash = ArgumentHash(), const ArgumentEqual & equal = ArgumentEqual(), const ArgumentAlloc & alloc = ArgumentAlloc())
        : EntryAlloc(alloc), Hasher(hash), Equal(equal)
    {
        rehash(bucket_count);
    }
    sherwood_v3_table(size_type bucket_count, const ArgumentAlloc & alloc)
        : sherwood_v3_table(bucket_count, ArgumentHash(), ArgumentEqual(), alloc)
    {
    }
    sherwood_v3_table(size_type bucket_count, const ArgumentHash & hash, const ArgumentAlloc & alloc)
        : sherwood_v3_table(bucket_count, hash, ArgumentEqual(), alloc)
    {
    }
    explicit sherwood_v3_table(const ArgumentAlloc & alloc)
        : EntryAlloc(alloc)
    {
    }
    template<typename It>
    sherwood_v3_table(It first, It last, size_type bucket_count = 0, const ArgumentHash & hash = ArgumentHash(), const ArgumentEqual & equal = ArgumentEqual(), const ArgumentAlloc & alloc = ArgumentAlloc())
        : sherwood_v3_table(bucket_count, hash, equal, alloc)
    {
        insert(first, last);
    }
    template<typename It>
    sherwood_v3_table(It first, It last, size_type bucket_count, const ArgumentAlloc & alloc)
        : sherwood_v3_table(first, last, bucket_count, ArgumentHash(), ArgumentEqual(), alloc)
    {
    }
    template<typename It>
    sherwood_v3_table(It first, It last, size_type bucket_count, const ArgumentHash & hash, const ArgumentAlloc & alloc)
        : sherwood_v3_table(first, last, bucket_count, hash, ArgumentEqual(), alloc)
    {
    }
    sherwood_v3_table(std::initializer_list<T> il, size_type bucket_count = 0, const ArgumentHash & hash = ArgumentHash(), const ArgumentEqual & equal = ArgumentEqual(), const ArgumentAlloc & alloc = ArgumentAlloc())
        : sherwood_v3_table(bucket_count, hash, equal, alloc)
    {
        if (bucket_count == 0)
            rehash(il.size());
        insert(il.begin(), il.end());
    }
    sherwood_v3_table(std::initializer_list<T> il, size_type bucket_count, const ArgumentAlloc & alloc)
        : sherwood_v3_table(il, bucket_count, ArgumentHash(), ArgumentEqual(), alloc)
    {
    }
    sherwood_v3_table(std::initializer_list<T> il, size_type bucket_count, const ArgumentHash & hash, const ArgumentAlloc & alloc)
        : sherwood_v3_table(il, bucket_count, hash, ArgumentEqual(), alloc)
    {
    }
    sherwood_v3_table(const sherwood_v3_table & other)
        : sherwood_v3_table(other, AllocatorTraits::select_on_container_copy_construction(other.get_allocator()))
    {
    }
    sherwood_v3_table(const sherwood_v3_table & other, const ArgumentAlloc & alloc)
        : EntryAlloc(alloc), Hasher(other), Equal(other), _max_load_factor(other._max_load_factor)
    {
        rehash_for_other_container(other);
        insert(other.begin(), other.end());
    }
    sherwood_v3_table(sherwood_v3_table && other) noexcept
        : EntryAlloc(std::move(other)), Hasher(std::move(other)), Equal(std::move(other))
    {
        swap_pointers(other);
    }
    sherwood_v3_table(sherwood_v3_table && other, const ArgumentAlloc & alloc) noexcept
        : EntryAlloc(alloc), Hasher(std::move(other)), Equal(std::move(other))
    {
        swap_pointers(other);
    }
    sherwood_v3_table & operator=(const sherwood_v3_table & other)
    {
        if (this == std::addressof(other))
            return *this;

        clear();
        if (AllocatorTraits::propagate_on_container_copy_assignment::value)
        {
            if (static_cast<EntryAlloc &>(*this) != static_cast<const EntryAlloc &>(other))
            {
                reset_to_empty_state();
            }
            AssignIfTrue<EntryAlloc, AllocatorTraits::propagate_on_container_copy_assignment::value>()(*this, other);
        }
        _max_load_factor = other._max_load_factor;
        static_cast<Hasher &>(*this) = other;
        static_cast<Equal &>(*this) = other;
        rehash_for_other_container(other);
        insert(other.begin(), other.end());
        return *this;
    }
    sherwood_v3_table & operator=(sherwood_v3_table && other) noexcept
    {
        if (this == std::addressof(other))
            return *this;
        else if (AllocatorTraits::propagate_on_container_move_assignment::value)
        {
            clear();
            reset_to_empty_state();
            AssignIfTrue<EntryAlloc, AllocatorTraits::propagate_on_container_move_assignment::value>()(*this, std::move(other));
            swap_pointers(other);
        }
        else if (static_cast<EntryAlloc &>(*this) == static_cast<EntryAlloc &>(other))
        {
            swap_pointers(other);
        }
        else
        {
            clear();
            _max_load_factor = other._max_load_factor;
            rehash_for_other_container(other);
            for (T & elem : other)
                emplace(std::move(elem));
            other.clear();
        }
        static_cast<Hasher &>(*this) = std::move(other);
        static_cast<Equal &>(*this) = std::move(other);
        return *this;
    }
    ~sherwood_v3_table()
    {
        clear();
        deallocate_data(entries, num_slots_minus_one, max_lookups);
    }

    const allocator_type & get_allocator() const
    {
        return static_cast<const allocator_type &>(*this);
    }
    const ArgumentEqual & key_eq() const
    {
        return static_cast<const ArgumentEqual &>(*this);
    }
    const ArgumentHash & hash_function() const
    {
        return static_cast<const ArgumentHash &>(*this);
    }

    template<typename ValueType>
    struct templated_iterator
    {
        templated_iterator() = default;
        templated_iterator(EntryPointer current)
            : current(current)
        {
        }
        EntryPointer current = EntryPointer();

        using iterator_category = std::forward_iterator_tag;
        using value_type = ValueType;
        using difference_type = ptrdiff_t;
        using pointer = ValueType *;
        using reference = ValueType &;

        friend bool operator==(const templated_iterator & lhs, const templated_iterator & rhs)
        {
            return lhs.current == rhs.current;
        }
        friend bool operator!=(const templated_iterator & lhs, const templated_iterator & rhs)
        {
            return !(lhs == rhs);
        }

        templated_iterator & operator++()
        {
            do
            {
                ++current;
            }
            while(current->is_empty());
            return *this;
        }
        templated_iterator operator++(int)
        {
            templated_iterator copy(*this);
            ++*this;
            return copy;
        }

        ValueType & operator*() const
        {
            return current->value;
        }
        ValueType * operator->() const
        {
            return std::addressof(current->value);
        }

        operator templated_iterator<const value_type>() const
        {
            return { current };
        }
    };
    using iterator = templated_iterator<value_type>;
    using const_iterator = templated_iterator<const value_type>;

    iterator begin()
    {
        for (EntryPointer it = entries;; ++it)
        {
            if (it->has_value())
                return { it };
        }
    }
    const_iterator begin() const
    {
        for (EntryPointer it = entries;; ++it)
        {
            if (it->has_value())
                return { it };
        }
    }
    const_iterator cbegin() const
    {
        return begin();
    }
    iterator end()
    {
        return { entries + static_cast<ptrdiff_t>(num_slots_minus_one + max_lookups) };
    }
    const_iterator end() const
    {
        return { entries + static_cast<ptrdiff_t>(num_slots_minus_one + max_lookups) };
    }
    const_iterator cend() const
    {
        return end();
    }

#if 1
    template <typename Arg, typename std::enable_if<!std::is_same<
                            typename std::remove_cv<typename std::remove_reference<Arg>::type>::type, FindKey>::value &&
                            std::is_constructible<FindKey, typename std::remove_cv<typename std::remove_reference<Arg>::type>::type
                            >::value>::type * = nullptr>
    iterator find(const Arg & arg)
    {
        FindKey key(arg);
        return find(std::move(key));
    }

    template <typename Arg, typename std::enable_if<!std::is_same<
                            typename std::remove_cv<typename std::remove_reference<Arg>::type>::type, FindKey>::value &&
                            std::is_constructible<FindKey, typename std::remove_cv<typename std::remove_reference<Arg>::type>::type
                            >::value>::type * = nullptr>
    const_iterator find(const Arg & arg) const
    {
        FindKey key(arg);
        return find(std::move(key));
    }

    template <typename InKey, typename std::enable_if<std::is_same<
                              typename std::remove_cv<typename std::remove_reference<InKey>::type>::type, FindKey
                              >::value>::type * = nullptr>
    iterator find(const InKey & key)
    {
        size_t index = hash_policy.index_for_hash(hash_object(key), num_slots_minus_one);
        EntryPointer it = entries + ptrdiff_t(index);
        for (int8_t distance = 0; it->distance_from_desired >= distance; ++distance, ++it)
        {
            if (compares_equal(key, it->value))
                return { it };
        }
        return end();
    }

    template <typename InKey, typename std::enable_if<std::is_same<
                              typename std::remove_cv<typename std::remove_reference<InKey>::type>::type, FindKey
                              >::value>::type * = nullptr>
    const_iterator find(const InKey & key) const
    {
        return const_cast<sherwood_v3_table *>(this)->find(key);
    }
#else
    iterator find(const FindKey & key)
    {
        size_t index = hash_policy.index_for_hash(hash_object(key), num_slots_minus_one);
        EntryPointer it = entries + ptrdiff_t(index);
        for (int8_t distance = 0; it->distance_from_desired >= distance; ++distance, ++it)
        {
            if (compares_equal(key, it->value))
                return { it };
        }
        return end();
    }

    const_iterator find(const FindKey & key) const
    {
        return const_cast<sherwood_v3_table *>(this)->find(key);
    }
#endif

    size_t count(const FindKey & key) const
    {
        return find(key) == end() ? 0 : 1;
    }
    std::pair<iterator, iterator> equal_range(const FindKey & key)
    {
        iterator found = find(key);
        if (found == end())
            return { found, found };
        else
            return { found, std::next(found) };
    }
    std::pair<const_iterator, const_iterator> equal_range(const FindKey & key) const
    {
        const_iterator found = find(key);
        if (found == end())
            return { found, found };
        else
            return { found, std::next(found) };
    }

    template<typename Key, typename... Args>
    std::pair<iterator, bool> emplace(Key && key, Args &&... args)
    {
        size_t index = hash_policy.index_for_hash(hash_object(key), num_slots_minus_one);
        EntryPointer current_entry = entries + ptrdiff_t(index);
        int8_t distance_from_desired = 0;
        for (; current_entry->distance_from_desired >= distance_from_desired; ++current_entry, ++distance_from_desired)
        {
            if (compares_equal(key, current_entry->value))
                return { { current_entry }, false };
        }
        return emplace_new_key(distance_from_desired, current_entry, std::forward<Key>(key), std::forward<Args>(args)...);
    }

    std::pair<iterator, bool> insert(const value_type & value)
    {
        return emplace(value);
    }
    std::pair<iterator, bool> insert(value_type && value)
    {
        return emplace(std::move(value));
    }
    template<typename... Args>
    iterator emplace_hint(const_iterator, Args &&... args)
    {
        return emplace(std::forward<Args>(args)...).first;
    }
    iterator insert(const_iterator, const value_type & value)
    {
        return emplace(value).first;
    }
    iterator insert(const_iterator, value_type && value)
    {
        return emplace(std::move(value)).first;
    }

    template<typename It>
    void insert(It begin, It end)
    {
        for (; begin != end; ++begin)
        {
            emplace(*begin);
        }
    }
    void insert(std::initializer_list<value_type> il)
    {
        insert(il.begin(), il.end());
    }

    void rehash(size_t num_buckets)
    {
        num_buckets = std::max(num_buckets, static_cast<size_t>(std::ceil(num_elements / static_cast<double>(_max_load_factor))));
        if (num_buckets == 0)
        {
            reset_to_empty_state();
            return;
        }
        auto new_prime_index = hash_policy.next_size_over(num_buckets);
        if (num_buckets == bucket_count())
            return;
        int8_t new_max_lookups = compute_max_lookups(num_buckets);
        EntryPointer new_buckets(AllocatorTraits::allocate(*this, num_buckets + new_max_lookups));
        EntryPointer special_end_item = new_buckets + static_cast<ptrdiff_t>(num_buckets + new_max_lookups - 1);
        for (EntryPointer it = new_buckets; it != special_end_item; ++it)
            it->distance_from_desired = -1;
        special_end_item->distance_from_desired = Entry::special_end_value;
        std::swap(entries, new_buckets);
        std::swap(num_slots_minus_one, num_buckets);
        --num_slots_minus_one;
        hash_policy.commit(new_prime_index);
        int8_t old_max_lookups = max_lookups;
        max_lookups = new_max_lookups;
        num_elements = 0;
        for (EntryPointer it = new_buckets, end = it + static_cast<ptrdiff_t>(num_buckets + old_max_lookups); it != end; ++it)
        {
            if (it->has_value())
            {
                emplace(std::move(it->value));
                it->destroy_value();
            }
        }
        deallocate_data(new_buckets, num_buckets, old_max_lookups);
    }

    void reserve(size_t num_elements)
    {
        size_t required_buckets = num_buckets_for_reserve(num_elements);
        if (required_buckets > bucket_count())
            rehash(required_buckets);
    }

    // the return value is a type that can be converted to an iterator
    // the reason for doing this is that it's not free to find the
    // iterator pointing at the next element. if you care about the
    // next iterator, turn the return value into an iterator
    convertible_to_iterator erase(const_iterator to_erase)
    {
        EntryPointer current = to_erase.current;
        current->destroy_value();
        --num_elements;
        for (EntryPointer next = current + ptrdiff_t(1); !next->is_at_desired_position(); ++current, ++next)
        {
            current->emplace(next->distance_from_desired - 1, std::move(next->value));
            next->destroy_value();
        }
        return { to_erase.current };
    }

    iterator erase(const_iterator begin_it, const_iterator end_it)
    {
        if (begin_it == end_it)
            return { begin_it.current };
        for (EntryPointer it = begin_it.current, end = end_it.current; it != end; ++it)
        {
            if (it->has_value())
            {
                it->destroy_value();
                --num_elements;
            }
        }
        if (end_it == this->end())
            return this->end();
        ptrdiff_t num_to_move = std::min(static_cast<ptrdiff_t>(end_it.current->distance_from_desired), end_it.current - begin_it.current);
        EntryPointer to_return = end_it.current - num_to_move;
        for (EntryPointer it = end_it.current; !it->is_at_desired_position();)
        {
            EntryPointer target = it - num_to_move;
            target->emplace(it->distance_from_desired - num_to_move, std::move(it->value));
            it->destroy_value();
            ++it;
            num_to_move = std::min(static_cast<ptrdiff_t>(it->distance_from_desired), num_to_move);
        }
        return { to_return };
    }

    size_t erase(const FindKey & key)
    {
        auto found = find(key);
        if (found == end())
            return 0;
        else
        {
            erase(found);
            return 1;
        }
    }

    void clear()
    {
        for (EntryPointer it = entries, end = it + static_cast<ptrdiff_t>(num_slots_minus_one + max_lookups); it != end; ++it)
        {
            if (it->has_value())
                it->destroy_value();
        }
        num_elements = 0;
    }

    void shrink_to_fit()
    {
        rehash_for_other_container(*this);
    }

    void swap(sherwood_v3_table & other)
    {
        using std::swap;
        swap_pointers(other);
        swap(static_cast<ArgumentHash &>(*this), static_cast<ArgumentHash &>(other));
        swap(static_cast<ArgumentEqual &>(*this), static_cast<ArgumentEqual &>(other));
        if (AllocatorTraits::propagate_on_container_swap::value)
            swap(static_cast<EntryAlloc &>(*this), static_cast<EntryAlloc &>(other));
    }

    size_t size() const
    {
        return num_elements;
    }
    size_t max_size() const
    {
        return (AllocatorTraits::max_size(*this)) / sizeof(Entry);
    }
    size_t bucket_count() const
    {
        return num_slots_minus_one ? num_slots_minus_one + 1 : 0;
    }
    size_type max_bucket_count() const
    {
        return (AllocatorTraits::max_size(*this) - min_lookups) / sizeof(Entry);
    }
    size_t bucket(const FindKey & key) const
    {
        return hash_policy.index_for_hash(hash_object(key), num_slots_minus_one);
    }
    float load_factor() const
    {
        size_t buckets = bucket_count();
        if (buckets)
            return static_cast<float>(num_elements) / bucket_count();
        else
            return 0;
    }
    void max_load_factor(float value)
    {
        _max_load_factor = value;
    }
    float max_load_factor() const
    {
        return _max_load_factor;
    }

    bool empty() const
    {
        return num_elements == 0;
    }

private:
    EntryPointer entries = Entry::empty_default_table();
    size_t num_slots_minus_one = 0;
    typename HashPolicySelector<ArgumentHash>::type hash_policy;
    int8_t max_lookups = detailv3::min_lookups - 1;
    float _max_load_factor = 0.5f;
    size_t num_elements = 0;

    static int8_t compute_max_lookups(size_t num_buckets)
    {
        int8_t desired = detailv3::log2(num_buckets);
        return std::max(detailv3::min_lookups, desired);
    }

    size_t num_buckets_for_reserve(size_t num_elements) const
    {
        return static_cast<size_t>(std::ceil(num_elements / std::min(0.5, static_cast<double>(_max_load_factor))));
    }
    void rehash_for_other_container(const sherwood_v3_table & other)
    {
        rehash(std::min(num_buckets_for_reserve(other.size()), other.bucket_count()));
    }

    void swap_pointers(sherwood_v3_table & other)
    {
        using std::swap;
        swap(hash_policy, other.hash_policy);
        swap(entries, other.entries);
        swap(num_slots_minus_one, other.num_slots_minus_one);
        swap(num_elements, other.num_elements);
        swap(max_lookups, other.max_lookups);
        swap(_max_load_factor, other._max_load_factor);
    }

    template<typename Key, typename... Args>
    SKA_NOINLINE(std::pair<iterator, bool>) emplace_new_key(int8_t distance_from_desired, EntryPointer current_entry, Key && key, Args &&... args)
    {
        using std::swap;
        if (num_slots_minus_one == 0 || distance_from_desired == max_lookups || num_elements + 1 > (num_slots_minus_one + 1) * static_cast<double>(_max_load_factor))
        {
            grow();
            return emplace(std::forward<Key>(key), std::forward<Args>(args)...);
        }
        else if (current_entry->is_empty())
        {
            current_entry->emplace(distance_from_desired, std::forward<Key>(key), std::forward<Args>(args)...);
            ++num_elements;
            return { { current_entry }, true };
        }
        value_type to_insert(std::forward<Key>(key), std::forward<Args>(args)...);
        swap(distance_from_desired, current_entry->distance_from_desired);
        swap(to_insert, current_entry->value);
        iterator result = { current_entry };
        for (++distance_from_desired, ++current_entry;; ++current_entry)
        {
            if (current_entry->is_empty())
            {
                current_entry->emplace(distance_from_desired, std::move(to_insert));
                ++num_elements;
                return { result, true };
            }
            else if (current_entry->distance_from_desired < distance_from_desired)
            {
                swap(distance_from_desired, current_entry->distance_from_desired);
                swap(to_insert, current_entry->value);
                ++distance_from_desired;
            }
            else
            {
                ++distance_from_desired;
                if (distance_from_desired == max_lookups)
                {
                    swap(to_insert, result.current->value);
                    grow();
                    return emplace(std::move(to_insert));
                }
            }
        }
    }

    void grow()
    {
        rehash(std::max(size_t(4), 2 * bucket_count()));
    }

    void deallocate_data(EntryPointer begin, size_t num_slots_minus_one, int8_t max_lookups)
    {
        if (begin != Entry::empty_default_table())
        {
            AllocatorTraits::deallocate(*this, begin, num_slots_minus_one + max_lookups + 1);
        }
    }

    void reset_to_empty_state()
    {
        deallocate_data(entries, num_slots_minus_one, max_lookups);
        entries = Entry::empty_default_table();
        num_slots_minus_one = 0;
        hash_policy.reset();
        max_lookups = detailv3::min_lookups - 1;
    }

    template <typename U>
    size_t hash_object(const U & key)
    {
        return static_cast<Hasher &>(*this)(key);
        //return hasher()(key);
    }
    template <typename U>
    size_t hash_object(const U & key) const
    {
        return static_cast<const Hasher &>(*this)(key);
        //return hasher()(key);
    }

    template<typename L, typename R>
    bool compares_equal(const L & lhs, const R & rhs)
    {
        return static_cast<Equal &>(*this)(lhs, rhs);
    }

    struct convertible_to_iterator
    {
        EntryPointer it;

        operator iterator()
        {
            if (it->has_value())
                return { it };
            else
                return ++iterator{it};
        }
        operator const_iterator()
        {
            if (it->has_value())
                return { it };
            else
                return ++const_iterator{it};
        }
    };

};
}

struct prime_number_hash_policy
{
    static size_t mod0(size_t) { return 0llu; }
    static size_t mod2(size_t hash) { return hash % 2llu; }
    static size_t mod3(size_t hash) { return hash % 3llu; }
    static size_t mod5(size_t hash) { return hash % 5llu; }
    static size_t mod7(size_t hash) { return hash % 7llu; }
    static size_t mod11(size_t hash) { return hash % 11llu; }
    static size_t mod13(size_t hash) { return hash % 13llu; }
    static size_t mod17(size_t hash) { return hash % 17llu; }
    static size_t mod23(size_t hash) { return hash % 23llu; }
    static size_t mod29(size_t hash) { return hash % 29llu; }
    static size_t mod37(size_t hash) { return hash % 37llu; }
    static size_t mod47(size_t hash) { return hash % 47llu; }
    static size_t mod59(size_t hash) { return hash % 59llu; }
    static size_t mod73(size_t hash) { return hash % 73llu; }
    static size_t mod97(size_t hash) { return hash % 97llu; }
    static size_t mod127(size_t hash) { return hash % 127llu; }
    static size_t mod151(size_t hash) { return hash % 151llu; }
    static size_t mod197(size_t hash) { return hash % 197llu; }
    static size_t mod251(size_t hash) { return hash % 251llu; }
    static size_t mod313(size_t hash) { return hash % 313llu; }
    static size_t mod397(size_t hash) { return hash % 397llu; }
    static size_t mod499(size_t hash) { return hash % 499llu; }
    static size_t mod631(size_t hash) { return hash % 631llu; }
    static size_t mod797(size_t hash) { return hash % 797llu; }
    static size_t mod1009(size_t hash) { return hash % 1009llu; }
    static size_t mod1259(size_t hash) { return hash % 1259llu; }
    static size_t mod1597(size_t hash) { return hash % 1597llu; }
    static size_t mod2011(size_t hash) { return hash % 2011llu; }
    static size_t mod2539(size_t hash) { return hash % 2539llu; }
    static size_t mod3203(size_t hash) { return hash % 3203llu; }
    static size_t mod4027(size_t hash) { return hash % 4027llu; }
    static size_t mod5087(size_t hash) { return hash % 5087llu; }
    static size_t mod6421(size_t hash) { return hash % 6421llu; }
    static size_t mod8089(size_t hash) { return hash % 8089llu; }
    static size_t mod10193(size_t hash) { return hash % 10193llu; }
    static size_t mod12853(size_t hash) { return hash % 12853llu; }
    static size_t mod16193(size_t hash) { return hash % 16193llu; }
    static size_t mod20399(size_t hash) { return hash % 20399llu; }
    static size_t mod25717(size_t hash) { return hash % 25717llu; }
    static size_t mod32401(size_t hash) { return hash % 32401llu; }
    static size_t mod40823(size_t hash) { return hash % 40823llu; }
    static size_t mod51437(size_t hash) { return hash % 51437llu; }
    static size_t mod64811(size_t hash) { return hash % 64811llu; }
    static size_t mod81649(size_t hash) { return hash % 81649llu; }
    static size_t mod102877(size_t hash) { return hash % 102877llu; }
    static size_t mod129607(size_t hash) { return hash % 129607llu; }
    static size_t mod163307(size_t hash) { return hash % 163307llu; }
    static size_t mod205759(size_t hash) { return hash % 205759llu; }
    static size_t mod259229(size_t hash) { return hash % 259229llu; }
    static size_t mod326617(size_t hash) { return hash % 326617llu; }
    static size_t mod411527(size_t hash) { return hash % 411527llu; }
    static size_t mod518509(size_t hash) { return hash % 518509llu; }
    static size_t mod653267(size_t hash) { return hash % 653267llu; }
    static size_t mod823117(size_t hash) { return hash % 823117llu; }
    static size_t mod1037059(size_t hash) { return hash % 1037059llu; }
    static size_t mod1306601(size_t hash) { return hash % 1306601llu; }
    static size_t mod1646237(size_t hash) { return hash % 1646237llu; }
    static size_t mod2074129(size_t hash) { return hash % 2074129llu; }
    static size_t mod2613229(size_t hash) { return hash % 2613229llu; }
    static size_t mod3292489(size_t hash) { return hash % 3292489llu; }
    static size_t mod4148279(size_t hash) { return hash % 4148279llu; }
    static size_t mod5226491(size_t hash) { return hash % 5226491llu; }
    static size_t mod6584983(size_t hash) { return hash % 6584983llu; }
    static size_t mod8296553(size_t hash) { return hash % 8296553llu; }
    static size_t mod10453007(size_t hash) { return hash % 10453007llu; }
    static size_t mod13169977(size_t hash) { return hash % 13169977llu; }
    static size_t mod16593127(size_t hash) { return hash % 16593127llu; }
    static size_t mod20906033(size_t hash) { return hash % 20906033llu; }
    static size_t mod26339969(size_t hash) { return hash % 26339969llu; }
    static size_t mod33186281(size_t hash) { return hash % 33186281llu; }
    static size_t mod41812097(size_t hash) { return hash % 41812097llu; }
    static size_t mod52679969(size_t hash) { return hash % 52679969llu; }
    static size_t mod66372617(size_t hash) { return hash % 66372617llu; }
    static size_t mod83624237(size_t hash) { return hash % 83624237llu; }
    static size_t mod105359939(size_t hash) { return hash % 105359939llu; }
    static size_t mod132745199(size_t hash) { return hash % 132745199llu; }
    static size_t mod167248483(size_t hash) { return hash % 167248483llu; }
    static size_t mod210719881(size_t hash) { return hash % 210719881llu; }
    static size_t mod265490441(size_t hash) { return hash % 265490441llu; }
    static size_t mod334496971(size_t hash) { return hash % 334496971llu; }
    static size_t mod421439783(size_t hash) { return hash % 421439783llu; }
    static size_t mod530980861(size_t hash) { return hash % 530980861llu; }
    static size_t mod668993977(size_t hash) { return hash % 668993977llu; }
    static size_t mod842879579(size_t hash) { return hash % 842879579llu; }
    static size_t mod1061961721(size_t hash) { return hash % 1061961721llu; }
    static size_t mod1337987929(size_t hash) { return hash % 1337987929llu; }
    static size_t mod1685759167(size_t hash) { return hash % 1685759167llu; }
    static size_t mod2123923447(size_t hash) { return hash % 2123923447llu; }
    static size_t mod2675975881(size_t hash) { return hash % 2675975881llu; }
    static size_t mod3371518343(size_t hash) { return hash % 3371518343llu; }
    static size_t mod4247846927(size_t hash) { return hash % 4247846927llu; }
    static size_t mod5351951779(size_t hash) { return hash % 5351951779llu; }
    static size_t mod6743036717(size_t hash) { return hash % 6743036717llu; }
    static size_t mod8495693897(size_t hash) { return hash % 8495693897llu; }
    static size_t mod10703903591(size_t hash) { return hash % 10703903591llu; }
    static size_t mod13486073473(size_t hash) { return hash % 13486073473llu; }
    static size_t mod16991387857(size_t hash) { return hash % 16991387857llu; }
    static size_t mod21407807219(size_t hash) { return hash % 21407807219llu; }
    static size_t mod26972146961(size_t hash) { return hash % 26972146961llu; }
    static size_t mod33982775741(size_t hash) { return hash % 33982775741llu; }
    static size_t mod42815614441(size_t hash) { return hash % 42815614441llu; }
    static size_t mod53944293929(size_t hash) { return hash % 53944293929llu; }
    static size_t mod67965551447(size_t hash) { return hash % 67965551447llu; }
    static size_t mod85631228929(size_t hash) { return hash % 85631228929llu; }
    static size_t mod107888587883(size_t hash) { return hash % 107888587883llu; }
    static size_t mod135931102921(size_t hash) { return hash % 135931102921llu; }
    static size_t mod171262457903(size_t hash) { return hash % 171262457903llu; }
    static size_t mod215777175787(size_t hash) { return hash % 215777175787llu; }
    static size_t mod271862205833(size_t hash) { return hash % 271862205833llu; }
    static size_t mod342524915839(size_t hash) { return hash % 342524915839llu; }
    static size_t mod431554351609(size_t hash) { return hash % 431554351609llu; }
    static size_t mod543724411781(size_t hash) { return hash % 543724411781llu; }
    static size_t mod685049831731(size_t hash) { return hash % 685049831731llu; }
    static size_t mod863108703229(size_t hash) { return hash % 863108703229llu; }
    static size_t mod1087448823553(size_t hash) { return hash % 1087448823553llu; }
    static size_t mod1370099663459(size_t hash) { return hash % 1370099663459llu; }
    static size_t mod1726217406467(size_t hash) { return hash % 1726217406467llu; }
    static size_t mod2174897647073(size_t hash) { return hash % 2174897647073llu; }
    static size_t mod2740199326961(size_t hash) { return hash % 2740199326961llu; }
    static size_t mod3452434812973(size_t hash) { return hash % 3452434812973llu; }
    static size_t mod4349795294267(size_t hash) { return hash % 4349795294267llu; }
    static size_t mod5480398654009(size_t hash) { return hash % 5480398654009llu; }
    static size_t mod6904869625999(size_t hash) { return hash % 6904869625999llu; }
    static size_t mod8699590588571(size_t hash) { return hash % 8699590588571llu; }
    static size_t mod10960797308051(size_t hash) { return hash % 10960797308051llu; }
    static size_t mod13809739252051(size_t hash) { return hash % 13809739252051llu; }
    static size_t mod17399181177241(size_t hash) { return hash % 17399181177241llu; }
    static size_t mod21921594616111(size_t hash) { return hash % 21921594616111llu; }
    static size_t mod27619478504183(size_t hash) { return hash % 27619478504183llu; }
    static size_t mod34798362354533(size_t hash) { return hash % 34798362354533llu; }
    static size_t mod43843189232363(size_t hash) { return hash % 43843189232363llu; }
    static size_t mod55238957008387(size_t hash) { return hash % 55238957008387llu; }
    static size_t mod69596724709081(size_t hash) { return hash % 69596724709081llu; }
    static size_t mod87686378464759(size_t hash) { return hash % 87686378464759llu; }
    static size_t mod110477914016779(size_t hash) { return hash % 110477914016779llu; }
    static size_t mod139193449418173(size_t hash) { return hash % 139193449418173llu; }
    static size_t mod175372756929481(size_t hash) { return hash % 175372756929481llu; }
    static size_t mod220955828033581(size_t hash) { return hash % 220955828033581llu; }
    static size_t mod278386898836457(size_t hash) { return hash % 278386898836457llu; }
    static size_t mod350745513859007(size_t hash) { return hash % 350745513859007llu; }
    static size_t mod441911656067171(size_t hash) { return hash % 441911656067171llu; }
    static size_t mod556773797672909(size_t hash) { return hash % 556773797672909llu; }
    static size_t mod701491027718027(size_t hash) { return hash % 701491027718027llu; }
    static size_t mod883823312134381(size_t hash) { return hash % 883823312134381llu; }
    static size_t mod1113547595345903(size_t hash) { return hash % 1113547595345903llu; }
    static size_t mod1402982055436147(size_t hash) { return hash % 1402982055436147llu; }
    static size_t mod1767646624268779(size_t hash) { return hash % 1767646624268779llu; }
    static size_t mod2227095190691797(size_t hash) { return hash % 2227095190691797llu; }
    static size_t mod2805964110872297(size_t hash) { return hash % 2805964110872297llu; }
    static size_t mod3535293248537579(size_t hash) { return hash % 3535293248537579llu; }
    static size_t mod4454190381383713(size_t hash) { return hash % 4454190381383713llu; }
    static size_t mod5611928221744609(size_t hash) { return hash % 5611928221744609llu; }
    static size_t mod7070586497075177(size_t hash) { return hash % 7070586497075177llu; }
    static size_t mod8908380762767489(size_t hash) { return hash % 8908380762767489llu; }
    static size_t mod11223856443489329(size_t hash) { return hash % 11223856443489329llu; }
    static size_t mod14141172994150357(size_t hash) { return hash % 14141172994150357llu; }
    static size_t mod17816761525534927(size_t hash) { return hash % 17816761525534927llu; }
    static size_t mod22447712886978529(size_t hash) { return hash % 22447712886978529llu; }
    static size_t mod28282345988300791(size_t hash) { return hash % 28282345988300791llu; }
    static size_t mod35633523051069991(size_t hash) { return hash % 35633523051069991llu; }
    static size_t mod44895425773957261(size_t hash) { return hash % 44895425773957261llu; }
    static size_t mod56564691976601587(size_t hash) { return hash % 56564691976601587llu; }
    static size_t mod71267046102139967(size_t hash) { return hash % 71267046102139967llu; }
    static size_t mod89790851547914507(size_t hash) { return hash % 89790851547914507llu; }
    static size_t mod113129383953203213(size_t hash) { return hash % 113129383953203213llu; }
    static size_t mod142534092204280003(size_t hash) { return hash % 142534092204280003llu; }
    static size_t mod179581703095829107(size_t hash) { return hash % 179581703095829107llu; }
    static size_t mod226258767906406483(size_t hash) { return hash % 226258767906406483llu; }
    static size_t mod285068184408560057(size_t hash) { return hash % 285068184408560057llu; }
    static size_t mod359163406191658253(size_t hash) { return hash % 359163406191658253llu; }
    static size_t mod452517535812813007(size_t hash) { return hash % 452517535812813007llu; }
    static size_t mod570136368817120201(size_t hash) { return hash % 570136368817120201llu; }
    static size_t mod718326812383316683(size_t hash) { return hash % 718326812383316683llu; }
    static size_t mod905035071625626043(size_t hash) { return hash % 905035071625626043llu; }
    static size_t mod1140272737634240411(size_t hash) { return hash % 1140272737634240411llu; }
    static size_t mod1436653624766633509(size_t hash) { return hash % 1436653624766633509llu; }
    static size_t mod1810070143251252131(size_t hash) { return hash % 1810070143251252131llu; }
    static size_t mod2280545475268481167(size_t hash) { return hash % 2280545475268481167llu; }
    static size_t mod2873307249533267101(size_t hash) { return hash % 2873307249533267101llu; }
    static size_t mod3620140286502504283(size_t hash) { return hash % 3620140286502504283llu; }
    static size_t mod4561090950536962147(size_t hash) { return hash % 4561090950536962147llu; }
    static size_t mod5746614499066534157(size_t hash) { return hash % 5746614499066534157llu; }
    static size_t mod7240280573005008577(size_t hash) { return hash % 7240280573005008577llu; }
    static size_t mod9122181901073924329(size_t hash) { return hash % 9122181901073924329llu; }
    static size_t mod11493228998133068689(size_t hash) { return hash % 11493228998133068689llu; }
    static size_t mod14480561146010017169(size_t hash) { return hash % 14480561146010017169llu; }
    static size_t mod18446744073709551557(size_t hash) { return hash % 18446744073709551557llu; }

    using mod_function = size_t (*)(size_t);

    mod_function next_size_over(size_t & size) const
    {
        // prime numbers generated by the following method:
        // 1. start with a prime p = 2
        // 2. go to wolfram alpha and get p = NextPrime(2 * p)
        // 3. repeat 2. until you overflow 64 bits
        // you now have large gaps which you would hit if somebody called reserve() with an unlucky number.
        // 4. to fill the gaps for every prime p go to wolfram alpha and get ClosestPrime(p * 2^(1/3)) and ClosestPrime(p * 2^(2/3)) and put those in the gaps
        // 5. get PrevPrime(2^64) and put it at the end
        static constexpr const size_t prime_list[] =
        {
            2llu, 3llu, 5llu, 7llu, 11llu, 13llu, 17llu, 23llu, 29llu, 37llu, 47llu,
            59llu, 73llu, 97llu, 127llu, 151llu, 197llu, 251llu, 313llu, 397llu,
            499llu, 631llu, 797llu, 1009llu, 1259llu, 1597llu, 2011llu, 2539llu,
            3203llu, 4027llu, 5087llu, 6421llu, 8089llu, 10193llu, 12853llu, 16193llu,
            20399llu, 25717llu, 32401llu, 40823llu, 51437llu, 64811llu, 81649llu,
            102877llu, 129607llu, 163307llu, 205759llu, 259229llu, 326617llu,
            411527llu, 518509llu, 653267llu, 823117llu, 1037059llu, 1306601llu,
            1646237llu, 2074129llu, 2613229llu, 3292489llu, 4148279llu, 5226491llu,
            6584983llu, 8296553llu, 10453007llu, 13169977llu, 16593127llu, 20906033llu,
            26339969llu, 33186281llu, 41812097llu, 52679969llu, 66372617llu,
            83624237llu, 105359939llu, 132745199llu, 167248483llu, 210719881llu,
            265490441llu, 334496971llu, 421439783llu, 530980861llu, 668993977llu,
            842879579llu, 1061961721llu, 1337987929llu, 1685759167llu, 2123923447llu,
            2675975881llu, 3371518343llu, 4247846927llu, 5351951779llu, 6743036717llu,
            8495693897llu, 10703903591llu, 13486073473llu, 16991387857llu,
            21407807219llu, 26972146961llu, 33982775741llu, 42815614441llu,
            53944293929llu, 67965551447llu, 85631228929llu, 107888587883llu,
            135931102921llu, 171262457903llu, 215777175787llu, 271862205833llu,
            342524915839llu, 431554351609llu, 543724411781llu, 685049831731llu,
            863108703229llu, 1087448823553llu, 1370099663459llu, 1726217406467llu,
            2174897647073llu, 2740199326961llu, 3452434812973llu, 4349795294267llu,
            5480398654009llu, 6904869625999llu, 8699590588571llu, 10960797308051llu,
            13809739252051llu, 17399181177241llu, 21921594616111llu, 27619478504183llu,
            34798362354533llu, 43843189232363llu, 55238957008387llu, 69596724709081llu,
            87686378464759llu, 110477914016779llu, 139193449418173llu,
            175372756929481llu, 220955828033581llu, 278386898836457llu,
            350745513859007llu, 441911656067171llu, 556773797672909llu,
            701491027718027llu, 883823312134381llu, 1113547595345903llu,
            1402982055436147llu, 1767646624268779llu, 2227095190691797llu,
            2805964110872297llu, 3535293248537579llu, 4454190381383713llu,
            5611928221744609llu, 7070586497075177llu, 8908380762767489llu,
            11223856443489329llu, 14141172994150357llu, 17816761525534927llu,
            22447712886978529llu, 28282345988300791llu, 35633523051069991llu,
            44895425773957261llu, 56564691976601587llu, 71267046102139967llu,
            89790851547914507llu, 113129383953203213llu, 142534092204280003llu,
            179581703095829107llu, 226258767906406483llu, 285068184408560057llu,
            359163406191658253llu, 452517535812813007llu, 570136368817120201llu,
            718326812383316683llu, 905035071625626043llu, 1140272737634240411llu,
            1436653624766633509llu, 1810070143251252131llu, 2280545475268481167llu,
            2873307249533267101llu, 3620140286502504283llu, 4561090950536962147llu,
            5746614499066534157llu, 7240280573005008577llu, 9122181901073924329llu,
            11493228998133068689llu, 14480561146010017169llu, 18446744073709551557llu
        };
        static constexpr size_t (* const mod_functions[])(size_t) =
        {
            &mod0, &mod2, &mod3, &mod5, &mod7, &mod11, &mod13, &mod17, &mod23, &mod29, &mod37,
            &mod47, &mod59, &mod73, &mod97, &mod127, &mod151, &mod197, &mod251, &mod313, &mod397,
            &mod499, &mod631, &mod797, &mod1009, &mod1259, &mod1597, &mod2011, &mod2539, &mod3203,
            &mod4027, &mod5087, &mod6421, &mod8089, &mod10193, &mod12853, &mod16193, &mod20399,
            &mod25717, &mod32401, &mod40823, &mod51437, &mod64811, &mod81649, &mod102877,
            &mod129607, &mod163307, &mod205759, &mod259229, &mod326617, &mod411527, &mod518509,
            &mod653267, &mod823117, &mod1037059, &mod1306601, &mod1646237, &mod2074129,
            &mod2613229, &mod3292489, &mod4148279, &mod5226491, &mod6584983, &mod8296553,
            &mod10453007, &mod13169977, &mod16593127, &mod20906033, &mod26339969, &mod33186281,
            &mod41812097, &mod52679969, &mod66372617, &mod83624237, &mod105359939, &mod132745199,
            &mod167248483, &mod210719881, &mod265490441, &mod334496971, &mod421439783,
            &mod530980861, &mod668993977, &mod842879579, &mod1061961721, &mod1337987929,
            &mod1685759167, &mod2123923447, &mod2675975881, &mod3371518343, &mod4247846927,
            &mod5351951779, &mod6743036717, &mod8495693897, &mod10703903591, &mod13486073473,
            &mod16991387857, &mod21407807219, &mod26972146961, &mod33982775741, &mod42815614441,
            &mod53944293929, &mod67965551447, &mod85631228929, &mod107888587883, &mod135931102921,
            &mod171262457903, &mod215777175787, &mod271862205833, &mod342524915839,
            &mod431554351609, &mod543724411781, &mod685049831731, &mod863108703229,
            &mod1087448823553, &mod1370099663459, &mod1726217406467, &mod2174897647073,
            &mod2740199326961, &mod3452434812973, &mod4349795294267, &mod5480398654009,
            &mod6904869625999, &mod8699590588571, &mod10960797308051, &mod13809739252051,
            &mod17399181177241, &mod21921594616111, &mod27619478504183, &mod34798362354533,
            &mod43843189232363, &mod55238957008387, &mod69596724709081, &mod87686378464759,
            &mod110477914016779, &mod139193449418173, &mod175372756929481, &mod220955828033581,
            &mod278386898836457, &mod350745513859007, &mod441911656067171, &mod556773797672909,
            &mod701491027718027, &mod883823312134381, &mod1113547595345903, &mod1402982055436147,
            &mod1767646624268779, &mod2227095190691797, &mod2805964110872297, &mod3535293248537579,
            &mod4454190381383713, &mod5611928221744609, &mod7070586497075177, &mod8908380762767489,
            &mod11223856443489329, &mod14141172994150357, &mod17816761525534927,
            &mod22447712886978529, &mod28282345988300791, &mod35633523051069991,
            &mod44895425773957261, &mod56564691976601587, &mod71267046102139967,
            &mod89790851547914507, &mod113129383953203213, &mod142534092204280003,
            &mod179581703095829107, &mod226258767906406483, &mod285068184408560057,
            &mod359163406191658253, &mod452517535812813007, &mod570136368817120201,
            &mod718326812383316683, &mod905035071625626043, &mod1140272737634240411,
            &mod1436653624766633509, &mod1810070143251252131, &mod2280545475268481167,
            &mod2873307249533267101, &mod3620140286502504283, &mod4561090950536962147,
            &mod5746614499066534157, &mod7240280573005008577, &mod9122181901073924329,
            &mod11493228998133068689, &mod14480561146010017169, &mod18446744073709551557
        };
        const size_t * found = std::lower_bound(std::begin(prime_list), std::end(prime_list) - 1, size);
        size = *found;
        return mod_functions[1 + found - prime_list];
    }
    void commit(mod_function new_mod_function)
    {
        current_mod_function = new_mod_function;
    }
    void reset()
    {
        current_mod_function = &mod0;
    }

    size_t index_for_hash(size_t hash, size_t /*num_slots_minus_one*/) const
    {
        return current_mod_function(hash);
    }
    size_t keep_in_range(size_t index, size_t num_slots_minus_one) const
    {
        return index > num_slots_minus_one ? current_mod_function(index) : index;
    }

private:
    mod_function current_mod_function = &mod0;
};

struct power_of_two_hash_policy
{
    size_t index_for_hash(size_t hash, size_t num_slots_minus_one) const
    {
        return hash & num_slots_minus_one;
    }
    size_t keep_in_range(size_t index, size_t num_slots_minus_one) const
    {
        return index_for_hash(index, num_slots_minus_one);
    }
    int8_t next_size_over(size_t & size) const
    {
        size = detailv3::next_power_of_two(size);
        return 0;
    }
    void commit(int8_t)
    {
    }
    void reset()
    {
    }

};

struct fibonacci_hash_policy
{
    size_t index_for_hash(size_t hash, size_t /*num_slots_minus_one*/) const
    {
        return (11400714819323198485ull * hash) >> shift;
    }
    size_t keep_in_range(size_t index, size_t num_slots_minus_one) const
    {
        return index & num_slots_minus_one;
    }

    int8_t next_size_over(size_t & size) const
    {
        size = std::max(size_t(2), detailv3::next_power_of_two(size));
        return 64 - detailv3::log2(size);
    }
    void commit(int8_t shift)
    {
        this->shift = shift;
    }
    void reset()
    {
        shift = 63;
    }

private:
    int8_t shift = 63;
};

template<typename K, typename V, typename H = std::hash<K>, typename E = std::equal_to<K>, typename A = std::allocator<std::pair<K, V> > >
class flat_hash_map
        : public detailv3::sherwood_v3_table
        <
            std::pair<K, V>,
            K,
            H,
            detailv3::KeyOrValueHasher<K, std::pair<K, V>, H>,
            E,
            detailv3::KeyOrValueEquality<K, std::pair<K, V>, E>,
            A,
            typename std::allocator_traits<A>::template rebind_alloc<detailv3::sherwood_v3_entry<std::pair<K, V>>>
        >
{
    using Table = detailv3::sherwood_v3_table
    <
        std::pair<K, V>,
        K,
        H,
        detailv3::KeyOrValueHasher<K, std::pair<K, V>, H>,
        E,
        detailv3::KeyOrValueEquality<K, std::pair<K, V>, E>,
        A,
        typename std::allocator_traits<A>::template rebind_alloc<detailv3::sherwood_v3_entry<std::pair<K, V>>>
    >;
public:

    using key_type = K;
    using mapped_type = V;

    using Table::Table;
    flat_hash_map()
    {
    }

    static const char * name() {
        return "ska::flat_hash_map<K, V>";
    }

    inline V & operator[](const K & key)
    {
        return emplace(key, convertible_to_value()).first->second;
    }
    inline V & operator[](K && key)
    {
        return emplace(std::move(key), convertible_to_value()).first->second;
    }
    V & at(const K & key)
    {
        auto found = this->find(key);
//        if (found == this->end())
//            throw std::out_of_range("Argument passed to at() was not in the map.");
        return found->second;
    }
    const V & at(const K & key) const
    {
        auto found = this->find(key);
        //if (found == this->end())
        //    throw std::out_of_range("Argument passed to at() was not in the map.");
        return found->second;
    }

    using Table::emplace;
    std::pair<typename Table::iterator, bool> emplace()
    {
        return emplace(key_type(), convertible_to_value());
    }
    template<typename M>
    std::pair<typename Table::iterator, bool> insert_or_assign(const key_type & key, M && m)
    {
        auto emplace_result = emplace(key, std::forward<M>(m));
        if (!emplace_result.second)
            emplace_result.first->second = std::forward<M>(m);
        return emplace_result;
    }
    template<typename M>
    std::pair<typename Table::iterator, bool> insert_or_assign(key_type && key, M && m)
    {
        auto emplace_result = emplace(std::move(key), std::forward<M>(m));
        if (!emplace_result.second)
            emplace_result.first->second = std::forward<M>(m);
        return emplace_result;
    }
    template<typename M>
    typename Table::iterator insert_or_assign(typename Table::const_iterator, const key_type & key, M && m)
    {
        return insert_or_assign(key, std::forward<M>(m)).first;
    }
    template<typename M>
    typename Table::iterator insert_or_assign(typename Table::const_iterator, key_type && key, M && m)
    {
        return insert_or_assign(std::move(key), std::forward<M>(m)).first;
    }

    friend bool operator==(const flat_hash_map & lhs, const flat_hash_map & rhs)
    {
        if (lhs.size() != rhs.size())
            return false;
        for (const typename Table::value_type & value : lhs)
        {
            auto found = rhs.find(value.first);
            if (found == rhs.end())
                return false;
            else if (value.second != found->second)
                return false;
        }
        return true;
    }
    friend bool operator!=(const flat_hash_map & lhs, const flat_hash_map & rhs)
    {
        return !(lhs == rhs);
    }

private:
    struct convertible_to_value
    {
        operator V() const
        {
            return V();
        }
    };
};

template<typename T, typename H = std::hash<T>, typename E = std::equal_to<T>, typename A = std::allocator<T> >
class flat_hash_set
        : public detailv3::sherwood_v3_table
        <
            T,
            T,
            H,
            detailv3::functor_storage<size_t, H>,
            E,
            detailv3::functor_storage<bool, E>,
            A,
            typename std::allocator_traits<A>::template rebind_alloc<detailv3::sherwood_v3_entry<T>>
        >
{
    using Table = detailv3::sherwood_v3_table
    <
        T,
        T,
        H,
        detailv3::functor_storage<size_t, H>,
        E,
        detailv3::functor_storage<bool, E>,
        A,
        typename std::allocator_traits<A>::template rebind_alloc<detailv3::sherwood_v3_entry<T>>
    >;
public:

    using key_type = T;

    using Table::Table;
    flat_hash_set()
    {
    }

    static const char * name() {
        return "ska::flat_hash_set<K, V>";
    }

    template<typename... Args>
    std::pair<typename Table::iterator, bool> emplace(Args &&... args)
    {
        return Table::emplace(T(std::forward<Args>(args)...));
    }
    std::pair<typename Table::iterator, bool> emplace(const key_type & arg)
    {
        return Table::emplace(arg);
    }
    std::pair<typename Table::iterator, bool> emplace(key_type & arg)
    {
        return Table::emplace(arg);
    }
    std::pair<typename Table::iterator, bool> emplace(const key_type && arg)
    {
        return Table::emplace(std::move(arg));
    }
    std::pair<typename Table::iterator, bool> emplace(key_type && arg)
    {
        return Table::emplace(std::move(arg));
    }

    friend bool operator==(const flat_hash_set & lhs, const flat_hash_set & rhs)
    {
        if (lhs.size() != rhs.size())
            return false;
        for (const T & value : lhs)
        {
            if (rhs.find(value) == rhs.end())
                return false;
        }
        return true;
    }
    friend bool operator!=(const flat_hash_set & lhs, const flat_hash_set & rhs)
    {
        return !(lhs == rhs);
    }
};


template<typename T>
struct power_of_two_std_hash : std::hash<T>
{
    typedef ska::power_of_two_hash_policy hash_policy;
};

} // end namespace ska
