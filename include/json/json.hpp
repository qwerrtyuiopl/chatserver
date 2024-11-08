#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
namespace json
{
#define PARSEERROR(str, index)                                                                                   \
    do                                                                                                           \
    {                                                                                                            \
        char message[1024];                                                                                      \
        sprintf(message, "%s:An error occurred at location %d(%s_%d)", "PARSEERROR", index, __FILE__, __LINE__); \
        throw Exception(message);                                                                                \
    } while (0)
#define TRANSFORMERROR(t1, t2)                                                                                                                                \
    do                                                                                                                                                        \
    {                                                                                                                                                         \
        char message[1024];                                                                                                                                   \
        sprintf(message, "%s:%s cannot be converted to %s(%s_%d)", "TRANSFORMERROR", typeToString(t1).c_str(), typeToString(t2).c_str(), __FILE__, __LINE__); \
        throw Exception(message);                                                                                                                             \
    } while (0)
    enum error_type
    {
        ERROR_CONVERSION,
        ERROR_USAGE,
        ERROR_PARSE
    };
    enum value_type
    {
        VALUE_UNKNOW,
        VALUE_NULL,
        VALUE_BOOLEAN,
        VALUE_NUMBER,
        VALUE_STRING,
        VALUE_ARRAY,
        VALUE_OBJECT,
    };
    static std::string typeToString(value_type type)
    {
        switch (type)
        {
        case VALUE_NULL:
            return "VALUE_NULL";
        case VALUE_BOOLEAN:
            return "VALUE_BOOLEAN";
        case VALUE_NUMBER:
            return "VALUE_NUMBER";
        case VALUE_STRING:
            return "VALUE_STRING";
        case VALUE_ARRAY:
            return "VALUE_ARRAY";
        case VALUE_OBJECT:
            return "VALUE_OBJECT";
        default:
            // 6666
            break;
        }
        return "what?";
    }
    class Exception : public std::exception
    {
    public:
        Exception(std::string &&message) : _message(message)
        {
        }
        virtual const char *what() const noexcept override
        {
            return _message.c_str();
        }

    private:
        std::string _message;
    };
    class value
    {
    private:
        class value_value;
        using value_value_ptr = std::shared_ptr<value_value>;
        class value_value
        {
        public:
            value_type getType() const
            {
                return _type;
            }
            virtual std::string getString() const = 0;
            virtual std::string formatString() const = 0;

        protected:
            value_value(value_type type) : _type(type) {}
            const value_type _type;
        };
        class value_null : public value_value
        {
        public:
            value_null() : value_value(VALUE_NULL)
            {
            }
            std::string getString() const override
            {
                return "null";
            }
            virtual std::string formatString() const override
            {
                return "null";
            }
        };
        class value_boolean : public value_value
        {
        public:
            value_boolean(bool v) : value_value(VALUE_BOOLEAN), _value(v)
            {
            }
            std::string getString() const override
            {
                if (_value)
                    return "true";
                else
                    return "false";
            }
            std::string formatString() const override
            {
                if (_value)
                    return "true";
                else
                    return "false";
            }

        private:
            bool _value;
        };
        class value_number : public value_value
        {
        public:
            value_number(double v) : value_value(VALUE_NUMBER), isInt(false)
            {
                _value.doubleV = v;
            }
            value_number(long long int v) : value_value(VALUE_NUMBER), isInt(true)
            {
                _value.intV = v;
            }
            std::string getString() const override
            {
                return toString();
            }
            virtual std::string formatString() const override
            {
                return toString();
            }
            long long int toInt() const
            {
                if (isInt)
                    return _value.intV;
                return _value.doubleV;
            }
            double toDouble() const
            {
                if (isInt)
                    return _value.intV;
                return _value.doubleV;
            }

