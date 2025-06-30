/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief parse JSON
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 * Standarts:
 * https://www.json.org/json-en.html
 * IETF STD 90, RFC 8259
 * ECMA-404, 2nd edition, December 2017
 *
 * Unicode: https://www.compart.com/en/unicode/U+1F610
 */

#include "jsonParser.h"
#include "comp.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cinttypes>
#include <cmath>

using namespace std;

const size_t lineSize = 512;

enum e_token {
    JSON_OBJ_STA, // { left curly bracket
    JSON_OBJ_END, // } right curly bracket
    JSON_ARR_STA, // [ left square bracket
    JSON_ARR_END, // ] right square bracket
    JSON_VAL_SEP, // , comma
    JSON_NAM_SEP, // : colon
    JSON_STR,     // " quotation mark
    JSON_NUM,     // Number
    JSON_TRUE,    // 'true' keyword
    JSON_FALSE,   // 'false' keyword
    JSON_NULL,    // 'null' keyword
    JSON_EOF,     // End of file or string buffer
    JSON_INV,     // Invalid character
};

class jsonParser
{
  protected:
    const char *cur = nullptr;
    size_t lineNum = 0;
    bool useComments = false;

  public:

  protected:
    void skipBOM() {
        // Byte order mark U+FEFF
        // We support UTF-8!
        uint8_t *a = (uint8_t *)cur;
        if(a[0] == 0xef && a[1] == 0xbb && a[2] == 0xbf)
            cur += 3;
    }
    virtual bool isEOF() { return *cur == 0; }

  private:
    bool skip_ws() {
        for(;; cur++) {
            if(isEOF())
                return true;
            switch(*cur) {
                case '\n':
                    lineNum++;
                    break; // 0x0a line feed
                case '\r':
                    break; // 0x0d carriage return
                case '\t':
                    break; // 0x09 horizontal tabulation
                case ' ':
                    break; // 0x20 space
                default:
                    return false;
            }
        }
    }
    bool skipComments() {
        bool haveCloseStar = false;
        cur++;
        if(*cur == 0) // end of file or line should not happen after single solidus
            return true;
        switch(*cur) {
            case '/': // Comment until end of line
                for(;;) {
                    cur++;
                    if(*cur == 0 || *cur == '\n') // end of file or line
                        return false; // Comment is end
                }
                break;
            case '*': // Comment until closing
                for(;;) {
                    cur++;
                    switch(*cur) {
                        case 0:
                            if(isEOF())
                                return true; // End of file without closing comment
                            haveCloseStar = false;
                            break;
                        case '*':
                            haveCloseStar = true;
                            break;
                        case '/':
                            if(haveCloseStar) {
                                cur++;
                                return false; // Comment ends
                            }
                            break;
                        case '\n': // end of line
                            lineNum++;
                            FALLTHROUGH;
                        default:
                            haveCloseStar = false;
                            break;
                    }
                }
                break;
            default: // Wrong char
                break;
        }
        return true;
    }
    bool uToVal(uint16_t &val) {
        char hex[5]; // 4 Hex digits + null termination
        for(size_t i = 0; i < 4; i++) {
            if(!isxdigit(*cur) || isEOF())
                return false;
            hex[i] = *cur++;
        }
        hex[4] = 0;
        char *endptr = nullptr;
        val = strtoul(hex, &endptr, 16);
        return *endptr == 0;
    }
    bool uProc(string &ret, bool &useSur, uint16_t &W1) {
        uint32_t val;
        uint16_t val16;
        if(!uToVal(val16))
            return false;
        if(useSur) { // Handle surrogates
            if(val16 >= 0xe000 || val16 < 0xdc00)
                return false; // Invalid second surrogate
            // UTF-16 surrogate second value
            uint16_t W2 = (val16 - 0xdc00) & 0x3ff;
            val = ((W1 << 10) | W2) + 0x10000;
            useSur = false;
        } else if(val16 < 0xd800 || val16 >= 0xe000)
            val = val16;
        else if(val16 < 0xdc00) {
            // Save first surrogate
            useSur = true;
            W1 = (val16 - 0xd800) & 0x3ff;
            return true;
        } else // Invalid first surrogate
            return false;
        uint8_t bytes = 1;
        if(val < 0x80) // One byte
            ret += (char)(val & 0x7f);
        else if(val < 0x800) { // 2 bytes
            ret += (char)(0xc0 | (0x1f & (val >> 6)));
            bytes = 2;
        } else if(val < 0x10000) { // 3 bytes
            ret += (char)(0xe0 | (0xf & (val >> 12)));
            bytes = 3;
        } else { // 4 bytes
            ret += (char)(0xf0 | (0x7 & (val >> 18)));
            bytes = 4;
        }
        for(int i = bytes; i > 1; i--)
            ret += (char)(0x80 | (0x3f & (val >> ((i - 2) * 6))));
        return true;
    }
    bool checkUTF8(string &ret) {
        // Check for proper UTF-8!
        uint8_t val8 = (uint8_t) * cur;
        // Check range reserved for 2nd tp 4th bytes, not first!
        if(val8 >= 0x80 && val8 < 0xc0)
            return false;
        ret += *cur++; // First byte
        if(val8 >= 0xc0) { // Check how many extra bytes 2nd to 4th
            size_t e = 2; // 2nd
            if(val8 >= 0xe0)
                e = val8 >= 0xf0 ? 4 : 3; // up to 4th or 3rd
            for(size_t i = 1; i < e; i++) {
                uint8_t a = *(uint8_t *)cur;
                if(isEOF() || a < 0x80 || a >= 0xc0)
                    return false;
                ret += *cur++;
            }
        }
        return true;
    }
    e_token getKey() {
        string key;
        for(;;) {
            if(isEOF() || strchr("]}, \n\r\t", *cur) != nullptr) {
                if(key == "true")
                    return JSON_TRUE;
                if(key == "false")
                    return JSON_FALSE;
                if(key == "null")
                    return JSON_NULL;
                return JSON_INV;
            }
            if(*cur < 'a' || *cur > 'z')
                return JSON_INV;
            key += *cur++;
        }
    }

