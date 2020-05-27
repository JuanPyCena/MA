# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------------
# Copyright: AviBit GmbH, 2001-2015
# ----------------------------------------------------------------------------

"""
.. moduleauthor:: Christian Muschick <c.muschick@avibit.com>

  *Attention*: Note that the pattern of type switching employed for the avfromstring /
               avtostring mechanism is frowned upon, and was employed here mostly to mimic
               behaviour as currently in C++.
               Think twice before doing this in new code.
"""

# Python 3 compatibility
from __future__ import print_function, unicode_literals, absolute_import, division
from future.builtins import *

from future.types import newdict, newlist, newstr, newint

import datetime
import dateutil.parser

import types

from enum import Enum

# If touching any of those, also update SPECIAL_CHAR_REGEXP!
# Also keep this in sync with the C++ implementation!
DELIMITER_START = "[";
DELIMITER_END   = "]";
QUOTE           = "\"";
ESCAPE          = "\\";
SEPARATOR       = ";";
MAPPING         = ":";
NEWLINE         = "\n";
# Not in SPECIAL_CHAR_LIST as this is only used for QSize(s).
CROSS           = "x";

# Strings are quoted if they contain any of the following characters.
# Keep in sync with the constants above!
# Also keep this in sync with the python implementation!
SPECIAL_CHAR_LIST = [DELIMITER_START, DELIMITER_END, QUOTE, ESCAPE, SEPARATOR, MAPPING, NEWLINE]

# used for string and datetime null representation
NULL_REP = "\\0"

###########################################################################################

class ParseError(Exception) :
    pass

###########################################################################################

class __NullStringClass(object):
    def __str__(self):
        return NULL_REP

# This object can be returned as a special value when converting AviBit strings because
# python has no notion of null strings.
#
# Note that the avmsglib has its own concept of nullity, and this object is not used there.
# See AVMsgFundamentalType::is_null().
NULL_STRING = __NullStringClass()

class __NullDateTimeClass(object):
    def __str__(self):
        return NULL_REP

# This object can be returned as a special value when converting AviBit datetimes because
# python has no notion of null datetimes.
#
# Note that the avmsglib has its own concept of nullity, and this object is not used there.
# See AVMsgFundamentalType::is_null().
NULL_DATETIME = __NullDateTimeClass()

###########################################################################################

# The reason we need typed lists is avfromstring functionality - if it isn't typed, we can just put strings into the
# converted list.

# TODO CM those lists are not strictly typed, change that?
# TODO CM allow nesting of typed lists? probably not worth it for now...
# TOOD CM is the StringList type really useful?
class StringList(types.ListType):
    pass
class IntList(types.ListType):
    pass

###########################################################################################

def avfromstring(type, string):
    """
    Converts the AviBit string representation to the given data type (provided a handler
    for this type has been registered via register_avfromtostring_handlers).

    Parameters
    ----------
    type : type
        The type to convert to, e.g. datetime.datetime
    string : string
        The string to convert.

    Returns
    -------
    Almost always an instance of the given type (there are some exceptions e.g. for null strings and datetimes)

    Raises
    ------
    ParseError
        Raised if the conversion failed.
    Exception
        Raised if the datatype does not support fromstring conversion.
    """
    (from_handler, to_handler) = (None, None)
    try:
        (from_handler, to_handler) = __avfromtostring_handlers[type]
    except KeyError:
        pass
    # there are types which only support tostring conversion.
    if from_handler is None:
        raise Exception("avfromstring not implemented for " + type.__name__)

    # now we have a handler, but parsing can still fail...
    try:
        return from_handler(string)
    except ValueError as v:
        raise ParseError(str(v) + " - " + string + " of type " + type.__name__)


###########################################################################################