        private:
            std::string toString() const
            {
                std::string ret;
                if (isInt)
                {
                    long long int v = _value.intV;
                    do
                    {
                        ret += ((v % 10) + '0');
                        v /= 10;
                    } while (v);
                }
                else
                    ret = std::to_string(_value.doubleV);
                return ret;
            }
            union
            {
                long long int intV;
                double doubleV;
            } _value;
            bool isInt;
        };
        class value_string : public value_value
        {
        public:
            value_string(const std::string &v) : value_value(VALUE_STRING), _value(v)
            {
            }
            value_string(std::string &&v) : value_value(VALUE_STRING), _value(v)
            {
            }
            std::string getString() const override
            {
                return std::string("\"" + _value + "\"");
            }
            std::string formatString() const override
            {
                std::string ret("\"");
                decode_string(_value, ret);
                ret += "\"";
                return ret;
            }

        private:
            std::string _value;
        };
        class value_array : public value_value
        {
        public:
            value_array(std::vector<value> &&v) : value_value(VALUE_ARRAY)
            {
                _value.swap(v);
            }
            value_array(const std::vector<value> &v) : value_value(VALUE_ARRAY), _value(v)
            {
            }
            std::string getString() const override
            {
                std::string s("[");
                for (int i = 0; i < _value.size(); i++)
                {
                    s += _value[i].getString() + ',';
                }
                if (s.size() > 1)
                    s[s.size() - 1] = ']';
                return s;
            }
            std::string formatString() const override
            {
                std::string s("[");
                for (int i = 0; i < _value.size(); i++)
                {
                    s += _value[i].formatString() + ',';
                }
                if (s.size() > 1)
                    s[s.size() - 1] = ']';
                return s;
            }
            value &operator[](int index)
            {
                return _value.at(index); // 越界抛出异常
            }
            void push(const std::string &str)
            {
                int index = 0;
                if (_value.size() == 0)
                {
                    _value.push_back(value(parse_value(str, index)));
                }
                else
                {
                    switch (_value[0].getType())
                    {
                    case VALUE_NULL:
                        _value.push_back(value(parse_null(str, index)));
                        break;
                    case VALUE_BOOLEAN:
                        _value.push_back(value(parse_boolean(str, index)));
                        break;
                    case VALUE_NUMBER:
                        _value.push_back(value(parse_number(str, index)));
                        break;
                    case VALUE_STRING:
                        _value.push_back(value(parse_string(str, index)));
                        break;
                    case VALUE_ARRAY:
                        _value.push_back(value(parse_array(str, index)));
                        break;
                    case VALUE_OBJECT:
                        _value.push_back(value(parse_object(str, index)));
                        break;
                    default:
                        // what?
                        break;
                    }
                }
            }

        private:
            std::vector<value> _value;
        };
        class value_object : public value_value
        {
        public:
            value_object(const std::unordered_map<std::string, value> &v) : value_value(VALUE_OBJECT), _value(v)
            {
            }
            value_object(std::unordered_map<std::string, value> &&v) : value_value(VALUE_OBJECT)
            {
                _value.swap(v);
            }
            std::string getString() const override
            {
                std::string s = (std::string) "{" + " ";
                for (auto it = _value.begin(); it != _value.end(); it++)
                {
                    if (it != _value.begin())
                        s += ',';
                    s += ("\"" + it->first + "\"" + " : " + it->second.getString());
                }
                s += "}";
                return s;
            }
            std::string formatString() const override
            {
                std::string s = (std::string) "{" + " ";
                for (auto it = _value.begin(); it != _value.end(); it++)
                {
                    std::string name;
                    decode_string(it->first, name);
                    if (it != _value.begin())
                        s += ',';
                    s += ("\"" + name + "\"" + " : " + it->second.formatString());
                }
                s += "}";
                return s;
            }
            value &operator[](const std::string &str)
            {
                if (_value.find(str) == _value.end())
                {
                    _value[str] = value();
                }
                return _value[str];
            }

        private:
            std::unordered_map<std::string, value> _value;
        };

