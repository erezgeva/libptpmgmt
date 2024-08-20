/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief unit test Json parser
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "jsonParser.h"

TEST(jsonParser, UnicodeStrings)
{
    jsonMain j;
#define u_umlut "\xc3\x9c" // U+00dc U with umlaut
    EXPECT_TRUE(j.parseBuffer(" \"" "\\" "u00dc" "\" "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_string);
    jsonValue *s = j.getVal();
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->getStrLen(), 2);
    EXPECT_STREQ(s->getCStr(), u_umlut);
    EXPECT_TRUE(j.parseBuffer(" \"" u_umlut "\" "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_string);
    s = j.getVal();
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->getStrLen(), 2);
    EXPECT_STREQ(s->getCStr(), u_umlut);
#define heart "\xe2\x9d\xa4" // U+2764 Heavy Black Heart
    EXPECT_TRUE(j.parseBuffer(" \"" "\\" "u2764" "\" "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_string);
    s = j.getVal();
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->getStrLen(), 3);
    EXPECT_STREQ(s->getCStr(), heart);
    EXPECT_TRUE(j.parseBuffer(" \"" heart "\" "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_string);
    s = j.getVal();
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->getStrLen(), 3);
    EXPECT_STREQ(s->getCStr(), heart);
#define face "\xf0\x9f\x98\x90" // U+1F610 neutral face
    EXPECT_TRUE(j.parseBuffer(" \"" "\\" "ud83d" "\\" "ude10" "\" "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_string);
    s = j.getVal();
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->getStrLen(), 4);
    EXPECT_STREQ(s->getCStr(), face);
    EXPECT_TRUE(j.parseBuffer(" \"" face "\" "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_string);
    s = j.getVal();
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->getStrLen(), 4);
    EXPECT_STREQ(s->getCStr(), face);
    // Check escape sequance
    EXPECT_TRUE(j.parseBuffer(" \" \\\x2f \\\x5c \\\" \\b \\f \\n \\r \\t \" "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_string);
    s = j.getVal();
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->getStrLen(), 17);
    const char *a = s->getCStr();
    EXPECT_STREQ(a, " \x2f \x5c \" \b \f \n \r \t ");
    EXPECT_EQ(a[1], '/');
    EXPECT_EQ(a[3], '\\');
    EXPECT_EQ(a[5], '"');
    EXPECT_EQ(a[7], '\b');
    EXPECT_EQ(a[9], '\f');
    EXPECT_EQ(a[11], '\n');
    EXPECT_EQ(a[13], '\r');
    EXPECT_EQ(a[15], '\t');
    // Check them without escape
    EXPECT_TRUE(j.parseBuffer(" \" \x2f \b \f \n \r \t \" "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_string);
    s = j.getVal();
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->getStrLen(), 13);
    a = s->getCStr();
    EXPECT_STREQ(a, " \x2f \b \f \n \r \t ");
    EXPECT_EQ(a[ 1], '/');
    EXPECT_EQ(a[ 3], '\b');
    EXPECT_EQ(a[ 5], '\f');
    EXPECT_EQ(a[ 7], '\n');
    EXPECT_EQ(a[ 9], '\r');
    EXPECT_EQ(a[11], '\t');
    /*  printf("u_umlut |" u_umlut "|\n"
               "heart   |" heart   "|\n"
               "face    |" face    "|\n"); */
}

