#pragma once

#include <ostream>
#include <sstream>
#include <vector>

#include <Windows.h>

/// \brief This class is derives from basic_stringbuf which will output
/// all the written data using the OutputDebugString function
template <typename TChar, typename TTraits = std::char_traits<TChar>>
class OutputDebugStringBuf : public std::basic_stringbuf<TChar, TTraits> {
public:
    using int_type = typename TTraits::int_type;
    using pos_type = typename TTraits::pos_type;
    using off_type = typename TTraits::off_type;

    OutputDebugStringBuf()
        : _buffer(512) {
        this->setg(nullptr, nullptr, nullptr);
        this->setp(_buffer.data(), _buffer.data(), _buffer.data() + _buffer.size());
    }

    ~OutputDebugStringBuf() override = default;

    static_assert(std::is_same_v<TChar, char> || std::is_same_v<TChar, wchar_t>,
        "OutputDebugStringBuf only supports char and wchar_t types");

protected:
    int sync() override try {
        MessageOutputer<TChar, TTraits>()(this->pbase(), this->pptr());
        this->setp(_buffer.data(), _buffer.data(), _buffer.data() + _buffer.size());
        return 0;
    } catch (...) {
        return -1;
    }

    int_type overflow(int_type c = TTraits::eof()) override  {
        const int syncRet = sync();
        if (c != TTraits::eof()) {
            _buffer[0] = static_cast<TChar>(c);
            this->setp(_buffer.data(), _buffer.data() + 1, _buffer.data() + _buffer.size());
        }
        return syncRet == -1 ? TTraits::eof() : 0;
    }

private:
    std::vector<TChar> _buffer;

    template <typename TChar, typename TTraits> struct MessageOutputer;

    template <> struct MessageOutputer<char, std::char_traits<char>> {
        template <typename TIterator> void operator()(TIterator begin, TIterator end) const {
            const std::string s(begin, end);
            OutputDebugStringA(("[F] " + s).c_str());
        }
    };

    template <> struct MessageOutputer<wchar_t, std::char_traits<wchar_t>> {
        template <typename TIterator> void operator()(TIterator begin, TIterator end) const {
            const std::wstring s(begin, end);
            OutputDebugStringW((L"[F] " + s).c_str());
        }
    };
};