    public:
        value() : _value(value_value_ptr(new value_null))
        {
        }
        value(value_value_ptr &&v) : _value(std::move(v))
        {
        }
        value(value_value_ptr &v) : _value(std::move(v))
        {
        }
        value(const value& v){
            _value=v._value;
        }
        std::string getString() const
        {
            return _value->getString();
        }
        std::string formatString() const
        {
            return _value->formatString();
        }
        value_type getType() const
        {
            return _value->getType();
        }
        void reSet(const value_value_ptr &v)
        {
            _value = v;
        }
        long long int toInt()
        {
            switch (_value->getType())
            {
            case VALUE_NUMBER:
                return (dynamic_cast<value_number *>(_value.get())->toInt());
            default:
                TRANSFORMERROR(_value->getType(), VALUE_NUMBER);
                break;
            }
        }
        double toDouble()
        {
            switch (_value->getType())
            {
            case VALUE_NUMBER:
                return (dynamic_cast<value_number *>(_value.get())->toDouble());
            default:
                TRANSFORMERROR(_value->getType(), VALUE_NUMBER);
                break;
            }
        }
        std::string toString()
        {
            return _value->getString();
        }
        value &operator[](const std::string &str)
        {
            switch (_value->getType())
            {
            case VALUE_OBJECT:
                break;
            default:
                this->reSet(parse_json("{}"));
                break;
            }
            return (*dynamic_cast<value_object *>(_value.get()))[str];
        }
        value &operator[](int index)
        {
            int s = 0;
            switch (_value->getType())
            {
            case VALUE_ARRAY:
                break;
            default:
                this->reSet(parse_array("[]", s));
                break;
            }
            return (*dynamic_cast<value_array *>(_value.get()))[index];
        }
        /*
         * 会自动类型转换
         */
        void operator=(int v)
        {
            _value = value_value_ptr(new value_number((long long int)v));
        }
        void operator=(long long int v)
        {
            _value = value_value_ptr(new value_number(v));
        }
        void operator=(double v)
        {
            _value = value_value_ptr(new value_number(v));
        }
        /*
         * 会自动类型转换
         */
        void operator=(const std::string &v)
        {
            int index = 0;
            _value = value_value_ptr(parse_value(v, index));
            if (index != v.size())
            {
                PARSEERROR(v, 0);
            }
        }
        /*
         * 仅限array值类型使用否则会发生类型转换
         * 应传入适合的数据
         */
        void push(const std::string &str)
        {
            int index = 0;
            switch (_value->getType())
            {
            case VALUE_ARRAY:
                break;
            default:
                this->reSet(parse_array("[]", index));
                break;
            }
            dynamic_cast<value_array *>(_value.get())->push(str);
        }

