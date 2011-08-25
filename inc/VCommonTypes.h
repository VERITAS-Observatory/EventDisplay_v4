//-*-mode:c++; mode:font-lock;-*-

#ifndef VCOMMONTYPES_H
#define VCOMMONTYPES_H

#include<string>
#include<iostream>
#include<sstream>
#include<exception>

// ----------------------------------------------------------------------------
// COMMON TYPES -- TYPEDEFs AND FREQUENTLY USED CLASSES
// ----------------------------------------------------------------------------

typedef int                         index_type;

typedef index_type                  telescopenum_type;
typedef unsigned int                channelnum_type;

typedef unsigned int                utcdate_type;

// ----------------------------------------------------------------------------
// VException - BASE CLASS FOR ALL VERITAS EXCEPTION TYPES
// ----------------------------------------------------------------------------

class VException2: public std::exception
{
    public:
        virtual ~VException2() throw(){}

        virtual void printTypeSpecificExceptionDetails(std::ostream& stream) const;

        virtual const std::string& exceptionType() const { return m_exceptiontype; }
        virtual const std::string& thrownBy() const { return m_function; }
        virtual const std::string  message() const { return m_stream.str(); }

        virtual const char* what() const throw();

        std::ostream& stream() { return m_stream; }

    protected:

        VException2(const std::string& exceptiontype, const std::string& func)
            : exception(),
            m_stream(), m_exceptiontype(exceptiontype), m_function(func) {}

        VException2(const VException2& e)
            : std::exception(),
            m_stream(), m_exceptiontype(e.m_exceptiontype), m_function(e.m_function)
            { m_stream.str(e.message()); }

        const VException2& operator=(const VException2& e);

    private:
        std::ostringstream m_stream;
        std::string m_exceptiontype;
        std::string m_function;
};

//----------------------------------------------------------------------------

class VLogicError: public VException2
{
    public:
        virtual ~VLogicError() throw() { /* nothing to see here */ }

    protected:
        VLogicError(const std::string& exceptiontype, const std::string& func)
            : VException2(exceptiontype, func) { }
};

class VHardwareError: public VException2
{
    public:
        virtual ~VHardwareError() throw() { /* nothing to see here */ }

    protected:
        VHardwareError(const std::string& exceptiontype, const std::string& func)
            : VException2(exceptiontype, func) { }
};

// ----------------------------------------------------------------------------
// SPECIFIC EXCEPTIONS
// ----------------------------------------------------------------------------

class VIndexOutOfBounds: public VLogicError
{
    public:
        VIndexOutOfBounds(const std::string& func,
            index_type index, index_type max, index_type min=0)
            : VLogicError("VIndexOutOfBounds", func),
            m_index(index), m_max(max), m_min(min) {}
        virtual ~VIndexOutOfBounds() throw() { /* nothing to see here */ }

        index_type index() const { return m_index; }
        index_type max() const   { return m_max; }
        index_type min() const   { return m_min; }

        virtual void printTypeSpecificErrorDetails(std::ostream& stream) const;

    private:
        index_type m_index;
        index_type m_max;
        index_type m_min;
};

template<class parameter_type>
class VInvalidParameter: public VLogicError
{
    public:
        VInvalidParameter(const std::string& func, const std::string& paramname,
            const parameter_type& paramvalue)
            : VLogicError(func), m_paramname(paramname), m_paramvalue(paramvalue) {}
        virtual ~VInvalidParameter() throw() { /* nothing to see here */ }

        const std::string& parameterName() const     { return m_paramname; }
        const parameter_type& parameterValue() const { return m_paramvalue; }

        virtual void printTypeSpecificErrorDetails(std::ostream& stream) const;

    private:
        std::string    m_paramname;
        parameter_type m_paramvalue;
};

// ----------------------------------------------------------------------------
// Functions which are not members of a class
// ----------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& stream, const VException2 &x);

// ----------------------------------------------------------------------------
// Various functions which are too long to be in the class definition
// ----------------------------------------------------------------------------

inline const VException2& VException2::operator=(const VException2& e)
{
    m_exceptiontype = e.m_exceptiontype;
    m_function      = e.m_function;
    m_stream.str(e.message());
    return *this;
}


template<class parameter_type> void VInvalidParameter<parameter_type>::
printTypeSpecificErrorDetails(std::ostream& stream) const
{
    stream << "Parameter name:  " << parameterName() << std::endl
        << "Parameter value: " << parameterValue() << std::endl;
}
#endif                                            // VCOMMONTYPES_H
