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
    bool uProc(std::string &ret, bool &useSur, uint16_t &W1) {
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
    bool checkUTF8(std::string &ret) {
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
        std::string key;
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
    bool init(const std::string &head) {
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
    bool getStr(std::string &ret) {
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
    bool getNum(std::string &ret, size_t &dot_loc, size_t &e_loc) {
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
                return new jsonValue(t_boolean, true);
            case JSON_FALSE:
                return new jsonValue(t_boolean, false);
            case JSON_NULL:
                return new jsonValue(t_null);
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
    bool open(const std::string &name) {
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
        std::string key;
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
        members.emplace(std::make_pair(key, e));
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
bool jsonMain::parseFile(const std::string &name, bool useComments)
{
    jsonParserFile _p(useComments);
    return _p.open(name) && paresJson(&_p);
}
bool jsonMain::parseBuffer(const std::string &buffer, bool useComments)
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
static inline bool isJsonVal(e_type type)
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
    return main != nullptr &&
        isJsonVal(main->getType()) ? dynamic_cast<jsonValue *>(main) : nullptr;
};
jsonObject *jsonMain::getObj() const
{
    return main != nullptr && main->getType() == t_object ?
        dynamic_cast<jsonObject *>(main) : nullptr;
}
jsonArray *jsonMain::getArr() const
{
    return main != nullptr && main->getType() == t_array ?
        dynamic_cast<jsonArray *>(main) : nullptr;
}
jsonValueBase::jsonValueBase(e_type type) : m_type(type)
{
}
jsonValueBase::~jsonValueBase()
{
}
e_type jsonValueBase::getType() const
{
    return m_type;
}
jsonValue *jsonValueBase::getVal()
{
    return isJsonVal(m_type) ? dynamic_cast<jsonValue *>(this) : nullptr;
}
jsonObject *jsonValueBase::getObj()
{
    return m_type == t_object ? dynamic_cast<jsonObject *>(this) : nullptr;
}
jsonArray *jsonValueBase::getArr()
{
    return m_type == t_array ? dynamic_cast<jsonArray *>(this) : nullptr;
}
jsonValue::jsonValue(e_type type, bool boolean) : jsonValueBase(type),
    valBool(boolean)
{
}
const char *jsonValue::getCStr() const
{
    return val.c_str();
}
const std::string &jsonValue::getStr() const
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
            const std::string before = val.substr(0, dot_loc);
            char *endptr = nullptr;
            integer = (int64_t)strtoimax(before.c_str(), &endptr, 10);
            if(endptr == nullptr || *endptr != 0)
                return false;
            const std::string after = val.substr(dot_loc + 1);
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
size_t jsonObject::count(const std::string &key) const
{
    return members.count(key);
}
obj_iter jsonObject::find(const std::string &key)
{
    return members.find(key);
}
obj_iter jsonObject::begin()
{
    return members.begin();
}
obj_iter jsonObject::end()
{
    return members.end();
}
e_type jsonObject::getMulType(const obj_iter &iter) const
{
    return iter->second->getType();
}
jsonValue *jsonObject::getMulVal(const obj_iter &iter) const
{
    return isJsonVal(iter->second->getType()) ?
        dynamic_cast<jsonValue *>(iter->second) : nullptr;
}
jsonObject *jsonObject::getMulObj(const obj_iter &iter) const
{
    return iter->second->getType() == t_object ?
        dynamic_cast<jsonObject *>(iter->second) : nullptr;
}
jsonArray *jsonObject::getMulArr(const obj_iter &iter) const
{
    return iter->second->getType() == t_array ?
        dynamic_cast<jsonArray *>(iter->second) : nullptr;
}
e_type jsonObject::getType(const std::string &key)
{
    return count(key) == 1 ? find(key)->second->getType() : t_non;
}
jsonValue *jsonObject::getVal(const std::string &key)
{
    return count(key) == 1 ? getMulVal(find(key)) : nullptr;
}
jsonObject *jsonObject::getObj(const std::string &key)
{
    return count(key) == 1 ? getMulObj(find(key)) : nullptr;
}
jsonArray *jsonObject::getArr(const std::string &key)
{
    return count(key) == 1 ? getMulArr(find(key)) : nullptr;
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
arr_iter jsonArray::begin()
{
    return elements.begin();
}
arr_iter jsonArray::end()
{
    return elements.end();
}
e_type jsonArray::getType(size_t index) const
{
    return index < elements.size() ? elements[index]->getType() : t_non;
}
jsonValue *jsonArray::getVal(size_t index) const
{
    return index < elements.size() &&
        isJsonVal(elements[index]->getType()) ?
        dynamic_cast<jsonValue *>(elements[index]) : nullptr;
}
jsonObject *jsonArray::getObj(size_t index) const
{
    return index < elements.size() && elements[index]->getType() == t_object ?
        dynamic_cast<jsonObject *>(elements[index]) : nullptr;
}
jsonArray *jsonArray::getArr(size_t index) const
{
    return index < elements.size() && elements[index]->getType() == t_array ?
        dynamic_cast<jsonArray *>(elements[index]) : nullptr;
}
e_type jsonArray::getType(const arr_iter &iterator) const
{
    return (*iterator)->getType();
}
jsonValue *jsonArray::getVal(const arr_iter &iterator) const
{
    return isJsonVal((*iterator)->getType()) ?
        dynamic_cast<jsonValue *>(*iterator) : nullptr;
}
jsonObject *jsonArray::getObj(const arr_iter &iterator) const
{
    return (*iterator)->getType() == t_object ?
        dynamic_cast<jsonObject *>(*iterator) : nullptr;
}
jsonArray *jsonArray::getArr(const arr_iter &iterator) const
{
    return (*iterator)->getType() == t_array ?
        dynamic_cast<jsonArray *>(*iterator) : nullptr;
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