    private:
        static void whitespace(const std::string &s, int &index)
        {
            while (s[index] == ' ' || s[index] == '\t' || s[index] == '\n' || s[index] == '\r')
                ++index;
        }
        static bool parse_hex4(const std::string &s, int &index, unsigned &u)
        {
            int i;
            u = 0;
            for (i = 0; i < 4; i++)
            {
                char ch = s[index++];
                u <<= 4;
                if (ch >= '0' && ch <= '9')
                    u |= ch - '0';
                else if (ch >= 'A' && ch <= 'F')
                    u |= ch - ('A' - 10);
                else if (ch >= 'a' && ch <= 'f')
                    u |= ch - ('a' - 10);
                else
                    return false;
            }
            return true;
        }
        static void encode_utf8(std::string &s, unsigned u)
        {
            if (u <= 0x7F)
                s += (u & 0xFF);
            else if (u <= 0x7FF)
            {
                s += (0xC0 | ((u >> 6) & 0xFF));
                s += (0x80 | (u & 0x3F));
            }
            else if (u <= 0xFFFF)
            {
                s += (0xE0 | ((u >> 12) & 0xFF));
                s += (0x80 | ((u >> 6) & 0x3F));
                s += (0x80 | (u & 0x3F));
            }
            else
            {
                s += (0xF0 | ((u >> 18) & 0xFF));
                s += (0x80 | ((u >> 12) & 0x3F));
                s += (0x80 | ((u >> 6) & 0x3F));
                s += (0x80 | (u & 0x3F));
            }
        }
        static void decode_string(const std::string &str, std::string &ret)
        {
            static const char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
            for (int i = 0; i < str.size(); ++i)
            {
                unsigned char ch = (unsigned char)str[i];
                switch (ch)
                {
                case '\"':
                    ret += '\\';
                    ret += '\"';
                    break;
                case '\\':
                    ret += '\\';
                    ret += '\\';
                    break;
                case '\b':
                    ret += '\\';
                    ret += 'b';
                    break;
                case '\f':
                    ret += '\\';
                    ret += 'f';
                    break;
                case '\n':
                    ret += '\\';
                    ret += 'n';
                    break;
                case '\r':
                    ret += '\\';
                    ret += 'r';
                    break;
                case '\t':
                    ret += '\\';
                    ret += 't';
                    break;
                default:
                    if (ch < 0x20)
                    {
                        ret += '\\';
                        ret += 'u';
                        ret += '0';
                        ret += '0';
                        ret += hex_digits[ch >> 4];
                        ret += hex_digits[ch & 15];
                    }
                    else
                        ret += str[i];
                }
            }
        }
        static bool parse_text(const std::string &s, int &index, std::string &ret)
        {
            unsigned u1, u2;
            for (;;)
            {
                char ch = s[index++];
                switch (ch)
                {
                case '"':
                    return true;
                case '\\':
                    switch (s[index++])
                    {
                    case '\"':
                        ret += '\"';
                        break;
                    case '\\':
                        ret += '\\';
                        break;
                    case '/':
                        ret += '/';
                        break;
                    case 'b':
                        ret += '\b';
                        break;
                    case 'f':
                        ret += '\f';
                        break;
                    case 'n':
                        ret += '\n';
                        break;
                    case 'r':
                        ret += '\r';
                        break;
                    case 't':
                        ret += '\t';
                        break;
                    case 'u':
                        if (!parse_hex4(s, index, u1))
                        {
                            index -= 4;
                            PARSEERROR(s, index);
                        }

                        if (u1 >= 0xD800 && u1 <= 0xDBFF)
                        { /* surrogate pair */
                            if (s[index++] != '\\')
                            {
                                index--;
                                PARSEERROR(s, index);
                            }
                            if (s[index++] != 'u')
                            {
                                index--;
                                PARSEERROR(s, index);
                            }
                            if (!parse_hex4(s, index, u2))
                            {
                                index--;
                                PARSEERROR(s, index);
                            }
                            if (u2 < 0xDC00 || u2 > 0xDFFF)
                            {
                                index--;
                                PARSEERROR(s, index);
                            }
                            u1 = (((u1 - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        encode_utf8(ret, u1);
                        break;
                    default:
                    {
                        index--;
                        PARSEERROR(s, index);
                    }
                    }
                    break;
                case '\0':
                {
                    index--;
                    PARSEERROR(s, index);
                }
                default:
                    if ((unsigned char)ch < 0x20)
                    {
                        index--;
                        PARSEERROR(s, index);
                    }
                    ret += ch;
                }
            }
        }
        static bool parse_name(const std::string &s, int &index, std::string &name)
        {
            whitespace(s, index);
            bool ret = false;
            if (s[index] == '"')
            {
                index++;
                ret = parse_text(s, index, name);
            }
            whitespace(s, index);
            return ret;
        }

        static value_value_ptr parse_null(const std::string &s, int &index)
        {
            whitespace(s, index);
            static const std::string str = "null";
            for (int i = 0; i < str.size(); ++i)
            {
                if (s[index] == '\0' || str[i] != s[index + i])
                {
                    PARSEERROR(s, index + i);
                }
            }
            index += 4;
            whitespace(s, index);
            return value_value_ptr(new value_null());
        }
        static value_value_ptr parse_boolean(const std::string &s, int &index)
        {
            whitespace(s, index);
            static const std::string str1 = "true";
            static const std::string str2 = "false";
            if ((s.size() - index >= 4) && s.substr(index, 4) == str1)
            {
                index += 4;
                whitespace(s, index);
                return value_value_ptr(new value_boolean(true));
            }
            else if ((s.size() - index >= 5) && s.substr(index, 5) == str2)
            {
                index += 5;
                whitespace(s, index);
                return value_value_ptr(new value_boolean(false));
            }
            else
                PARSEERROR(s, index);
            whitespace(s, index);
        }
        static value_value_ptr parse_number(const std::string &s, int &index)
        {
            char a;
            bool isInt = true;
            char *e = &a;
            char **endptr = &e;
            double v = strtod(&s[index], endptr);
            if (*endptr == &s[index])
            {
                PARSEERROR(s, index);
            }
            for (; &s[index] != *endptr; index++)
            {
                if (s[index] == '.')
                    isInt = false;
            }
            if (isInt)
            {
                return value_value_ptr(new value_number((long long int)v));
            }
            return value_value_ptr(new value_number(v));
        }
        static value_value_ptr parse_string(const std::string &s, int &index)
        {
            whitespace(s, index);
            if (s[index] != '"')
            {
                PARSEERROR(s, index);
            }
            index++;
            std::string v;
            parse_text(s, index, v);
            whitespace(s, index);
            return value_value_ptr(new value_string(std::move(v)));
        }
        static value_value_ptr parse_array(const std::string &s, int &index)
        {
            whitespace(s, index);
            value_type type = VALUE_UNKNOW;
            bool isCycle = true;
            std::vector<value> v;
            if (s[index] != '[')
            {
                PARSEERROR(s, index);
            }
            index++;
            whitespace(s, index);
            if (s[index] == ']')
            {
                isCycle = false; // 空
            }
            while (isCycle)
            {
                if (type != VALUE_UNKNOW)
                {
                    if (s[index] != ',')
                    {
                        PARSEERROR(s, index);
                    }
                    index++;
                }
                int last = index;
                v.push_back(value(parse_value(s, index)));
                type = type == VALUE_UNKNOW ? v.back().getType() : type;
                if (v.back().getType() != type)
                {
                    index = last;
                    PARSEERROR(s, index);
                }
                if (s[index] == ']')
                {
                    index++;
                    isCycle = false;
                }
            }
            whitespace(s, index);
            return value_value_ptr(new value_array(std::move(v)));
        }
        /*
         * 解析object({})类型,并指向下一个类型的起始地址,如果此类型为json则指向jsonStr的最后一个字符加1.
         */
        static value_value_ptr parse_object(const std::string &s, int &index)
        {
            whitespace(s, index);
            bool isStart = true;
            std::unordered_map<std::string, value> v;
            std::string name;
            if (s[index] != '{')
            {
                PARSEERROR(s, index);
            }
            index++;
            while (1)
            {
                bool is = false;
                if (!isStart && s[index] != '}' && ((is = true) && s[index++] != ','))
                {
                    index--;
                    PARSEERROR(s, index);
                }
                isStart = false;
                name.clear();
                if (!parse_name(s, index, name))
                {
                    if (s[index++] == '}' && !is) // 解析成功
                    {
                        whitespace(s, index);
                        break;
                    }
                    else
                        PARSEERROR(s, index);
                }
                if (s[index++] != ':')
                    PARSEERROR(s, index);
                v[name] = value(parse_value(s, index));
            }
            whitespace(s, index);
            return value_value_ptr(new value_object(std::move(v)));
        }
        /*
         */
        static value_value_ptr parse_value(const std::string &s, int &index)
        {
            whitespace(s, index);
            switch (s[index])
            {
            case '"':
                return parse_string(s, index);
                break;
            case 'n':
                return parse_null(s, index);
                break;
            case 'f':
            case 't':
                return parse_boolean(s, index);
                break;
            case '[':
                return parse_array(s, index);
                break;
            case '{':
                return parse_object(s, index);
                break;
            default:
                return parse_number(s, index);
                break;
            }
            whitespace(s, index);
        }
        static value_value_ptr parse_json(const std::string &s, int *index = NULL)
        {
            int l = 0;
            if (index == NULL)
            {
                index = &l;
            }
            value_value_ptr ret = parse_object(s, *index);
            if (*index != s.size())
            {
                PARSEERROR(s, *index);
            }
            return ret;
        }
        friend class json;

    private:
        value_value_ptr _value;
    };
    class json
    {
    public:
        json(const std::string &jsonStr) : _value(value::parse_json(jsonStr))
        {
        }
        json() : _value(value::parse_json("{}"))
        {
        }
        std::string toString()
        {
            return _value.getString();
        }
        std::string formatString()
        {
            return _value.formatString();
        }
        void operator=(const std::string &jsonStr)
        {
            _value.reSet(value::parse_json(jsonStr));
        }
        /*
         * 会自动类型转换
         */
        value &operator[](const std::string &str)
        {
            return _value[str];
        }

    private:
        value _value;
    };
}