  public:
    jsonParser(bool _c = false) : useComments(_c) {}
    virtual ~jsonParser() {}
    size_t getLine() {return lineNum;}
    bool init(const string &head) {
        if(head.empty())
            return false;
        cur = head.c_str();
        lineNum = 0;
        skipBOM();
        return true;
    }
    e_token getToken() {
        for(;;) {
            if(skip_ws())
                return JSON_EOF;
            switch(*cur) {
                case '{':
                    return JSON_OBJ_STA;
                case '}':
                    return JSON_OBJ_END;
                case '[':
                    return JSON_ARR_STA;
                case ']':
                    return JSON_ARR_END;
                case ',':
                    return JSON_VAL_SEP;
                case ':':
                    return JSON_NAM_SEP;
                case '"':
                    return JSON_STR;
                case '-':
                    return JSON_NUM; // negitive number
                case '/':
                    if(!useComments || skipComments())
                        return JSON_INV;
                    break;
                default:
                    if(isdigit(*cur))
                        return JSON_NUM;
                    if(*cur >= 'a' && *cur <= 'z')
                        return getKey();
                    return JSON_INV;
            }
        }
        return JSON_INV;
    }
    void closeTk() { cur++; }
    bool getStr(string &ret) {
        ret.clear();
        cur++;
        bool useSur = false; // Have UTF-16 surrogate pair
        uint16_t W1; // UTF-16 surrogate first value
        for(;;) {
            if(isEOF())
                return false; //indicate a broken JSON
            if(useSur && *cur != '\\') // surrogate pair must follow
                return false;
            switch(*cur) {
                case '"': // End of string!
                    cur++;
                    return true;
                case '\\': // Handle escape
                    cur++;
                    if(isEOF())
                        return false; //indicate a broken JSON
                    if(useSur && *cur != 'u') // surrogate pair must follow
                        return false;
                    switch(*cur) {
                        case '"': // quotation mark
                            FALLTHROUGH;
                        case '\\': // reverse solidus
                            FALLTHROUGH;
                        case '/': // solidus
                            ret += *cur;
                            cur++;
                            break;
                        case 'b': // backspace
                            ret += '\b';
                            cur++;
                            break;
                        case 'f': // form feed
                            ret += '\f';
                            cur++;
                            break;
                        case 'n': // line feed
                            ret += '\n';
                            cur++;
                            break;
                        case 'r': // carriage return
                            ret += '\r';
                            cur++;
                            break;
                        case 't': // horizontal tabulation
                            ret += '\t';
                            cur++;
                            break;
                        case 'u':
                            cur++;
                            if(!uProc(ret, useSur, W1))
                                return false;
                            break;
                        default:
                            return false; // Invalid
                    }
                    break;
                default:
                    if(!checkUTF8(ret))
                        return false;
                    break;
            }
        }
    }
    bool getNum(string &ret, size_t &dot_loc, size_t &e_loc) {
        ret.clear();
        dot_loc = 0;
        e_loc = 0;
        if(isEOF())
            return false;
        char first = *cur++;
        bool haveZero = false;
        size_t locZero = 0;
        bool lastNotDigit = true;
        bool zeroProhib = false;
        switch(first) {
            case '0':
                haveZero = true;
                locZero = 0;
                lastNotDigit = false;
                zeroProhib = true;
                break;
            case '-':
                if(isEOF())
                    return false;
                if(*cur == '0') {
                    haveZero = true;
                    locZero = 1;
                }
                zeroProhib = false;
                lastNotDigit = true;
                break;
            default:
                lastNotDigit = isdigit(first) == 0;
                if(lastNotDigit)
                    return false;
                zeroProhib = false;
                break;
        }
        ret += first;
        size_t loc = 1;
        for(;;) {
            if(isEOF() || strchr("]}, \n\r\t", *cur) != nullptr) {
                if(lastNotDigit)
                    return false;
                return true;
            }
            bool curNotDigit = true;
            switch(*cur) {
                case 'e':
                    FALLTHROUGH;
                case 'E':
                    if(e_loc > 0 || lastNotDigit) // can only happens once!
                        return false;
                    e_loc = loc;
                    haveZero = false;
                    zeroProhib = true;
                    break;
                case '.':
                    if(e_loc > 0 || dot_loc > 0 || lastNotDigit)
                        return false;
                    dot_loc = loc;
                    haveZero = false;
                    zeroProhib = false;
                    break;
                case '+':
                    FALLTHROUGH;
                case '-':
                    if(e_loc == 0 || loc != e_loc + 1) // Only after 'e'
                        return false;
                    zeroProhib = true;
                    break;
                default:
                    if(!isdigit(*cur) || (haveZero && loc > locZero) ||
                        (*cur == '0' && zeroProhib))
                        return false;
                    curNotDigit = false;
                    zeroProhib = false;
                    break;
            }
            lastNotDigit = curNotDigit;
            if(*cur != '+') {
                ret += *cur;
                loc++;
            }
            cur++;
        }
    }
    jsonValueBase *ParserCreate(e_token tk) {
        switch(tk) {
            case JSON_OBJ_STA:
                cur++;
                return new jsonObject;
            case JSON_ARR_STA:
                cur++;
                return new jsonArray;
            case JSON_STR:
                return new jsonValue(t_string);
            case JSON_NUM:
                return new jsonValue(t_number);
            case JSON_TRUE:
                return new jsonValue(true);
            case JSON_FALSE:
                return new jsonValue(false);
            case JSON_NULL:
                return new jsonValue();
            default:
                break;
        }
        return nullptr;
    }
};

