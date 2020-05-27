# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------------
# Copyright: AviBit GmbH, 2001-2015
# ----------------------------------------------------------------------------

"""
.. moduleauthor:: Christian Muschick <c.muschick@avibit.com>
"""

# Python 3 compatibility
#from __future__ import print_function, unicode_literals, absolute_import, division
#from future.builtins import *

# Python 3 compatibility
from __future__ import print_function, unicode_literals, absolute_import, division
from future.builtins import *

import types

import pytest

from datetime import datetime
from enum import Enum

from avlib import avfromtostring
from avlib.avfromtostring import _nested_split, _escape, ParseError, avfromstring, avtostring, StringList, IntList, register_enum

###########################################################################################

def test_escape():
    string = "axbsomeothertextab"

    from_items = ["a", "b"]
    to_items   = ["aa", "ab"]

    escaped = _escape(string, from_items, to_items)

    assert(escaped == "aaxabsomeothertextaaab")
    # Verify backward conversion
    assert (string == _escape(escaped, to_items, from_items))

###########################################################################################

def test_nested_split():
    str = "[  test    ] ";
    split = _nested_split(str, True)
    assert(split == ["test"])

    str = " [abc[[]];;def];abc;\"];   \";  ;  "
    split = _nested_split(str, False)
    assert(split == ["[abc[[]];;def]", "abc", "\"];   \"", "", ""])

    str = "";
    split = _nested_split(str, False)
    assert(not split)

    str = "   ";
    split = _nested_split(str, False)
    assert(not split)

    str = "\"\"";
    split = _nested_split(str, False)
    assert(split == ["\"\""])

    str = ";  ";
    split = _nested_split(str, False)
    assert(split == ["",""])

    str = "a;b";
    split = _nested_split(str, False)
    assert(split == ["a","b"])

    str = ";";
    split = _nested_split(str, False)
    assert(split == ["",""])

###########################################################################################

def test_nested_split_failure():
    str = "]"
    with pytest.raises(ParseError):
        split = _nested_split(str, False)

###########################################################################################

def test_string():
    assert(avfromstring(str, "\"\"") == "")

    null_string = avfromtostring.NULL_STRING
    null_rep = avtostring(null_string)
    assert(null_rep == "\\0")
    assert(avfromstring(str, null_rep) is avfromtostring.NULL_STRING)

    string = '"'
    tostr = avtostring(string)
    assert(tostr == '"\\""')
    assert(avfromstring(str, tostr) == string)

    string = "   "
    assert(avfromstring(str, avtostring(string)) == string)

    string = ";"
    to_string = avtostring(string)
    assert(to_string == "\";\"")
    assert(avfromstring(str, to_string) == string)


    string = "\\"
    to_string = avtostring(string)
    assert(to_string == "\"\\\\\"")
    assert(avfromstring(str, to_string) == string)

###########################################################################################

def test_datetime():
    string = "2010-04-03T18:31:25.123"
    dt = avfromstring(datetime, string)
    assert(dt == datetime(2010, 4, 3, 18, 31, 25, 123000))
    assert(avtostring(dt) == string)

    string = avfromtostring.NULL_REP
    assert(avfromstring(datetime, string) == avfromtostring.NULL_DATETIME)
    assert(avtostring(avfromtostring.NULL_DATETIME) == avfromtostring.NULL_REP)

    dt = datetime(2010, 4, 3, 18, 31, 25, 123321)
    assert(avtostring(dt) == "2010-04-03T18:31:25.123")
    dt = datetime(2010, 4, 3, 18, 31, 25, 123999)
    assert(avtostring(dt) == "2010-04-03T18:31:25.123")
    dt = datetime(2010, 4, 3, 18, 31, 25, 0)
    assert(avtostring(dt) == "2010-04-03T18:31:25.000")
    dt = datetime(2010, 4, 3, 18, 31, 25, 1)
    assert(avtostring(dt) == "2010-04-03T18:31:25.000")
    dt = datetime(2010, 4, 3, 18, 31, 25, 1000)
    assert(avtostring(dt) == "2010-04-03T18:31:25.001")

###########################################################################################

def test_primitive_number_types():
    assert(avtostring(1) == "1")
    assert(avfromstring(type(1), "1") == 1)
    with pytest.raises(ParseError):
        avfromstring(type(1), "1.1")

    assert(avtostring(1.1) == "1.1")
    assert(avfromstring(type(1.1), "1.1") == 1.1)
    with pytest.raises(ParseError):
        avfromstring(type(1.1), "1q")

###########################################################################################

def test_dict():
    d = {1:2, "a":"\"b\"", "d":{"nested":"dict"}}
    assert(avtostring(d) == "[a:\"\\\"b\\\"\"; 1:2; d:[nested:dict]]")

###########################################################################################

def test_list():
    l = [1,2,[3,4]]
    assert(avtostring(l) == "[1; 2; [3; 4]]")

###########################################################################################

def test_dict():
    d_str = "[1:A; 2:B]"
    d = avfromstring(types.DictType, d_str)
    assert(d["1"] == "A")
    assert(d["2"] == "B")

    d_str = "[1:A; 1:B]"
    with pytest.raises(ParseError):
        d = avfromstring(types.DictType, d_str)


###########################################################################################

def test_stringlist():
    l = StringList()
    l.extend(["A", "B", "C"])
    stringrep = avtostring(l)
    assert(stringrep == "[A; B; C]")
    assert(avfromstring(StringList, stringrep) == l)

###########################################################################################

def test_intlist():
    l = IntList()
    l.extend([1,2,3])
    stringrep = avtostring(l)
    assert(stringrep == "[1; 2; 3]")
    assert(avfromstring(IntList, stringrep) == l)

###########################################################################################

def test_enum():

    class TestEnum(Enum):
        A = 1
        B = 2
    register_enum(TestEnum)

    a = TestEnum.A
    assert(avtostring(a) == "A")
    assert(avfromstring(TestEnum, "B") == TestEnum.B)

    with pytest.raises(ParseError):
        avfromstring(TestEnum, "C")

# end of file
