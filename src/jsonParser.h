/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief parse JSON header
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#ifndef __PTPMGMT_JSON_PARSER_H
#define __PTPMGMT_JSON_PARSER_H

#include <map>
#include <vector>
#include <string>
#include <cstdint>

class jsonMain;
class jsonParser;
class jsonValue;
class jsonObject;
class jsonArray;

/** jsonValue type */
enum e_type {
    t_non,     /**< Not exist */
    t_string,  /**< String value */
    t_number,  /**< Number value */
    t_boolean, /**< Boolean value */
    t_null,    /**< null value */
    t_object,  /**< Value is object */
    t_array,   /**< Value is array */
};

/**
  * Convert type to string
  * @param[in] type
  * @return string
  */
const char *jsonType2str(e_type type);

/** Base class for jsonValue */
class jsonValueBase
{
  protected:
    e_type m_type; /**< value type */

    /**
      * parse JSON value
      * @param[in] parser object
      * @return valid JSON parsed
      */
    virtual bool parserVal(jsonParser *parser) = 0;
    /**
      * Constractor
      * @param[in] type value
      */
    jsonValueBase(e_type type);

    friend class jsonMain;
    friend class jsonObject;
    friend class jsonArray;

  public:

    virtual ~jsonValueBase();

    /**
      * Get value type
      * @return value type
      */
    e_type getType() const;
    /**
      * Get value object
      * @return value object
      */
    jsonValue *getVal();
    /**
      * Get JSON object object
      * @return JSON object object
      */
    jsonObject *getObj();
    /**
      * Get JSON array object
      * @return JSON array object
      */
    jsonArray *getArr();
};

/** class jsonValue for string, number, boolean or null value */
class jsonValue : public jsonValueBase
{
  private:
    std::string val;
    bool valBool;
    size_t dot_loc = 0;
    size_t e_loc = 0;

  protected:
    /**
      * parse JSON value
      * @param[in] parser object
      * @return valid JSON parsed
      */
    bool parserVal(jsonParser *parser) override;

  public:
    /**
      * Constractor
      * @param[in] type value
      * @param[in] boolean value
      */
    jsonValue(e_type type, bool boolean = false);
    /**
      * Get String value
      * @return pointer to a C string
      */
    const char *getCStr() const;
    /**
      * Get String value
      * @return String
      */
    const std::string &getStr() const;
    /**
      * Get String length
      * @return length
      */
    size_t getStrLen() const;
    /**
      * Get boolean value
      * @return boolean value
      */
    bool getBool() const;
    /**
      * get signed 64 bits integer
      * @param[out] val integer value
      * @param[in] flexible conversion to number
      * @return true for valid value
      */
    bool getInt64(int64_t &val, bool flexible = false) const;
    /**
      * get unsigned 64 bits integer
      * @param[out] val integer value
      * @param[in] flexible conversion to number
      * @return true for valid value
      */
    bool getUint64(uint64_t &val, bool flexible = false) const;
    /**
      * get float
      * @param[out] val float value
      * @return true for valid value
      */
    bool getFloat(long double &val) const;
    /**
      * get fraction
      * @param[out] integer value
      * @param[out] fraction value
      * @param[in] fracSize size of fraction
      * @return true for valid result
      */
    bool getFrac(int64_t &integer, uint64_t &fraction, size_t fracSize) const;
};

/** Iterator type of jsonObject members */
typedef std::multimap<std::string, jsonValueBase *>::iterator obj_iter;

/** class jsonObject for holding JSON object with members */
class jsonObject : public jsonValueBase
{
  private:
    std::multimap<std::string, jsonValueBase *> members;
  protected:
    /**
      * parse JSON value
      * @param[in] parser object
      * @return valid JSON parsed
      */
    bool parserVal(jsonParser *parser) override;

  public:
    jsonObject();
    ~jsonObject();