def avtostring(data, quote_specials = False):
    """
    Converts the given object to its AviBit string representation.

    Also traverses base types when searching for a tostring handler.

    Parameters
    ----------
    data : any type for which a tostring conversion handler has been registered
        The object to convert.
    quote_specials : bool
        If true, implementations must either produce a string which can be correctly parsed by
        _nested_split, or quote their string rep if any character from SPECIAL_CHAR_LIST is
        contained.

    Returns
    -------
    The AviBit string representation of the object.

    Raises
    ------
    Exception
        Raised if the datatype does not support tostring conversion.
    """

    types = [type(data)]
    types.extend(type(data).__bases__)

    for t in types:
        try:
            (fh, th) = __avfromtostring_handlers[t]
        except KeyError:
            continue

        return th(data, quote_specials)

    raise Exception("avtostring not implemented for " + type(data).__name__)

###########################################################################################

def register_avfromtostring_handlers(datatype, fromstring_handler, tostring_handler):
    """
    Defines how a data type is converted from/to its AviBit string representation.

    Parameters
    ----------
    datatype : type
        The type for which to define the conversion
    fromstring_handler : function
        The function converting the data type from a string. Can be None if the conversion is not supported.
    tostring_handler : function
        The function converting the data type to a string. Can be None if the conversion is not supported.

    Raises
    ------
    Exception
       Raised if the conversion was already defined.
    """
    if datatype in __avfromtostring_handlers.keys():
        raise Exception("avfromtostring conversion for " + str(datatype) + " was defined more than once")
    __avfromtostring_handlers[datatype] = (fromstring_handler, tostring_handler)

###########################################################################################

def _nested_split(input, strip_outmost_delimiters, separator=SEPARATOR):
    """
    This method splits the input string along separator tokens, but ignoring separators in nested
    delimited or escaped regions. The outmost enclosing delimiters are optionally stripped.
    Note that leading and trailing whitespace is removed from the elements in the result list.

    Passing an empty string will return an empty list. Passing a single separator token will result
    in two empty strings in the list.

    Keep this method implementation in sync with C++.

    Parameters
    ----------
    input : string
        The string to process.
    strip_outmost_delimiters : bool
        Whether to remove the outmost delimiter pair before processing
        the input string. If this is true, leading and trailing whitespace
        is removed from the input string as well.
    separator : string, optional
        The separator along which to split, usually ";"

    Returns
    -------
    list of strings
        The split elements of the input string

    Raises
    ------
    ParseError
        Raised if the input cannot be split (e.g. mismatched delimiters, or no outmost
        delimiters although they should be stripped)
    """

    original_input = input

    if strip_outmost_delimiters:
        input = input.strip()
        if not input.startswith(DELIMITER_START) or not input.endswith(DELIMITER_END):
            raise ParseError("Missing start/end delimiter in " + input)
        input = input[1:-1]

    if not input.strip():
        return []

    quoted      = False; # whether we currently are in a quoted part of the input string
    prev_escape = False; # whether the previously read character was the escpae character
    nest_count  = 0;
    start       = 0;     # The start position of the currently read list element
    pos         = 0;

    ret = []
    while len(input) > pos:
        cur_char = input[pos]
        if quoted:
            if cur_char == QUOTE:
                if not prev_escape:
                    quoted = False
                prev_escape = False
            elif cur_char == ESCAPE:
                prev_escape = not prev_escape
            else:
                prev_escape = False
        else:
            if cur_char == DELIMITER_START:
                nest_count += 1
            elif cur_char == DELIMITER_END:
                if nest_count == 0:
                    raise ParseError("Incorrectly nested delimiters in " + original_input)
                nest_count -= 1
            elif cur_char == QUOTE:
                quoted = True
            elif nest_count == 0 and cur_char == separator:
                ret.append(input[start:pos].strip())
                start = pos+1
        pos += 1;

    # last character is separator, thus add an empty string
    # if str == separator there will be two empty strings
    if input and input[-1] == separator:
        ret.append("")

    if pos > start:
        ret.append(input[start:pos].strip())

    if nest_count != 0 or quoted:
        raise ParseError("Incorrectly nested delimiters/quotation in " + original_input)

    return ret

