/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  * Copyright by the Board of Trustees of the University of Illinois.         *
  * All rights reserved.                                                      *
  *		                                                              *
  * This file is part of HDF5.  The full HDF5 copyright notice, including     *
  * terms governing use, modification, and redistribution, is contained in    *
  * the files COPYING and Copyright.html.  COPYING can be found at the root   *
  * of the source code distribution tree; Copyright.html can be found at the  *
  * root level of an installed copy of the electronic HDF5 document set and   *
  * is linked from the top-level documents page.  It can also be found at     *
  * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *
  * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "H5Include.h"
#include "H5Exception.h"

#ifndef H5_NO_NAMESPACE
namespace H5 {
#ifndef H5_NO_STD
    using namespace std;
#endif  // H5_NO_STD
#endif

const string Exception::DEFAULT_MSG("No detailed information provided");

//--------------------------------------------------------------------------
// Function:	Exception default constructor
///\brief	Default constructor.
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
Exception::Exception() : detailMessage(""), funcName("") {}

//--------------------------------------------------------------------------
// Function:	Exception overloaded constructor
///\brief	Creates an exception with a function name where the failure
///		occurs and an optional detailed message
///\param	func_name - IN: Name of the function where failure occurs
///\param	message   - IN: Message on the failure
///\exception	H5::DataTypeIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
Exception::Exception(const string func_name, const string message) : detailMessage(message), funcName(func_name) {}

//--------------------------------------------------------------------------
// Function:	Exception copy constructor
///\brief	Copy constructor: makes a copy of the original Exception object.
///\param	orig - IN: Exception instance to copy
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
Exception::Exception( const Exception& orig )
{
   detailMessage = orig.detailMessage;
   funcName = orig.funcName;
}

//--------------------------------------------------------------------------
// Function:	Exception::getMajorString
///\brief	Returns the text string that describes an error
///		specified by a major error number.
///\param	err_major - IN: Major error number
///\par Description
///		In the failure case, the string "Invalid major error number"
///		will be returned.
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
string Exception::getMajorString( H5E_major_t major_num ) const
{
   // calls the C API routine to get the major string - Note: in the
   // failure case, the string "Invalid major error number" will be returned.
   string major_str( H5Eget_major( major_num ));
   return( major_str );
}

//--------------------------------------------------------------------------
// Function:	Exception::getMinorString
///\brief	Returns the text string that describes an error
///		specified by a minor error number.
///\param	err_minor - IN: Minor error number
///\par Description
///		In the failure case, the string "Invalid minor error number"
///		will be returned.
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
string Exception::getMinorString( H5E_minor_t minor_num ) const
{
   // calls the C API routine to get the minor string - Note: in the
   // failure case, the string "Invalid minor error number" will be returned.
   string minor_str( H5Eget_minor( minor_num ));
   return( minor_str );
}

//--------------------------------------------------------------------------
// Function:	Exception::setAutoPrint
///\brief	Turns on the automatic error printing.
///\param	func        - IN: Function to be called upon an error condition
///\param	client_data - IN: Data passed to the error function
///\par Description
///		When the library is first initialized the auto printing
///		function is set to the C API \c H5Eprint and \a client_data is
///		the standard error stream pointer, \c stderr.  Automatic stack
///		traversal is always in the \c H5E_WALK_DOWNWARD direction.
///\par
///		Users are encouraged to write their own more specific error
///		handlers
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
void Exception::setAutoPrint( H5E_auto_t& func, void* client_data )
{
   // calls the C API routine H5Eset_auto to set the auto printing to
   // the specified function.
   herr_t ret_value = H5Eset_auto( func, client_data );
   if( ret_value < 0 )
      throw Exception( "Exception::setAutoPrint", "H5Eset_auto failed" );
}

//--------------------------------------------------------------------------
// Function:	Exception::dontPrint
///\brief	Turns off the automatic error printing.
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
void Exception::dontPrint()
{
   // calls the C API routine H5Eset_auto with NULL parameters to turn
   // off the automatic error printing.
   herr_t ret_value = H5Eset_auto( NULL, NULL );
   if( ret_value < 0 )
      throw Exception( "Exception::dontPrint", "H5Eset_auto failed" );
}

//--------------------------------------------------------------------------
// Function:	Exception::getAutoPrint
///\brief	Retrieves the current settings for the automatic error
///		stack traversal function and its data.
///\param	func        - IN: Function to be called upon an error condition
///\param	client_data - IN: Data passed to the error function
///\par Description
///		When the library is first initialized the auto printing
///		function is set to the C API \c H5Eprint and \a client_data is
///		the standard error stream pointer, \c stderr.  Automatic stack
///		traversal is always in the \c H5E_WALK_DOWNWARD direction.
///\par
///		Users are encouraged to write their own more specific error
///		handlers
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
void Exception::getAutoPrint( H5E_auto_t& func, void** client_data )
{
   // calls the C API routine H5Eget_auto to get the current setting of
   // the automatic error printing 
   herr_t ret_value = H5Eget_auto( &func, client_data );
   if( ret_value < 0 )
      throw Exception( "Exception::getAutoPrint", "H5Eget_auto failed" );
}

//--------------------------------------------------------------------------
// Function:	Exception::clearErrorStack
///\brief	Clears the error stack for the current thread.
///\par Description
///		The stack is also cleared whenever a C API function is
///		called, with certain exceptions (for instance, H5Eprint).
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
void Exception::clearErrorStack()
{
   // calls the C API routine H5Eclear to clear the error stack
   herr_t ret_value = H5Eclear();
   if( ret_value < 0 )
      throw Exception( "Exception::clearErrorStack", "H5Eclear failed" );
}

//--------------------------------------------------------------------------
// Function:	Exception::walkErrorStack
///\brief	Walks the error stack for the current thread, calling the
///		specified function.
///\param	direction - IN: Direction in which the error stack is to be walked
///\param	func - IN: Function to be called for each error encountered
///\param	client_data - IN: Data passed to the error function
///\par Description
///		Valid values for \a direction include:
///		\li \c H5E_WALK_UPWARD - begin with the most specific error
///		        and end at the API
///		\li \c H5E_WALK_DOWNWARD - begin at the API and end at the
///		        inner-most function where the error was first detected
///\par
///		The function specified by \a func will be called for each
///		error in the error stack.  The \c H5E_walk_t prototype is as
///		follows:
///\code
/// typedef herr_t (*H5E_walk_t)(int n, H5E_error_t *err_desc, void *client_data)
///     int n - Indexed position of the error in the stack; it begins at zero
///		regardless of stack traversal direction
///     H5E_error_t *err_desc - Pointer to a data structure describing the
///		error.  This structure is listed below.
///     void *client_data - Pointer to client data in the format expected by
///		the user-defined function.
///\endcode
///\par
///     Data structure to describe the error:
///\code
/// typedef struct H5E_error_t {
///     hid_t       cls_id;         //class ID
///     hid_t       maj_num;        //major error ID
///     hid_t       min_num;        //minor error number
///     const char  *func_name;     //function in which error occurred
///     const char  *file_name;     //file in which error occurred
///     unsigned    line;           //line in file where error occurs
///     const char  *desc;          //optional supplied description
/// } H5E_error_t;
///\endcode
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
void Exception::walkErrorStack( H5E_direction_t direction, H5E_walk_t func, void* client_data )
{
   // calls the C API routine H5Ewalk to walk the error stack
   herr_t ret_value = H5Ewalk( direction, func, client_data );
   if( ret_value < 0 )
      throw Exception( "Exception::walkErrorStack", "H5Ewalk failed" );
}

//--------------------------------------------------------------------------
// Function:	Exception::getDetailMsg
///\brief	Returns the detailed message set at the time the exception
///		is thrown.
///\return	Text message - \c std::string
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
string Exception::getDetailMsg() const
{
   return(detailMessage);
}

//--------------------------------------------------------------------------
// Function:	Exception::getCDetailMsg
///\brief	Returns the detailed message set at the time the exception
///		is thrown.
///\return	Text message - \c char pointer
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
const char* Exception::getCDetailMsg() const
{
   return(detailMessage.c_str());
}

//--------------------------------------------------------------------------
// Function:	Exception::getFuncName
///\brief	Returns the name of the function, where the exception is thrown.
///\return	Text message - \c std::string
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
string Exception::getFuncName() const
{
   return(funcName);
}

//--------------------------------------------------------------------------
// Function:	Exception::getCFuncName
///\brief	Returns the name of the function, where the exception is thrown.
///\return	Text message - \c char pointer
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
const char* Exception::getCFuncName() const
{
   return(funcName.c_str());
}

//--------------------------------------------------------------------------
// Function:	Exception::printError
///\brief	Prints the error stack in a default manner.
///\param	stream - IN: File pointer
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
void Exception::printError( FILE* stream ) const
{
   herr_t ret_value = H5Eprint(stream); // print to stderr
   if( ret_value < 0 )
      throw Exception( "Exception::printError", "H5Eprint failed" );
}

//--------------------------------------------------------------------------
// Function:	Exception destructor
///\brief	Noop destructor
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
Exception::~Exception() {}

//--------------------------------------------------------------------------
// Subclasses:	FileIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// Function:	FileIException default constructor
///\brief	Default constructor.
//--------------------------------------------------------------------------
FileIException::FileIException():Exception(){}
//--------------------------------------------------------------------------
// Function:	FileIException overloaded constructor
///\brief	Creates a FileIException with a function name where the failure 
///		occurs and an optional detailed message.
///\param	func_name - IN: Name of the function where failure occurs
///\param	message   - IN: Message on the failure
//--------------------------------------------------------------------------
FileIException::FileIException(const string func_name, const string message) : Exception(func_name, message) {}
//--------------------------------------------------------------------------
// Function:	FileIException destructor
///\brief	Noop destructor.
//--------------------------------------------------------------------------
FileIException::~FileIException() {}

//--------------------------------------------------------------------------
// Subclasses:	GroupIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// Function:	GroupIException default constructor
///\brief	Default constructor.
//--------------------------------------------------------------------------
GroupIException::GroupIException():Exception(){}
//--------------------------------------------------------------------------
// Function:	GroupIException overloaded constructor
///\brief	Creates a GroupIException with a function name where the 
///		failure occurs and an optional detailed message.
///\param	func_name - IN: Name of the function where failure occurs
///\param	message   - IN: Message on the failure
//--------------------------------------------------------------------------
GroupIException::GroupIException(const string func_name, const string message) : Exception(func_name, message) {}
//--------------------------------------------------------------------------
// Function:	GroupIException destructor
///\brief	Noop destructor.
//--------------------------------------------------------------------------
GroupIException::~GroupIException() {}

//--------------------------------------------------------------------------
// Subclasses:	DataSpaceIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// Function:	DataSpaceIException default constructor
///\brief	Default constructor.
//--------------------------------------------------------------------------
DataSpaceIException::DataSpaceIException():Exception(){}
//--------------------------------------------------------------------------
// Function:	DataSpaceIException overloaded constructor
///\brief	Creates a DataSpaceIException with a function name where 
///		the failure occurs and an optional detailed message.
///\param	func_name - IN: Name of the function where failure occurs
///\param	message   - IN: Message on the failure
//--------------------------------------------------------------------------
DataSpaceIException::DataSpaceIException(const string func_name, const string message) : Exception(func_name, message) {}
//--------------------------------------------------------------------------
// Function:	DataSpaceIException destructor
///\brief	Noop destructor.
//--------------------------------------------------------------------------
DataSpaceIException::~DataSpaceIException() {}

//--------------------------------------------------------------------------
// Subclasses:	DataTypeIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// Function:	DataTypeIException default constructor
///\brief	Default constructor.
//--------------------------------------------------------------------------
DataTypeIException::DataTypeIException():Exception(){}
//--------------------------------------------------------------------------
// Function:	DataTypeIException overloaded constructor
///\brief	Creates a DataTypeIException with a function name where the 
///		failure occurs and an optional detailed message.
///\param	func_name - IN: Name of the function where failure occurs
///\param	message   - IN: Message on the failure
//--------------------------------------------------------------------------
DataTypeIException::DataTypeIException(const string func_name, const string message) : Exception(func_name, message) {}
//--------------------------------------------------------------------------
// Function:	DataTypeIException destructor
///\brief	Noop destructor.
//--------------------------------------------------------------------------
DataTypeIException::~DataTypeIException() {}

//--------------------------------------------------------------------------
// Subclasses:	PropListIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// Function:	PropListIException default constructor
///\brief	Default constructor.
//--------------------------------------------------------------------------
PropListIException::PropListIException():Exception(){}
//--------------------------------------------------------------------------
// Function:	PropListIException overloaded constructor
///\brief	Creates a PropListIException with a function name where 
///		the failure occurs and an optional detailed message.
///\param	func_name - IN: Name of the function where failure occurs
///\param	message   - IN: Message on the failure
//--------------------------------------------------------------------------
PropListIException::PropListIException(const string func_name, const string message) : Exception(func_name, message) {}
//--------------------------------------------------------------------------
// Function:	PropListIException destructor
///\brief	Noop destructor.
//--------------------------------------------------------------------------
PropListIException::~PropListIException() {}

//--------------------------------------------------------------------------
// Subclasses:	DataSetIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// Function:	DataSetIException default constructor
///\brief	Default constructor.
//--------------------------------------------------------------------------
DataSetIException::DataSetIException():Exception(){}
//--------------------------------------------------------------------------
// Function:	DataSetIException overloaded constructor
///\brief	Creates a DataSetIException with a function name where 
///		the failure occurs and an optional detailed message.
///\param	func_name - IN: Name of the function where failure occurs
///\param	message   - IN: Message on the failure
//--------------------------------------------------------------------------
DataSetIException::DataSetIException(const string func_name, const string message) : Exception(func_name, message) {}
//--------------------------------------------------------------------------
// Function:	DataSetIException destructor
///\brief	Noop destructor.
//--------------------------------------------------------------------------
DataSetIException::~DataSetIException() {}

//--------------------------------------------------------------------------
// Subclasses:	AttributeIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// Function:	AttributeIException default constructor
///\brief	Default constructor.
//--------------------------------------------------------------------------
AttributeIException::AttributeIException():Exception(){}
//--------------------------------------------------------------------------
// Function:	AttributeIException overloaded constructor
///\brief	Creates a AttributeIException with a function name where 
///		the failure occurs and an optional detailed message.
///\param	func_name - IN: Name of the function where failure occurs
///\param	message   - IN: Message on the failure
//--------------------------------------------------------------------------
AttributeIException::AttributeIException(const string func_name, const string message) : Exception(func_name, message) {}
//--------------------------------------------------------------------------
// Function:	AttributeIException destructor
///\brief	Noop destructor.
//--------------------------------------------------------------------------
AttributeIException::~AttributeIException() {}

//--------------------------------------------------------------------------
// Subclasses:	ReferenceException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// Function:	ReferenceException default constructor
///\brief	Default constructor.
//--------------------------------------------------------------------------
ReferenceException::ReferenceException():Exception(){}
//--------------------------------------------------------------------------
// Function:	ReferenceException overloaded constructor
///\brief	Creates a ReferenceException with a function name where 
///		the failure occurs and an optional detailed message.
///\param	func_name - IN: Name of the function where failure occurs
///\param	message   - IN: Message on the failure
//--------------------------------------------------------------------------
ReferenceException::ReferenceException(const string func_name, const string message) : Exception(func_name, message) {}
//--------------------------------------------------------------------------
// Function:	ReferenceException destructor
///\brief	Noop destructor.
//--------------------------------------------------------------------------
ReferenceException::~ReferenceException() {}

//--------------------------------------------------------------------------
// Subclasses:	LibraryIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// Function:	LibraryIException default constructor
///\brief	Default constructor.
//--------------------------------------------------------------------------
LibraryIException::LibraryIException():Exception(){}
//--------------------------------------------------------------------------
// Function:	LibraryIException overloaded constructor
///\brief	Creates a LibraryIException with a function name where 
///		the failure occurs and an optional detailed message.
///\param	func_name - IN: Name of the function where failure occurs
///\param	message   - IN: Message on the failure
//--------------------------------------------------------------------------
LibraryIException::LibraryIException(const string func_name, const string message) : Exception(func_name, message) {}
//--------------------------------------------------------------------------
// Function:	LibraryIException destructor
///\brief	Noop destructor.
//--------------------------------------------------------------------------
LibraryIException::~LibraryIException() {}

//--------------------------------------------------------------------------
// Subclasses:	IdComponentException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// Function:	IdComponentException default constructor
///\brief	Default constructor.
//--------------------------------------------------------------------------
IdComponentException::IdComponentException(): Exception() {}
//--------------------------------------------------------------------------
// Function:	IdComponentException overloaded constructor
///\brief	Creates a IdComponentException with a function name where 
///		the failure occurs and an optional detailed message.
///\param	func_name - IN: Name of the function where failure occurs
///\param	message   - IN: Message on the failure
//--------------------------------------------------------------------------
IdComponentException::IdComponentException(const string func_name, const string message) : Exception(func_name, message) {}
//--------------------------------------------------------------------------
// Function:	IdComponentException destructor
///\brief	Noop destructor.
//--------------------------------------------------------------------------
IdComponentException::~IdComponentException() {}
#ifndef H5_NO_NAMESPACE
} // end namespace
#endif