    /**
      * Get number of members
      * @return number of members
      */
    size_t size() const;
    /**
      * Get number of members with a key
      * @param[in] key string with key value
      * @return number of members with a key
      */
    size_t count(const std::string &key) const;
    /**
      * Get members with a key
      * @param[in] key string
      * @return iterator to loop all members match the key
      */
    obj_iter find(const std::string &key);
    /**
      * Get members begin iterator
      * @return iterator to start of all members
      */
    obj_iter begin();
    /**
      * Get members end iterator
      * @return iterator to end
      */
    obj_iter end();
    /**
      * Get member value type
      * @param[in] iterator to a member
      * @return value type
      */
    e_type getMulType(const obj_iter &iterator) const;
    /**
      * Get member value object
      * @param[in] iterator to a member
      * @return value object
      */
    jsonValue *getMulVal(const obj_iter &iterator) const;
    /**
      * Get member JSON object object
      * @param[in] iterator to a member
      * @return JSON object object
      */
    jsonObject *getMulObj(const obj_iter &iterator) const;
    /**
      * Get member JSON array object
      * @param[in] iterator to a member
      * @return JSON array object
      */
    jsonArray *getMulArr(const obj_iter &iterator) const;
    /**
      * Get member value type with key
      * @param[in] key string
      * @return value type
      * @note Use on single member with this key!
      */
    e_type getType(const std::string &key);
    /**
      * Get member value object with key
      * @param[in] key string
      * @return value object
      * @note Use on single member with this key!
      */
    jsonValue *getVal(const std::string &key);
    /**
      * Get member JSON object object with key
      * @param[in] key string
      * @return JSON object object
      * @note Use on single member with this key!
      */
    jsonObject *getObj(const std::string &key);
    /**
      * Get member JSON array object with key
      * @param[in] key string
      * @return JSON array object
      * @note Use on single member with this key!
      */
    jsonArray *getArr(const std::string &key);
};

/** Iterator type of jsonArray elements */
typedef std::vector<jsonValueBase *>::iterator arr_iter;

/** class jsonArray for holding JSON array with elements */
class jsonArray : public jsonValueBase
{
  private:
    std::vector<jsonValueBase *> elements;

  protected:
    /**
      * parse JSON value
      * @param[in] parser object
      * @return valid JSON parsed
      */
    bool parserVal(jsonParser *parser) override;

  public:
    jsonArray();
    ~jsonArray();
    /**
      * Get number of elements
      * @return number of elements
      */
    size_t size() const;
    /**
      * Get members begin iterator
      * @return iterator to start of all members
      */
    arr_iter begin();
    /**
      * Get members end iterator
      * @return iterator to end
      */
    arr_iter end();
    /**
      * Get element value type with index
      * @param[in] index number
      * @return value type
      */
    e_type getType(size_t index) const;
    /**
      * Get element value object with index
      * @param[in] index number
      * @return value object
      */
    jsonValue *getVal(size_t index) const;
    /**
      * Get element JSON object object with index
      * @param[in] index number
      * @return JSON object object
      */
    jsonObject *getObj(size_t index) const;
    /**
      * Get element JSON array object with index
      * @param[in] index number
      * @return JSON array object
      */
    jsonArray *getArr(size_t index) const;
    /**
      * Get element value type with index
      * @param[in] iterator to element
      * @return value type
      */
    e_type getType(const arr_iter &iterator) const;
    /**
      * Get element value object with index
      * @param[in] iterator to element
      * @return value object
      */
    jsonValue *getVal(const arr_iter &iterator) const;
    /**
      * Get element JSON object object with index
      * @param[in] iterator to element
      * @return JSON object object
      */
    jsonObject *getObj(const arr_iter &iterator) const;
    /**
      * Get element JSON array object with index
      * @param[in] iterator to element
      * @return JSON array object
      */
    jsonArray *getArr(const arr_iter &iterator) const;
};

/** class jsonMain for holding a JSON value */
class jsonMain
{
  private:
    jsonValueBase *main = nullptr;

  protected:
    /**
      * parse JSON
      * @param[in] parser object
      * @return valid JSON parsed
      */
    bool paresJson(jsonParser *parser);

  public:
    jsonMain() = default;
    ~jsonMain();
    /**
      * Parse a JSON file
      * @param[in] file name
      * @param[in] useComments use javascript comments
      * @return valid JSON parsed
      */
    bool parseFile(const std::string &file, bool useComments = false);
    /**
      * Parse JSONtring
      * @param[in] buffer contain JSON
      * @param[in] useComments use javascript comments
      * @return valid JSON parsed
      */
    bool parseBuffer(const std::string &buffer, bool useComments = false);
    /**
      * Quary if value exist
      * @return no value exist
      */
    bool empty() const;
    /**
      * Get value type
      * @return value type
      */
    e_type getType() const;
    /**
      * Get value object
      * @return value object
      */
    jsonValue *getVal() const;
    /**
      * Get JSON object object
      * @return JSON object object
      */
    jsonObject *getObj() const;
    /**
      * Get JSON array object
      * @return JSON array object
      */
    jsonArray *getArr() const;
};

#endif /* __PTPMGMT_JSON_PARSER_H */
