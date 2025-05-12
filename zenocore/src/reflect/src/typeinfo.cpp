#include <zeno/reflect/typeinfo.hpp>
#include <zeno/reflect/container/string>
#include <zeno/reflect/typeinfo.hpp>


zeno::reflect::RTTITypeInfo::RTTITypeInfo(const char* in_name, std::size_t hashcode, size_t flags, size_t decayed_hash)
    : m_name(in_name)
    , m_hashcode(hashcode)
    , m_flags(flags)
    , m_decayed_hash(decayed_hash)
{}

zeno::reflect::RTTITypeInfo::RTTITypeInfo(const RTTITypeInfo & other) {
    m_name = other.m_name;
    m_hashcode = other.m_hashcode;
    m_decayed_hash = other.m_decayed_hash;
    m_flags = other.m_flags;
}

zeno::reflect::RTTITypeInfo::RTTITypeInfo(RTTITypeInfo && other)
    : m_name(other.m_name)
    , m_hashcode(other.m_hashcode)
    , m_decayed_hash(other.m_decayed_hash)
    , m_flags(other.m_flags)
{
    // As the name is stored as constant var, we will not release it
    other.m_name = nullptr;
    other.m_hashcode = 0;
    other.m_decayed_hash = 0;
}

zeno::reflect::RTTITypeInfo &zeno::reflect::RTTITypeInfo::operator=(const zeno::reflect::RTTITypeInfo &other)
{
    m_name = other.m_name;
    m_hashcode = other.m_hashcode;
    m_decayed_hash = other.m_decayed_hash;
    m_flags = other.m_flags;
    return *this;
}

zeno::reflect::RTTITypeInfo &zeno::reflect::RTTITypeInfo::operator=(zeno::reflect::RTTITypeInfo  &&other)
{
    if (&other != this) {
        m_name = other.m_name;
        m_hashcode = other.m_hashcode;
        m_decayed_hash = other.m_decayed_hash;
        m_flags = other.m_flags;
        other.m_name = nullptr;
        other.m_hashcode = 0;
        other.m_decayed_hash = 0;
        other.m_flags = TF_None;
    }
    return *this;
}

const char *zeno::reflect::RTTITypeInfo::name() const
{
    return m_name;
}

size_t zeno::reflect::RTTITypeInfo::hash_code() const
{
    return m_hashcode;
}

size_t zeno::reflect::RTTITypeInfo::flags() const
{
    return m_flags;
}

bool zeno::reflect::RTTITypeInfo::has_flags(size_t in_flags) const
{
    return m_flags & in_flags;
}

const size_t zeno::reflect::RTTITypeInfo::get_decayed_hash() const
{
    return m_decayed_hash;
}

REFLECT_STATIC_CONSTEXPR bool zeno::reflect::RTTITypeInfo::equal_fast(const RTTITypeInfo &other) const
{
    return hash_code() == other.hash_code();
}

REFLECT_STATIC_CONSTEXPR bool zeno::reflect::RTTITypeInfo::equal_fast_unsafe(const RTTITypeInfo &other) const
{
    return this == &other;
}

bool zeno::reflect::RTTITypeInfo::operator==(const RTTITypeInfo &other) const
{
    return hash_code() == other.hash_code() && CStringUtil<char>::strcmp(other.name(), name()) == 0;
}

bool zeno::reflect::RTTITypeInfo::operator!=(const RTTITypeInfo &other) const
{
    return !operator==(other);
}
