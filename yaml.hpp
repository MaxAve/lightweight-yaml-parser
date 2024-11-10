/*
  MIT License

  Copyright (c) 2024 MaxAve

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

// Version 1.0.0
//
// Warning: this is an early version that does not support certain aspects of YAML!
// Some features that currently have no implementation include:
// 1-line objetcs e.g. 'value: {a: 1, b: 2, c: 3}'
// Anchors
// Object lists (with -)
// Mulit-line strings

#include <iostream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <array>
#include <exception>
#include <stack>

namespace yaml
{
enum YAMLType
{
	None_,
	Int_,
	Double_,
	String_,
	Array_,
	Bool_,
	Object_,
};

std::string yaml_type_to_str(YAMLType type)
{
	switch(type)
	{
	case YAMLType::Int_:
		return "int"; // int
	case YAMLType::Double_:
		return "double"; // double
	case YAMLType::String_:
		return "string"; // std::string
	case YAMLType::Array_:
		return "array"; // yaml::Array
	case YAMLType::Bool_:
		return "bool"; // bool
	case YAMLType::Object_:
		return "object"; // yaml::Object
	}
	return "None";
}

class TypeMismatchException : public std::exception
{
private:
    	std::string message;

public:
	TypeMismatchException(const char* msg) : message(msg) {}

    	const char* what() const throw()
    	{
		return message.c_str();
    	}
};

typedef struct
{
	std::string name;
	YAMLType type;
	void* value;
} NamedValue; // Pointer to an arbitrary type with a variable name

class TypedValue
{
public:
	YAMLType type;
	void *value;
	
	TypedValue()
	{
		type = YAMLType::None_;
		value = nullptr;
	}

	template<typename T>
	T cast()
	{
		// Type checking
		switch(type)
		{
		case YAMLType::Int_:
			if(!std::is_same<T, int>::value && !std::is_same<T, double>::value)
				throw TypeMismatchException("Attempting to cast Integer value to an incompatible type");
			break;	
		case YAMLType::Double_:
			if(!std::is_same<T, double>::value)
				throw TypeMismatchException("Attempting to cast Double value to an incompatible type. If you are trying to cast to a float, cast to a Double first. Example: static_cast<float>(yaml_file[\"data\"].cast<double>())");
			break;
		case YAMLType::String_:
			if(!std::is_same<T, std::string>::value)
				throw TypeMismatchException("Attempting to cast String value to an incompatible type");
			break;
		case YAMLType::Array_:
			if(!std::is_same<T, std::vector<TypedValue>>::value)
				throw TypeMismatchException("Attempting to cast Array value to an incompatible type");
			break;
		case YAMLType::Bool_:
			if(!std::is_same<T, bool>::value)
				throw TypeMismatchException("Attempting to cast Bool value to an incompatible type");
			break;
		case YAMLType::Object_:
			if(!std::is_same<T, std::unordered_map<std::string, TypedValue>>::value)
				throw TypeMismatchException("Attempting to cast Object value to an incompatible type");
			break;
		}

		return *(static_cast<T*>(value));
	}

	size_t size()
	{
		switch(type)
		{
		case YAMLType::Array_:
			return static_cast<std::vector<TypedValue>*>(value)->size();
		case YAMLType::Object_:
			return static_cast<std::unordered_map<std::string, TypedValue>*>(value)->size();
		}
		std::cerr << "ERROR: Unable to get length of type " << yaml_type_to_str(type) << ". This operation is only applicable to array and object.\n";
		return -1;
	}

	TypedValue operator[](size_t index)
	{
		if(type != YAMLType::Array_)
		{
			std::cerr << "ERROR: Attempting to cast TypedValue of type " << yaml_type_to_str(type) << " to array.";
			return TypedValue();
		}
		return static_cast<std::vector<TypedValue>*>(value)->at(index);
	}

	TypedValue operator[](std::string name)
	{
		if(type != YAMLType::Object_)
		{
			std::cerr << "ERROR: Attempting to cast TypedValue of type " << yaml_type_to_str(type) << " to object.";
			return TypedValue();
		}
		return static_cast<std::unordered_map<std::string, TypedValue>*>(value)->at(name);
	}
}; // Pointer to an arbitrary type with a type enum includedi

typedef std::vector<TypedValue> YAML_array;
typedef std::unordered_map<std::string, TypedValue> YAML_map;

typedef std::string String;
typedef YAML_array  Array;
typedef YAML_map    Object;

// Counts the number of spaces set at the beginning of a string
int count_spaces(const std::string &str, int tab_spaces=4)
{
	int spaces = 0;
	for(int i = 0; i < str.length(); i++)
	{
		if(str[i] == ' ')
			spaces++;
		else if(str[i] == '\t')
			spaces += tab_spaces;
		else
			return spaces;
	}
	return spaces;
}

// Removes tabs and spaces at the beginning and end of the string
// E.g. "   name: person " -> "name: person"
std::string truncate_spaces(const std::string &str)
{
	std::string new_str = "";

	int trailing_spaces = 0;

	for(int i = str.length() - 1; i >= 0; i--)
	{
		if(new_str.length() == 0 && (str[i] == ' ' || str[i] == '\t'))
		{
			trailing_spaces++;
			continue;
		}
		break;
	}

	for(int i = 0; i < str.length() - trailing_spaces; i++)
	{
		if(new_str.length() == 0 && (str[i] == ' ' || str[i] == '\t'))
			continue;
		new_str += str[i];
	}

	return new_str;
}

// Removes excess spaces and tabs that come after a line
std::string truncate_spaces_after(const std::string &str)
{
	int trailing_spaces = 0;

	for(int i = str.length() - 1; i >= 0; i--)
	{
		if(str.length() == 0 && (str[i] == ' ' || str[i] == '\t'))
		{
			trailing_spaces++;
			continue;
		}
		break;
	}

	return str.substr(0, str.length() - trailing_spaces);
}

// Removes all tabs and spaces that come after a specified char
// E.g. "name:    person" -> "name:person"
std::string remove_spaces_after_char(const std::string &str, char c)
{
	std::string new_str = "";
	char last_char = '\0';
	for(int i = new_str.length(); i < str.length(); i++)
	{
		if(str[i] != ' ')
			last_char = str[i];
		if(last_char == c && str[i] == ' ')
			continue;
		new_str += str[i];
	}
	return new_str;
}

// Returns true if the YAML line is a variable-value pair, otherwise returns false
bool is_variable_value_pair(const std::string &str)
{
	return (str.find(':') != (str.length() - 1));
}

// Remove a character completely from the given string
std::string filter(const std::string &str, char c)
{
	std::string new_str;
	for(int i = 0; i < str.length(); i++)
	{
		if(str[i] == c)
			continue;
		new_str += str[i];
	}
	return new_str;
}

// Splits a string into the name and value based on the : delimiter
// Example: "variable_name:value" -> {"variable_name", "value"}
std::array<std::string, 2> split_variable(const std::string &str)
{
	std::string name, value;
	int delimiter_pos = str.find(':');
	name = str.substr(0, delimiter_pos);
	value = str.substr(delimiter_pos + 1, str.length());
	std::array<std::string, 2> name_value = {name, value};
	return name_value;
}

// Get the data type of the YAML value
YAMLType get_type(const std::string &str)
{
	if(str.length() == 0)
		return YAMLType::None_;

	// Boolean (YAML 1.2.2 only)
	if(str == std::string("true") || str == std::string("false"))
		return YAMLType::Bool_;

	// Array
	if(str[0] == '[' && str[str.length() - 1] == ']')
		return YAMLType::Array_;
	
	// Int
	bool is_int = true;
	for(int i = 0; i < str.length(); i++)
	{
		if((str[i] < '0' || str[i] > '9') && str[i] != '-')
		{
			is_int = false;
			break;
		}
	}
	if(is_int)
		return YAMLType::Int_;

	// Double (only 1 dot may be present; otherwise will be interpreted as a string. E.g. 420.69 -> double; 127.0.0.1 -> string)
	int number_of_dots = 0;
	bool contains_other_char = false;
	for(int i = 0; i < str.length(); i++)
	{
		// When only 1 dot is present but non-number chars are found, interpret value as string
		if((str[i] < '0' || str[i] > '9') && str[i] != '.' && str[i] != '-')
		{
			contains_other_char = true;
			break;
		}
		if(str[i] == '.')
			number_of_dots++;
		if(number_of_dots > 1)
			break;
	}
	if(number_of_dots == 1 && !contains_other_char)
		return YAMLType::Double_;
	
	// Everything else is interpreted as a String
	return YAMLType::String_;
}

std::vector<std::string> split_array(const std::string& str)
{
	std::string tmp = "";
	int scope = 0;
	std::vector<std::string> tokens;
	for(int i = 0; i < str.length(); i++)
	{
		if(str[i] == '[')
			scope++;
		else if(str[i] == ']')
			scope--;
		if((str[i] == ',' || str[i] == ']') && scope <= 1)
		{
			bool valid = true;
			tmp = tmp.substr(1, tmp.length()-1);
			if(tmp.length() == 0)
				valid = false;
			if(tmp[0] == '[')
				tmp += ']';
			if(valid)
				tokens.push_back(tmp);
			tmp = "";
		}
		tmp += str[i];
	}
	return tokens;
}

// Converts a value string to an actual value
NamedValue str_to_value(const std::string &str)
{
	NamedValue val;
	val.value = nullptr;
	
	std::array<std::string, 2> name_value = split_variable(str);
	YAMLType type = get_type(name_value[1]);
	
	val.type = type;
	val.name = name_value[0];
	
	if(type == YAMLType::None_)
	{
		return val;
	}

	switch(type)
	{
	case YAMLType::Int_:
		val.value = new int(std::stoi(name_value[1]));
		break;
	case YAMLType::Double_:
		val.value = new double(std::stod(name_value[1]));
		break;
	case YAMLType::String_:
		val.value = new std::string(name_value[1]);
		break;
	case YAMLType::Array_:
	{
		val.value = new Array();
		std::string values_str = name_value[1];
		std::vector<std::string> values_split = split_array(values_str);
		for(int i = 0; i < values_split.size(); i++)
		{
			std::string named_val = "_:" + values_split.at(i);
			TypedValue new_val;
			NamedValue nval = str_to_value(values_split.at(i));
			new_val.type = nval.type;
			new_val.value = nval.value;
			static_cast<Array*>(val.value)->push_back(new_val);
		}
		break;
	}
	case YAMLType::Bool_:
		val.value = new bool(name_value[1] == std::string("true"));
		break;
	}
	
	return val;
}

// Returns true if the input string is a valid key-value pair
bool is_valid(const NamedValue &val)
{
	return val.value != nullptr;
}

// Parses the provided YAML file and returns a hashmap representing the YAML data structure
YAML_map parse(std::string path)
{
	YAML_map yaml;
	std::vector<std::string> lines;

	int file_indent=0;	

	std::ifstream f(path);
    	if (!f.is_open()) {
        	std::cerr << "Encountered error while trying to open " << path << "\n";
        	return yaml;
    	}
    	std::string tmp;
    	while (getline(f, tmp))
	{
		tmp = tmp.substr(0, tmp.find('#')); // Ignore comments (everything that starts with a #)

		if(truncate_spaces(tmp).length() == 0)
			continue;

		int spaces = count_spaces(tmp);
		if(spaces > 0 && file_indent == 0)
			file_indent = spaces;

		if(tmp.find(':') != std::string::npos)
		{
        		lines.push_back(remove_spaces_after_char(remove_spaces_after_char(truncate_spaces_after(tmp), ':'), ','));
		}
		else
		{
			lines.at(lines.size()-1) += remove_spaces_after_char(remove_spaces_after_char(truncate_spaces(tmp), ':'), ',');
		}
	}
    	f.close();

	std::stack<void*> scope_stack; // This is used for properly handling nested objects
	scope_stack.push(&yaml);
	int prev_indent = 0;	
	
	for(int i = 0; i < lines.size(); i++)
	{
		int indent = count_spaces(lines.at(i));
		
		if(indent < prev_indent)
		{
			for(int i = 0; i < (prev_indent - indent) / file_indent; i++)
			{
				scope_stack.pop();
			}
		}

		NamedValue v = str_to_value(truncate_spaces(lines.at(i)));

		if(is_valid(v))
		{
			// Adding value to the deepest object
			(*((Object*)(scope_stack.top())))[v.name].value = v.value;
			(*((Object*)(scope_stack.top())))[v.name].type = v.type;
		}
		else
		{
			// Add a new object to the deepest object if the string has no value after the semicolon
			(*((Object*)(scope_stack.top())))[v.name] = TypedValue();
			(*((Object*)(scope_stack.top())))[v.name].type = YAMLType::Object_;
			(*((Object*)(scope_stack.top())))[v.name].value = new Object();
			scope_stack.push((*((Object*)(scope_stack.top())))[v.name].value); 
		}

		prev_indent = indent;
	}

	return yaml;
}

// Recursively frees every value
void delete_values(const TypedValue &v)
{
	switch(v.type)
	{
	case YAMLType::Int_:
		delete static_cast<int*>(v.value);
		break;
	case YAMLType::Double_:
		delete static_cast<double*>(v.value);
		break;
	case YAMLType::Bool_:
		delete static_cast<bool*>(v.value);
		break;
	case YAMLType::String_:
		delete static_cast<std::string*>(v.value);
		break;
	case YAMLType::Array_:
	{
		for(int i = 0; i < (static_cast<Array*>(v.value))->size(); i++)
		{
			delete_values((static_cast<Array*>(v.value))->at(i));
		}
		delete static_cast<Array*>(v.value);
		break;
	}
	case YAMLType::Object_:
	{
		for(auto it : *(static_cast<Object*>(v.value)))
		{
			delete_values(it.second);
		}
		delete static_cast<Object*>(v.value);
		break;
	}
	}
}

class YAML
{
public:
	YAML_map data;

	YAML(std::string path)
	{
		data = parse(path);
	}

	~YAML()
	{
		for(auto it : data)
		{
			delete_values(it.second);
		}
	}

	size_t size()
	{
		return data.size();
	}
	
	TypedValue operator[](std::string name)
	{
		return data[name];
	}
};
} // namespace yaml
