#pragma once

#include <iostream>
#include <sstream>

#include <initializer_list>
#include <set>
#include <unordered_set>
#include <vector>
#include <list>

enum ConType { notInit, stringType, intType, longType, doubleType, boolType, vectorType, pairType, setType };
class ConvertObj {
    private:    
        std::string string_value = "";
        int int_value = 0;
        long long_value = 0.;
        double double_value = 0.;
        bool bool_value = false;
        std::vector<ConvertObj> vector_value;
    
    public:
        ConType held_type = ConType::notInit;
        ConvertObj() {
            held_type = ConType::notInit;
        }

        ConvertObj(const std::string &value) {
            held_type = ConType::stringType;
            string_value = value;
        }

        ConvertObj(const char* value) {
            held_type = ConType::stringType;
            string_value = value;
        }

        ConvertObj(int value) {
            held_type = ConType::intType;
            int_value = value;
        }
        
        ConvertObj(long value) {
            held_type = ConType::longType;
            long_value = value;
        }

        ConvertObj(double value) {
            held_type = ConType::doubleType;
            double_value = value;
        }

        ConvertObj(bool value) {
            held_type = ConType::boolType;
            bool_value = value;
        }

        template<typename T>
        ConvertObj(std::vector<T> itr) {
            vector_value.clear();
            held_type = ConType::vectorType;
            for (auto it: itr) {
                vector_value.push_back(ConvertObj(it));
            }
        }

        template<typename T, typename G>
        ConvertObj(std::pair<T, G> value) {
            vector_value.clear();
            held_type = ConType::pairType;
            vector_value.push_back(ConvertObj(value.first));
            vector_value.push_back(ConvertObj(value.second));
        }

        template<typename T>
        ConvertObj(std::unordered_set<T> itr) {
            vector_value.clear();
            held_type = ConType::setType;
            for (auto it = std::begin(itr); it!=std::end(itr); ++it) {
                vector_value.push_back(ConvertObj(*it));
            }
        }

        template<typename T>
        ConvertObj(std::set<T> itr) {
            vector_value.clear();
            held_type = ConType::setType;
            for (auto it = std::begin(itr); it!=std::end(itr); ++it) {
                vector_value.push_back(ConvertObj(*it));
            }
        }

        bool operator < (const ConvertObj& rhs) const 
        {
            if (held_type == rhs.held_type) {
                if (held_type == ConType::stringType) {
                    return string_value < rhs.string_value;
                }
                if (held_type == ConType::intType) {
                    return int_value < rhs.int_value;
                }
                if (held_type == ConType::longType) {
                    return long_value < rhs.long_value;
                }
                if (held_type == ConType::doubleType) {
                    return double_value < rhs.double_value;
                }
                if (held_type == ConType::boolType) {
                    return bool_value != rhs.bool_value;
                }
            }
            return false;
        }

        std::string to_string();
};

struct Obj2Str {
    static std::string convert(ConvertObj x) {return x.to_string();}
    static std::string convert(const std::string &x) { return x; }
    static std::string convert(int x) { return std::to_string(x); }
    static std::string convert(long x) { return std::to_string(x); }
    static std::string convert(double x) { return std::to_string(x); }
    static std::string convert(bool x) {return x ? "true" : "false"; }

    template<typename T, typename G>
    static std::string convert(std::pair<T, G> x) {
        std::ostringstream rv;
        rv << "(" << Obj2Str::convert(x.first) << "," << Obj2Str::convert(x.second) << ")";
        return rv.str(); 
    }
    
    template<typename T>
    static std::string convert(std::unordered_set<T> x) {
        std::ostringstream rv;
        rv << "{";
        int i = 0;
        int xsi = x.size();
        for (auto it = std::begin(x); it!=std::end(x); ++it) {
            rv << Obj2Str::convert(*it);
            if (i < x.size() - 1) {
                rv << ",";
            }
            i++;
        }
        rv << "}";
        return rv.str();
    }

    template<typename T>
    static std::string convert(std::set<T> x) {
        std::ostringstream rv;
        rv << "{";
        int i = 0;
        int xsi = x.size();
        for (auto it = std::begin(x); it!=std::end(x); ++it) {
            rv << Obj2Str::convert(*it);
            if (i < x.size() - 1) {
                rv << ",";
            }
            i++;
        }
        rv << "}";
        return rv.str();
    }

    template <typename T>
    static std::string convert(std::vector<T> v) {
        std::ostringstream buffer;
        buffer << "[";
        for (int i = 0; i < v.size(); i++){
            if (i == (v.size() - 1) ){
                buffer << Obj2Str::convert(v[i]);
            }else{
                buffer << Obj2Str::convert(v[i]) << ",";
            }
        }
        buffer << "]";
        return buffer.str();
    }

    template <typename T>
    static std::string convert(std::vector<std::vector<T>> v) {
        std::ostringstream buffer;
        buffer << "[";
        for(int i=0; i< v.size();i++) {
            std::vector<T> piece = v[i];
            int pieceSize = piece.size();
            buffer << "[";
            for(int k=0; k < pieceSize; k++) {
                buffer << Obj2Str::convert(piece[k]);
                if(k == (pieceSize-1)){
                    buffer << "]";
                } else {
                    buffer << ",";
                }
            }
            if(i != v.size()-1) buffer << ",";
        }
        buffer << "]";
        return buffer.str();
    }

    template<typename T>
    static std::string convert(std::initializer_list<T> x) {
        std::ostringstream rv;
        int i = 0;
        int xsi = x.size();
        for (auto it = std::begin(x); it!=std::end(x); ++it) {
            rv << Obj2Str::convert(*it);
            if (i < x.size() - 1) {
                rv << " ";
            }
            i++;
        }
        return rv.str();
    }
};

std::string ConvertObj::to_string() {
    if (held_type == ConType::stringType) {
        return string_value;
    }
    if (held_type == ConType::intType) {
        return Obj2Str::convert(int_value);
    }
    if (held_type == ConType::longType) {
        return Obj2Str::convert(long_value);
    }
    if (held_type == ConType::doubleType) {
        return Obj2Str::convert(double_value);
    }
    if (held_type == ConType::boolType) {
        return Obj2Str::convert(bool_value);
    }
    if (held_type == ConType::vectorType) {
        return Obj2Str::convert(vector_value);
    }
    if (held_type == ConType::pairType) {
        std::pair<ConvertObj, ConvertObj> tvar {vector_value[0], vector_value[1]};
        return Obj2Str::convert(tvar);
    }
    if (held_type == ConType::setType) {
        std::set<ConvertObj> tvar;
        for (auto it: vector_value) {
            tvar.insert(it);
        }
        return Obj2Str::convert(tvar);
    }
    return "";
}

#define __VMSG__(...) " " << __func__ << "@[" << __FILE__ << ":" << __LINE__ << "] " <<  Obj2Str::convert(std::initializer_list<ConvertObj>{__VA_ARGS__})
#define INFO_MSG(...) std::cout << "INFO  " << __VMSG__(__VA_ARGS__) << std::endl
#define WARN_MSG(...) std::cout << "WARN  " << __VMSG__(__VA_ARGS__) << std::endl
#define ERR_MSG(...) std::cout << "ERROR " << __VMSG__(__VA_ARGS__) << std::endl