###########################################################################################

def _escape(string, esc_from, esc_to):
    """
    Escapes and unescapes special strings by the provided escape sequences.

    Replaces all strings from esc_from which are contained in str by the corresponding
    string from esc_to. esc_from and esc_to must have the same size.
    Use with exchanged "esc_from" and "esc_to" parameters to unescape strings.

    It is the responsibility of the user to avoid ambiguous calls, such as
    overlapping esc_froms in the input string.
    """
    assert(len(esc_from) == len(esc_to))
    from_lengths = [len(f) for f in esc_from]

    # collect string chunks and join them later for efficiency
    ret = []

    cur_position = 0
    while True:

        # Find first occurences of all strings to be replaced
        indices = [string.find(f, cur_position) for f in esc_from]
        # ... and discard the ones which had no match. Group them with data we need for replacing.
        data = [d for d in zip(indices, from_lengths, esc_to) if d[0] != -1]

        if not data:
            # nothing more to replace, just append the remaining string
            ret.append(string[cur_position:])
            break;

        # find the first match and replace it
        data = sorted(data, key = lambda d:d[0])

        replace_index       = data[0][0]
        replace_from_length = data[0][1]
        replace_to          = data[0][2]

        ret.append(string[cur_position:replace_index])
        ret.append(replace_to)

        cur_position = replace_index + replace_from_length

    return "".join(ret)

###########################################################################################

def _string_to_string(string, quote_specials):
    quote = False
    if string.strip() != string: # TODO CM this is different from C++. Why even strip in _string_from_string?
        quote = True
    else:
        for sc in SPECIAL_CHAR_LIST:
            if sc in string: quote = True

    if not quote: return string
    return QUOTE + _escape(string, [QUOTE, ESCAPE], [ESCAPE+QUOTE, ESCAPE+ESCAPE]) + QUOTE

###########################################################################################

def _string_from_string(string):
    if string == NULL_REP:
        return NULL_STRING

    ret = string.strip()

    # If the string was quoted, we need to unquote and unescape it.
    if ret.startswith(QUOTE):
        if not ret.endswith(QUOTE):
            raise ParseError(string + " is not correctly quoted")
        ret = ret[1:-1]
        ret = _escape(ret, [ESCAPE+QUOTE, ESCAPE+ESCAPE], [QUOTE, ESCAPE])

    return ret


###########################################################################################

def _datetime_to_string(dt, quote_specials):
    ret = dt.isoformat() # "%Y-%m-%dT%H:%M:%S.%f"

    # if there are any microseconds, drop the last 3 digits.
    if dt.microsecond != 0:
        ret = ret[:-3]

    # if we have no milliseconds, add (.000) milliseconds (If there are no microseconds
    # dt.isoformat() returns "%Y-%m-%dT%H:%M:%S)
    if dt.microsecond == 0:
        ret += ".000"

    if quote_specials:
        ret = QUOTE + ret + QUOTE
    return ret

###########################################################################################

def _datetime_from_string(string):
    if string == NULL_REP:
        return NULL_DATETIME

    if string.startswith(QUOTE):
        if not string.endswith(QUOTE):
            raise ParseError(string + " is not correctly quoted")
        string = string[1:-1]

    # TODO: avoid dateutil dependency?
    return dateutil.parser.parse(string)

###########################################################################################

def _dict_to_string(d):
    ret = []
    for key in sorted(d.iterkeys()):
        ret.append(avtostring(key, True) + ":" + avtostring(d[key], True))
    return "[" + "; ".join(ret) + "]"

###########################################################################################

