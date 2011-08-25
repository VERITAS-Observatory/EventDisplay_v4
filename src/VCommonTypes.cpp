#include<iostream>
#include<sstream>
#include<string>

#include"VCommonTypes.h"

void VException2::printTypeSpecificExceptionDetails(std::ostream& stream) const
{
// nothing to see here
}


const char* VException2::what() const throw()
{
    static std::string whatstring;
    std::ostringstream whatstream;
    whatstream << *this;
    whatstring = whatstream.str();                // not a good way to do this but "what" will
    return whatstring.c_str();                    // probably not be called very often.
}


void VIndexOutOfBounds::
printTypeSpecificErrorDetails(std::ostream& stream) const
{
    stream << "Index: " << index()
        << " must be in range " << min() << " <= x < " << max() << std::endl;
}


std::ostream& operator<<(std::ostream& stream, const VException2 &x)
{
    stream << "Exception " << x.exceptionType() << " thrown by function "
        << x.thrownBy() << std::endl;
    stream << x.message() << std::endl;
    x.printTypeSpecificExceptionDetails(stream);
    return stream;
}