class jsonParserFile : public jsonParser
{
  private:
    FILE *f = nullptr;
    char buf[lineSize] = { 0 };

  public:
    jsonParserFile(bool _c = false) : jsonParser(_c) {}
    ~jsonParserFile() override {
        if(f != nullptr)
            fclose(f);
    }
    bool open(const string &name) {
        if(name.empty())
            return false;
        f = fopen(name.c_str(), "r");
        // Fail or empty file
        if(f == nullptr || fgets(buf, lineSize, f) == nullptr)
            return false;
        cur = buf;
        lineNum = 0;
        skipBOM();
        return true;
    }
    bool isEOF() override {
        if(*cur == 0) {
            if(fgets(buf, lineSize, f) == nullptr)
                return true; // EOF
            lineNum++;
            cur = buf;
        }
        return false;
    }
};
bool jsonValue::parserVal(jsonParser *_p)
{
    switch(m_type) {
        case t_string:
            return _p->getStr(val);
        case t_number:
            return _p->getNum(val, dot_loc, e_loc);
        case t_boolean:
            FALLTHROUGH;
        case t_null:
            return true;
        default:
            break;
    }
    return false;
}
bool jsonObject::parserVal(jsonParser *_p)
{
    e_token tk = _p->getToken();
    if(tk == JSON_OBJ_END) {
        _p->closeTk();
        return true;
    }
    for(;;) {
        // Key
        string key;
        tk = _p->getToken();
        if(tk != JSON_STR || !_p->getStr(key))
            return false;
        // member seperator
        tk = _p->getToken();
        if(tk != JSON_NAM_SEP)
            return false;
        _p->closeTk();
        // Value
        tk = _p->getToken();
        jsonValueBase *e = _p->ParserCreate(tk);
        if(e == nullptr)
            return false;
        if(!e->parserVal(_p)) {
            delete e;
            return false;
        }
        members.insert(make_pair(key, e));
        // Separetor or end of object
        tk = _p->getToken();
        switch(tk) {
            case JSON_OBJ_END:
                _p->closeTk();
                return true;
            case JSON_VAL_SEP:
                _p->closeTk();
                break;
            default:
                return false;
        }
    }
    return false;
}
bool jsonArray::parserVal(jsonParser *_p)
{
    e_token tk = _p->getToken();
    if(tk == JSON_ARR_END) {
        _p->closeTk();
        return true;
    }
    for(;;) {
        jsonValueBase *e = _p->ParserCreate(tk);
        if(e == nullptr)
            return false;
        if(!e->parserVal(_p)) {
            delete e;
            return false;
        }
        elements.push_back(e);
        tk = _p->getToken();
        switch(tk) {
            case JSON_ARR_END:
                _p->closeTk();
                return true;
            case JSON_VAL_SEP:
                _p->closeTk();
                tk = _p->getToken();
                break;
            default:
                return false;
        }
    }
    return false;
}
bool jsonMain::paresJson(jsonParser *_p)
{
    e_token tk = _p->getToken();
    if(tk == JSON_EOF)
        return true; // empty JSON
    jsonValueBase *m = _p->ParserCreate(tk);
    if(m == nullptr)
        return false;
    if(!m->parserVal(_p) || _p->getToken() != JSON_EOF) {
        delete m;
        return false;
    }
    delete main;
    main = m;
    return true;
}
jsonMain::~jsonMain()
{
    delete main;
}
bool jsonMain::parseFile(const string &name, bool useComments)
{
    jsonParserFile _p(useComments);
    return _p.open(name) && paresJson(&_p);
}
bool jsonMain::parseBuffer(const string &buffer, bool useComments)
{
    jsonParser _p(useComments);
    return _p.init(buffer) && paresJson(&_p);
}
bool jsonMain::empty() const
{
    return main == nullptr;
}
e_type jsonMain::getType() const
{
    return main != nullptr ? main->getType() : t_non;
}
string jsonMain::toString(size_t ident) const
{
    return main != nullptr ? main->toString(ident) : "";
}
static inline bool isJsonValue(e_type type)
{
    switch(type) {
        case t_string:
            FALLTHROUGH;
        case t_number:
            FALLTHROUGH;
        case t_boolean:
            FALLTHROUGH;
        case t_null:
            return true;
        default:
            return false;
    }
}
jsonValue *jsonMain::getVal() const
{
    return main != nullptr ? main->getVal() : nullptr;
};
jsonObject *jsonMain::getObj() const
{
    return main != nullptr ? main->getObj() : nullptr;
}
jsonArray *jsonMain::getArr() const
{
    return main != nullptr ? main->getArr() : nullptr;
}
jsonValueBase::jsonValueBase(e_type type) : m_type(type)
{
}
jsonValueBase::~jsonValueBase()
{
}
static inline string jUtf16(uint16_t val)
{
    char b[10];
    sprintf(b, "\\""u%.4x", val);
    return b;
}
string jsonValueBase::strToStr(const string &str)
{
    string ret = "\"";
    const size_t sz = str.size();
    for(size_t i = 0; i < sz; i++) {
        switch(str[i]) {
            case '\\': // reverse solidus
            case '"': // quotation mark
            case '/': // solidus
                ret += '\\';
                ret += str[i];
                break;
            case '\b': // backspace
                ret += "\\b";
                break;
            case '\f': // form feed
                ret += "\\f";
                break;
            case '\n': // line feed
                ret += "\\n";
                break;
            case '\r': // carriage return
                ret += "\\r";
                break;
            case '\t': // horizontal tabulation
                ret += "\\t";
                break;
            default: {
                uint8_t v = (uint8_t)str[i];
                if(v >= 0x20 && v < 0x7f)
                    // 1 UTF-8 printable byte are passes as is
                    ret += str[i];
                else {
                    // Convert UTF-8 to Unicode
                    uint32_t val = v; // Unicode code
                    if(v > 0x7f) {
                        size_t b = 0; // Number of extra characters
                        bool wrong_utf8 = false;
                        if((v & 0xe0) == 0xc0) {
                            val &= 0x1f;
                            b = 1;
                        } else if((v & 0xf0) == 0xe0) {
                            val &= 0xf;
                            b = 2;
                        } else if((v & 0xf8) == 0xf0) {
                            val &= 7;
                            b = 3;
                        } else
                            wrong_utf8 = true;
                        for(size_t j = 0; j < b; j++) {
                            if(((uint8_t)str[i + j + 1] & 0xc0) != 0x80) {
                                wrong_utf8 = true;
                                break;
                            }
                        }
                        /*
                         * Check if UTF8 is too short
                         * Or we use a wrong code
                         * Then use original string, as it is NOT a proper UTF-8
                         * We do not know how to translate.
                         */
                        if(wrong_utf8 || i + b > sz) {
                            ret = "\"";
                            ret += str + "\"";
                            return ret;
                        }
                        // Translate UTF-8 back to unicode
                        for(size_t j = 0; j < b; j++) {
                            val <<= 6;
                            val |= (0x3f & (uint8_t)str[++i]);
                        }
                    }
                    // JSON escape with UTF-16
                    if(val <= UINT16_MAX)
                        ret += jUtf16(val);
                    else {
                        // UTF-16 surrogate pair
                        val -= 0x10000;
                        uint16_t val2 = (val & 0xbff) | 0xdc00;
                        val >>= 10;
                        uint16_t val1 = (val & 0xbff) | 0xd800;
                        ret += jUtf16(val1);
                        ret += jUtf16(val2);
                    }
                }
                break;
            }
        }
    }
    ret += "\"";
    return ret;
}
e_type jsonValueBase::getType() const
{
    return m_type;
}
#define cast1(n) dynamic_cast<n *>(const_cast<jsonValueBase *>(this))
#define cast2(n) const_cast<decltype(n)*>(&n)
jsonValue *jsonValueBase::getVal() const
{
    return isJsonValue(m_type) ? cast1(jsonValue) : nullptr;
}
jsonObject *jsonValueBase::getObj() const
{
    return m_type == t_object ? cast1(jsonObject) : nullptr;
}
jsonArray *jsonValueBase::getArr() const
{
    return m_type == t_array ? cast1(jsonArray) : nullptr;
}
jsonValue::jsonValue(e_type type) : jsonValueBase(type) {}
jsonValue::jsonValue(bool boolean) : jsonValueBase(t_boolean),
    valBool(boolean) {}
