#pragma  once

// Like std::string, except with no allocator template parameter, and no inheritance 
// from basic_string.
//
// Advantages:
// * Simpler compile errors.
// * Better intellisense autocompletions.
// * Smaller executable.
// * Simpler code and data structure means it is easier to debug.


#if 0

#include <string>
typedef std::string String;

#else 

#define noinl __declspec(noinline)


class String
{
private:
    void assign(char const *s);
    noinl void assign(char const *s, unsigned len);
    noinl void join(char const *lhs, unsigned lenLhs, char const *rhs, unsigned lenRhs);

public:
    char *m_array;
    unsigned m_len;

    static const unsigned npos;

    // Constructors and destructor
    String();
    String(char const *string);
    noinl String(const String &string);
    ~String();

    // Capacity
    unsigned length() const { return m_len; }
    unsigned size() const { return m_len; }
    noinl void resize(size_t n, char c = '\0');
    noinl void clear();

    // Element access
    char &operator[] (size_t pos) { return m_array[pos]; }
    const char &operator[] (size_t pos) const { return m_array[pos]; }

    // Modifiers
    noinl void operator = (String const &s);
    noinl void operator = (char const *s);
    noinl void operator += (String const &s);
    noinl void operator += (char const *s);
    noinl void operator += (char c);
    noinl void append(const char* s, size_t n);
    noinl void append(const String &s, size_t subpos, size_t sublen);
    noinl void push_back(char c);
    noinl void insert(size_t pos, const char *s);
    noinl void insert(size_t pos, size_t n, char c);

    noinl void erase(size_t pos, size_t len = npos);

    noinl void replace(size_t pos, size_t len, char const *s);

    // Operations
    const char *c_str() const { return m_array; }

    noinl size_t find(char const *str, size_t pos = 0) const;
    noinl size_t find(char c, size_t pos = 0) const;

    noinl size_t rfind(char c, size_t pos = npos) const;
    
    noinl size_t find_first_of(const char *s, size_t pos = 0) const;

    String substr(size_t pos = 0, size_t len = npos) const;
};


String operator + (const String &lhs, const String &rhs);
String operator + (const String &lhs, const char   *rhs);
String operator + (const char   *lhs, const String &rhs);
String operator + (const String &lhs, char          rhs);
String operator + (char          lhs, const String &rhs);

bool operator== (const String &lhs, const String &rhs);
bool operator== (const char   *lhs, const String &rhs);
bool operator== (const String &lhs, const char   *rhs);

bool operator!= (const String &lhs, const String &rhs);
bool operator!= (const char   *lhs, const String &rhs);
bool operator!= (const String &lhs, const char   *rhs);

bool operator<  (const String &lhs, const String &rhs);
bool operator<  (const char   *lhs, const String &rhs);
bool operator<  (const String &lhs, const char   *rhs);

bool operator<= (const String &lhs, const String &rhs);
bool operator<= (const char   *lhs, const String &rhs);
bool operator<= (const String &lhs, const char   *rhs);

bool operator>  (const String &lhs, const String &rhs);
bool operator>  (const char   *lhs, const String &rhs);
bool operator>  (const String &lhs, const char   *rhs);

bool operator>= (const String &lhs, const String &rhs);
bool operator>= (const char   *lhs, const String &rhs);
bool operator>= (const String &lhs, const char   *rhs);

void AndyStringTest();

#endif