def _dict_from_string(dict_type, key_type, value_type, string):
    ret = dict_type()
    key_value_pairs = _nested_split(string, True)
    for kv in key_value_pairs:
        key_str,value_str = _nested_split(kv, False, MAPPING)
        key = avfromstring(key_type, key_str)
        if key in ret.keys():
            raise ParseError("Key " + key + " is present twice in " + string)
        ret[key] = avfromstring(value_type, value_str)
    return ret

###########################################################################################

def _list_to_string(l):
    ret = []
    for item in l:
        ret.append(avtostring(item, True))
    return "[" + "; ".join(ret) + "]"

###########################################################################################

def _list_from_string(list_type, element_type, string):
    ret = list_type()
    elements = _nested_split(string, True)
    for cur_element in elements:
        ret.append(avfromstring(element_type, cur_element))
    return ret

###########################################################################################

def _bool_from_string(string):
    if string.lower() == "true":
        return True
    return False

###########################################################################################

def _bool_to_string(value, quote_specials):
    if value:
        return "true"
    return "false"

###########################################################################################

def _enum_to_string(value, quote_specials):
    """
    Also see register_enum, which allows full from/to string support for enums.
    """
    return value.name

###########################################################################################

def register_enum(enum_class):
    """
    Creates and registers functions to convert a given enum class. Note that to string conversion of any enum works
    out of th box, see _enum_to_string.

    Parameters
    ----------
    enum_class : The class for which to create from/tostring conversion methods
    """
    def enum_from_string(enum_class, string_value):
        try:
            return enum_class[string_value]
        except KeyError:
            raise ParseError(string_value + " is not a valid enum value")
    register_avfromtostring_handlers(enum_class,    lambda s: enum_from_string(enum_class, s), lambda l,q: _enum_to_string(l,q))

###########################################################################################

# *Attention* Please read the disclaimer at the start of the module before basing your
#             code on this pattern.
__avfromtostring_handlers = dict()


# don't force usage of new types - register conversion for both old and new types...

register_avfromtostring_handlers(unicode, _string_from_string, _string_to_string)
register_avfromtostring_handlers(newstr, _string_from_string, _string_to_string)

register_avfromtostring_handlers(__NullStringClass, None, lambda s, q: NULL_REP)

register_avfromtostring_handlers(datetime.datetime, _datetime_from_string, _datetime_to_string)
register_avfromtostring_handlers(__NullDateTimeClass, None, lambda s, q: NULL_REP)

register_avfromtostring_handlers(types.IntType,   lambda s:types.IntType(s),   lambda i, q:str(i))
register_avfromtostring_handlers(newint,          lambda s:newint(s),   lambda i, q:str(i))
register_avfromtostring_handlers(types.FloatType, lambda s:types.FloatType(s), lambda f, q:str(f))
register_avfromtostring_handlers(bool, lambda s:_bool_from_string(s), lambda b, q:_bool_to_string(b, q))

# if the dict type is not specified, simply treat it as a string dict.
register_avfromtostring_handlers(types.DictType, lambda s:_dict_from_string(types.DictType, unicode, unicode, s), lambda d,q:_dict_to_string(d))
register_avfromtostring_handlers(newdict,        lambda s:_dict_from_string(newdict,        unicode, unicode, s), lambda d,q:_dict_to_string(d))
# if the list type is not specified, simply treat it as a string list.
register_avfromtostring_handlers(types.ListType, lambda s:_list_from_string(types.ListType, unicode, s), lambda l,q:_list_to_string(l))
register_avfromtostring_handlers(newlist,        lambda s:_list_from_string(newlist,        unicode, s), lambda l,q:_list_to_string(l))

register_avfromtostring_handlers(StringList, lambda s:_list_from_string(StringList, unicode, s), lambda l,q:_list_to_string(l))
register_avfromtostring_handlers(IntList,    lambda s:_list_from_string(IntList, types.IntType, s), lambda l,q:_list_to_string(l))

# use register_enum to have fromstring support for user-defined enum types.
register_avfromtostring_handlers(Enum, None, _enum_to_string)

# end of file