jsonValue::jsonValue() : jsonValueBase(t_null) {}
jsonValue::jsonValue(const string &str) : jsonValueBase(t_string), val(str) {}
const char *jsonValue::getCStr() const
{
    return val.c_str();
}
const string &jsonValue::getStr() const
{
    return val;
}
size_t jsonValue::getStrLen() const
{
    return val.length();
}
bool jsonValue::getBool() const
{
    return valBool;
}
bool jsonValue::getInt64(int64_t &_val, bool flexible) const
{
    int base = 10;
    switch(m_type) {
        case t_number:
            if(dot_loc > 0 || e_loc > 0)
                return false;
            if(val[0] == '0') {
                // Leading zero means zero or fractional
                // as dot_loc is zero, it must be zero!
                _val = 0;
                return true;
            }
            break;
        case t_string:
            if(flexible) {
                base = 0;
                break;
            }
            return false;
        case t_boolean:
            if(flexible) {
                _val = valBool ? 1 : 0;
                return true;
            }
            return false;
        case t_null:
            if(flexible) {
                _val = 0;
                return true;
            }
            return false;
        default:
            return false;
    }
    char *endptr = nullptr;
    _val = (int64_t)strtoimax(val.c_str(), &endptr, base);
    return endptr != nullptr && (*endptr == 0 ||
            (flexible && strchr(" \n\r\t", *endptr) != nullptr));
}
bool jsonValue::getUint64(uint64_t &_val, bool flexible) const
{
    int base = 10;
    switch(m_type) {
        case t_number:
            if(dot_loc > 0 || e_loc > 0 || val[0] == '-')
                return false;
            if(val[0] == '0') {
                // Leading zero means zero or fractional
                // as dot_loc is zero, it must be zero!
                _val = 0;
                return true;
            }
            break;
        case t_string:
            if(flexible) {
                base = 0;
                break;
            }
            return false;
        case t_boolean:
            if(flexible) {
                _val = valBool ? 1 : 0;
                return true;
            }
            return false;
        case t_null:
            if(flexible) {
                _val = 0;
                return true;
            }
            return false;
        default:
            return false;
    }
    char *endptr = nullptr;
    _val = (uint64_t)strtoumax(val.c_str(), &endptr, base);
    return endptr != nullptr && (*endptr == 0 ||
            (flexible && strchr(" \n\r\t", *endptr) != nullptr));
}
bool jsonValue::getFloat(long double &_val) const
{
    if(m_type == t_number) {
        size_t pos;
        _val = stold(val, &pos);
        if(pos == val.length())
            return true;
    }
    return false;
}
bool jsonValue::getFrac(int64_t &integer, uint64_t &fraction,
    size_t fracSize) const
{
    if(m_type == t_number && e_loc == 0) {
        if(dot_loc == 0) {
            fraction = 0;
            return getInt64(integer);
        }
        size_t aDot = val.length() - dot_loc - 1;
        if(aDot <= fracSize) {
            const string before = val.substr(0, dot_loc);
            char *endptr = nullptr;
            integer = (int64_t)strtoimax(before.c_str(), &endptr, 10);
            if(endptr == nullptr || *endptr != 0)
                return false;
            const string after = val.substr(dot_loc + 1);
            endptr = nullptr;
            fraction = (uint64_t)strtoumax(after.c_str(), &endptr, 10);
            if(endptr == nullptr || *endptr != 0)
                return false;
            /*for(size_t i = aDot; i < fracSize; i++)
                  fraction *= 10; */
            size_t pow10 = fracSize - aDot;
            if(pow10 > 0)
                fraction *= (uint64_t)pow((double)10, (double)pow10);
            return true;
        }
    }
    return false;
}
string jsonValue::toString(size_t) const
{
    switch(m_type) {
        case t_string:
            return strToStr(val);
        case t_number:
            return val;
        case t_boolean:
            return valBool ? "true" : "false";
        case t_null:
            return "null";
        default:
            return "";
    }
}
jsonObject::jsonObject() : jsonValueBase(t_object)
{
}
jsonObject::~jsonObject()
{
    for(auto &i : members)
        delete i.second;
}
size_t jsonObject::size() const
{
    return members.size();
}
size_t jsonObject::count(const string &key) const
{
    return members.count(key);
}
obj_iter jsonObject::find(const string &key) const
{
    return cast2(members)->find(key);
}
std::pair<obj_iter, obj_iter> jsonObject::equal_range(const std::string &key)
const
{
    return cast2(members)->equal_range(key);
}
obj_iter jsonObject::begin() const
{
    return cast2(members)->begin();
}
obj_iter jsonObject::end() const
{
    return cast2(members)->end();
}
e_type jsonObject::getType(const obj_iter &iter) const
{
    return iter->second->getType();
}
jsonValue *jsonObject::getVal(const obj_iter &iter) const
{
    return iter->second->getVal();
}
jsonObject *jsonObject::getObj(const obj_iter &iter) const
{
    return iter->second->getObj();
}
jsonArray *jsonObject::getArr(const obj_iter &iter) const
{
    return iter->second->getArr();
}
e_type jsonObject::getType(const string &key) const
{
    return count(key) == 1 ? getType(find(key)) : t_non;
}
jsonValue *jsonObject::getVal(const string &key) const
{
    return count(key) == 1 ? getVal(find(key)) : nullptr;
}
jsonObject *jsonObject::getObj(const string &key) const
{
    return count(key) == 1 ? getObj(find(key)) : nullptr;
}
jsonArray *jsonObject::getArr(const string &key) const
{
    return count(key) == 1 ? getArr(find(key)) : nullptr;
}
string jsonObject::toString(size_t ident) const
{
    string it1(2 * ident, ' ');
    string ret(it1 + "{\n");
    if(!members.empty()) {
        for(const auto &m : members)
            ret += it1 + "  " + strToStr(m.first) + " : " +
                m.second->toString(ident + 2) + ",\n";
        ret.erase(ret.size() - 2, 1); // Remove the last comma
    }
    ret += it1 + "}";
    return ret;
}
jsonArray::jsonArray() : jsonValueBase(t_array)
{
}
jsonArray::~jsonArray()
{
    for(auto &i : elements)
        delete i;
}
size_t jsonArray::size() const
{
    return elements.size();
}
arr_iter jsonArray::begin() const
{
    return cast2(elements)->begin();
}
arr_iter jsonArray::end() const
{
    return cast2(elements)->end();
}
e_type jsonArray::getType(size_t index) const
{
    return index < elements.size() ? elements[index]->getType() : t_non;
}
jsonValue *jsonArray::getVal(size_t index) const
{
    return index < elements.size() ? elements[index]->getVal() : nullptr;
}
jsonObject *jsonArray::getObj(size_t index) const
{
    return index < elements.size() ? elements[index]->getObj() : nullptr;
}
jsonArray *jsonArray::getArr(size_t index) const
{
    return index < elements.size() ? elements[index]->getArr() : nullptr;
}
e_type jsonArray::getType(const arr_iter &iterator) const
{
    return (*iterator)->getType();
}
jsonValue *jsonArray::getVal(const arr_iter &iterator) const
{
    return (*iterator)->getVal();
}
jsonObject *jsonArray::getObj(const arr_iter &iterator) const
{
    return (*iterator)->getObj();
}
jsonArray *jsonArray::getArr(const arr_iter &iterator) const
{
    return (*iterator)->getArr();
}
string jsonArray::toString(size_t ident) const
{
    string it1(2 * ident, ' ');
    string ret = it1 + "[\n";
    if(!elements.empty()) {
        for(const auto &e : elements)
            ret += it1 + "  " + e->toString(ident + 2) + ",\n";
        ret.erase(ret.size() - 2, 1); // Remove the last comma
    }
    ret += it1 + "]";
    return ret;
}

const char *jsonType2str(e_type type)
{
    switch(type) {
        case t_string:
            return "string";
        case t_number:
            return "number";
        case t_boolean:
            return "boolean";
        case t_null:
            return "null";
        case t_object:
            return "object";
        case t_array:
            return "array";
        default:
            break;
    }
    return "unkown";
}