TEST(jsonParser, keywords)
{
    jsonMain j;
    EXPECT_TRUE(j.parseBuffer(" true \t "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_boolean);
    jsonValue *b = j.getVal();
    ASSERT_NE(b, nullptr);
    EXPECT_TRUE(b->getBool());
    EXPECT_TRUE(j.parseBuffer(" \r \n false \t "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_boolean);
    b = j.getVal();
    ASSERT_NE(b, nullptr);
    EXPECT_FALSE(b->getBool());
    EXPECT_TRUE(j.parseBuffer(" \r \n null \t "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_null);
    jsonValue *n = j.getVal();
    ASSERT_NE(n, nullptr);
    EXPECT_NE(n, nullptr);
}

TEST(jsonParser, numbers)
{
    jsonMain j;
    int64_t i;
    uint64_t u, f;
    EXPECT_TRUE(j.parseBuffer(" 0  "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_number);
    jsonValue *n = j.getVal();
    ASSERT_NE(n, nullptr);
    EXPECT_TRUE(n->getUint64(u));
    EXPECT_EQ(u, 0);
    EXPECT_TRUE(n->getFrac(i, f, 4));
    EXPECT_EQ(i, 0);
    EXPECT_EQ(f, 0);
    EXPECT_TRUE(j.parseBuffer(" 794  "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_number);
    n = j.getVal();
    ASSERT_NE(n, nullptr);
    EXPECT_TRUE(n->getUint64(u));
    EXPECT_EQ(u, 794);
    EXPECT_TRUE(n->getFrac(i, f, 4));
    EXPECT_EQ(i, 794);
    EXPECT_EQ(f, 0);
    EXPECT_TRUE(j.parseBuffer(" 50  "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_number);
    n = j.getVal();
    ASSERT_NE(n, nullptr);
    EXPECT_TRUE(n->getUint64(u));
    EXPECT_EQ(u, 50);
    EXPECT_TRUE(n->getFrac(i, f, 4));
    EXPECT_EQ(i, 50);
    EXPECT_EQ(f, 0);
    EXPECT_TRUE(j.parseBuffer(" -6427  "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_number);
    n = j.getVal();
    ASSERT_NE(n, nullptr);
    EXPECT_TRUE(n->getInt64(i));
    EXPECT_EQ(i, -6427);
    EXPECT_TRUE(n->getFrac(i, f, 9));
    EXPECT_EQ(i, -6427);
    EXPECT_EQ(f, 0);
    EXPECT_TRUE(j.parseBuffer(" 0.78  "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_number);
    n = j.getVal();
    ASSERT_NE(n, nullptr);
    long double d;
    EXPECT_TRUE(n->getFloat(d));
    EXPECT_DOUBLE_EQ(d, 0.78);
    EXPECT_TRUE(n->getFrac(i, f, 5));
    EXPECT_EQ(i, 0);
    EXPECT_EQ(f, 78000);
    EXPECT_TRUE(j.parseBuffer(" 9.0078  "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_number);
    n = j.getVal();
    ASSERT_NE(n, nullptr);
    EXPECT_TRUE(n->getFloat(d));
    EXPECT_DOUBLE_EQ(d, 9.0078);
    EXPECT_TRUE(n->getFrac(i, f, 5));
    EXPECT_EQ(i, 9);
    EXPECT_EQ(f, 780);
    EXPECT_TRUE(j.parseBuffer("   9.000709000 "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_number);
    n = j.getVal();
    ASSERT_NE(n, nullptr);
    EXPECT_TRUE(n->getFloat(d));
    EXPECT_DOUBLE_EQ(d, 9.000709);
    EXPECT_TRUE(n->getFrac(i, f, 9));
    EXPECT_EQ(i, 9);
    EXPECT_EQ(f, 709000);
    EXPECT_TRUE(j.parseBuffer(" 0.78e-45 "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_number);
    n = j.getVal();
    ASSERT_NE(n, nullptr);
    EXPECT_TRUE(n->getFloat(d));
    EXPECT_DOUBLE_EQ(d, 0.78e-45);
    EXPECT_TRUE(j.parseBuffer(" 0.78e+37 "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_number);
    n = j.getVal();
    ASSERT_NE(n, nullptr);
    EXPECT_TRUE(n->getFloat(d));
    EXPECT_DOUBLE_EQ(d, 0.78e37);
    EXPECT_TRUE(j.parseBuffer(" 0.8e37 "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_number);
    n = j.getVal();
    ASSERT_NE(n, nullptr);
    EXPECT_TRUE(n->getFloat(d));
    EXPECT_DOUBLE_EQ(d, 0.8e37);
    EXPECT_TRUE(j.parseBuffer(" -8.4e37 "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_number);
    n = j.getVal();
    ASSERT_NE(n, nullptr);
    EXPECT_TRUE(n->getFloat(d));
    EXPECT_DOUBLE_EQ(d, -8.4e37);
    EXPECT_TRUE(j.parseBuffer(" -8e+37 "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_number);
    n = j.getVal();
    ASSERT_NE(n, nullptr);
    EXPECT_TRUE(n->getFloat(d));
    EXPECT_DOUBLE_EQ(d, -8e37);
    EXPECT_TRUE(j.parseBuffer(" -846e-37 "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_number);
    n = j.getVal();
    ASSERT_NE(n, nullptr);
    EXPECT_TRUE(n->getFloat(d));
    EXPECT_DOUBLE_EQ(d, -846e-37);
    EXPECT_TRUE(j.parseBuffer(" -8.4 "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_number);
    n = j.getVal();
    ASSERT_NE(n, nullptr);
    EXPECT_TRUE(n->getFloat(d));
    EXPECT_DOUBLE_EQ(d, -8.4);
    EXPECT_TRUE(n->getFrac(i, f, 4));
    EXPECT_EQ(i, -8);
    EXPECT_EQ(f, 4000);
}

TEST(jsonParser, type2str)
{
    EXPECT_STREQ(jsonType2str(t_string), "string");
    EXPECT_STREQ(jsonType2str(t_number), "number");
    EXPECT_STREQ(jsonType2str(t_boolean), "boolean");
    EXPECT_STREQ(jsonType2str(t_null), "null");
    EXPECT_STREQ(jsonType2str(t_object), "object");
    EXPECT_STREQ(jsonType2str(t_array), "array");
    EXPECT_STREQ(jsonType2str(t_non), "unkown");
};

TEST(jsonParser, comments)
{
    jsonMain j;
    const char j1[] =
        "// This is a comment\n"
        "true \t \n"
        "// This is after";
    EXPECT_TRUE(j.parseBuffer(j1, true));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_boolean);
    jsonValue *b = j.getVal();
    ASSERT_NE(b, nullptr);
    EXPECT_TRUE(b->getBool());
    const char j2[] =
        "/* This is a multi line comment\n"
        "   Til here */"
        "// This is also\n"
        "true \t \n"
        "// This is after\n"
        "/* And more */ /* and  \n"
        "   never stop */";
    EXPECT_TRUE(j.parseBuffer(j2, true));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_boolean);
    b = j.getVal();
    ASSERT_NE(b, nullptr);
    EXPECT_TRUE(b->getBool());
}

TEST(jsonParser, arrays)
{
    jsonMain j;
    EXPECT_TRUE(j.parseBuffer(" [ \" 1\" , 0 ] "));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_array);
    jsonArray *a = j.getArr();
    ASSERT_NE(a, nullptr);
    EXPECT_EQ(a->size(), 2);
    EXPECT_EQ(a->getType(0), t_string);
    jsonValue *s = a->getVal(0);
    ASSERT_NE(s, nullptr);
    EXPECT_STREQ(s->getCStr(), " 1");
    EXPECT_EQ(a->getType(1), t_number);
    jsonValue *n = a->getVal(1);
    ASSERT_NE(n, nullptr);
    uint64_t u;
    EXPECT_TRUE(n->getUint64(u));
    EXPECT_EQ(u, 0);
}

TEST(jsonParser, objects)
{
    const char j1[] =
        "{ \"actionField\"   : \"GET\" ,       "
        "  \"managementId\"  : \"PRIORITY1\" , "
        "  \"Val\"           : -144            "
        "}                                     ";
    jsonMain j;
    EXPECT_TRUE(j.parseBuffer(j1));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_object);
    jsonObject *o = j.getObj();
    ASSERT_NE(o, nullptr);
    EXPECT_EQ(o->size(), 3);
    EXPECT_EQ(o->getType("actionField"), t_string);
    jsonValue *s = o->getVal("actionField");
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->getStrLen(), 3);
    EXPECT_STREQ(s->getCStr(), "GET");
    EXPECT_EQ(o->getType("managementId"), t_string);
    s = o->getVal("managementId");
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->getStrLen(), 9);
    EXPECT_STREQ(s->getCStr(), "PRIORITY1");
    EXPECT_EQ(o->getType("Val"), t_number);
    jsonValue *n = o->getVal("Val");
    ASSERT_NE(n, nullptr);
    int64_t i;
    EXPECT_TRUE(n->getInt64(i));
    EXPECT_EQ(i, -144);
}

TEST(jsonParser, file)
{
    jsonMain j;
    EXPECT_TRUE(j.parseFile("utest/test.json", true));
    ASSERT_FALSE(j.empty());
    EXPECT_EQ(j.getType(), t_object);
    jsonObject *o = j.getObj();
    ASSERT_NE(o, nullptr);
    EXPECT_EQ(o->size(), 1);
    EXPECT_EQ(o->getType("glossary"), t_object);
    jsonObject *o1 = o->getObj("glossary");
    ASSERT_NE(o1, nullptr);
    EXPECT_EQ(o1->size(), 2);
    EXPECT_EQ(o1->getType("title"), t_string);
    jsonValue *s = o1->getVal("title");
    ASSERT_NE(s, nullptr);
    EXPECT_STREQ(s->getCStr(), "example glossary");
    jsonObject *o2 = o1->getObj("GlossDiv");
    ASSERT_NE(o2, nullptr);
    EXPECT_EQ(o2->size(), 2);
    EXPECT_EQ(o2->getType("title"), t_string);
    s = o2->getVal("title");
    ASSERT_NE(s, nullptr);
    EXPECT_STREQ(s->getCStr(), "S");
    jsonObject *o3 = o2->getObj("GlossList");
    ASSERT_NE(o3, nullptr);
    EXPECT_EQ(o3->size(), 1);
    jsonObject *o4 = o3->getObj("GlossEntry");
    ASSERT_NE(o4, nullptr);
    EXPECT_EQ(o4->size(), 7);
    EXPECT_EQ(o4->getType("Abbrev"), t_string);
    s = o4->getVal("Abbrev");
    ASSERT_NE(s, nullptr);
    EXPECT_STREQ(s->getCStr(), "ISO 8879:1986");
}
