/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  * Copyright by the Board of Trustees of the University of Illinois.         *
  * All rights reserved.                                                      *
  *                                                                           *
  * This file is part of HDF5.  The full HDF5 copyright notice, including     *
  * terms governing use, modification, and redistribution, is contained in    *
  * the files COPYING and Copyright.html.  COPYING can be found at the root   *
  * of the source code distribution tree; Copyright.html can be found at the  *
  * root level of an installed copy of the electronic HDF5 document set and   *
  * is linked from the top-level documents page.  It can also be found at     *
  * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *
  * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <string>

#include "H5Include.h"
#include "H5Exception.h"
#include "H5IdComponent.h"
#include "H5PropList.h"
#include "H5Object.h"
#include "H5DcreatProp.h"
#include "H5CommonFG.h"
#include "H5Alltypes.h"
#include "H5AbstractDs.h"
#include "H5DxferProp.h"
#include "H5DataSpace.h"
#include "H5DataSet.h"
#include "H5private.h"

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

//--------------------------------------------------------------------------
// Function:	CompType default constructor
///\brief	Default constructor: Creates a stub compound datatype
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
CompType::CompType() : DataType() {}

//--------------------------------------------------------------------------
// Function:	CompType copy constructor
///\brief	Copy constructor: makes copy of the original CompType object
///\param	original - IN: Original CompType instance
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
CompType::CompType( const CompType& original ) : DataType( original ) {}

//--------------------------------------------------------------------------
// Function:	CompType overloaded constructor
///\brief	Creates a CompType object using the id of an existing datatype.
///\param	existing_id - IN: Id of an existing compound datatype
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
CompType::CompType( const hid_t existing_id ) : DataType( existing_id ) {}

//--------------------------------------------------------------------------
// Function:	CompType overloaded constructor
///\brief	Creates an empty compound datatype given a size, in bytes.
///\param	size - IN: Number of bytes in the datatype to create
///\exception	H5::DataTypeIException
// Description
// 		The DataType constructor calls the C API H5Tcreate to create
// 		the compound datatype.
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
CompType::CompType( size_t size ) : DataType( H5T_COMPOUND, size ) {}

//--------------------------------------------------------------------------
// Function:	CompType overloaded constructor
///\brief	Gets the compound datatype of the specified dataset.
///\param	dataset - IN: Dataset that this enum datatype associates with
///\return	CompType instance
///\exception	H5::DataTypeIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
CompType::CompType( const DataSet& dataset ) : DataType()
{
   // Calls C function H5Dget_type to get the id of the datatype
   id = H5Dget_type( dataset.getId() );

   // If the datatype id is invalid, throw exception
   if( id <= 0 )
   {
      throw DataSetIException("CompType constructor", "H5Dget_type failed");
   }
}

//--------------------------------------------------------------------------
// Function:	CompType::getNmembers
///\brief	Returns the number of members in this compound datatype. 
///\return	Number of members
///\exception	H5::DataTypeIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
int CompType::getNmembers() const
{
   int num_members = H5Tget_nmembers( id );
   if( num_members < 0 )
   {
      throw DataTypeIException("CompType::getNmembers", 
		"H5Tget_nmembers returns negative number of members");
   }
   return( num_members );
}

//--------------------------------------------------------------------------
// Function:	CompType::getMemberName
///\brief	Returns the name of a member in this compound datatype. 
///\param	member_num - IN: Zero-based index of the member
///\return	Name of member
///\exception	H5::DataTypeIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
string CompType::getMemberName( int member_num ) const
{
    char* member_name_C = H5Tget_member_name( id, member_num );
    if( member_name_C == NULL )  // NULL means failure
    {
	throw DataTypeIException("CompType::getMemberName", 
		"H5Tget_member_name returns NULL for member name");
    }
    string member_name = string(member_name_C); // convert C string to string 
    HDfree(member_name_C); // free the C string 
    return( member_name ); // return the member name string
}

//--------------------------------------------------------------------------
// Function:	CompType::getMemberIndex
///\brief	Returns the index of a member in this compound datatype. 
///\param	name - IN: Name of the member
///\return	Index of member
///\exception	H5::DataTypeIException
///\par Description
///		Members are stored in no particular order with numbers 0 
///		through N-1, where N is the value returned by the member 
///		function \c CompType::getNmembers.
// Programmer	Binh-Minh Ribler - May 16, 2002
//--------------------------------------------------------------------------
int CompType::getMemberIndex(const char* name) const
{
   int member_index = H5Tget_member_index(id, name);
   if( member_index < 0 )
   {
      throw DataTypeIException("CompType::getMemberIndex", 
		"H5Tget_member_index returns negative value");
   }
   return( member_index );
}
int CompType::getMemberIndex(const string& name) const
{
   return(getMemberIndex(name.c_str()));
}

//--------------------------------------------------------------------------
// Function:	CompType::getMemberOffset
///\brief	Returns the byte offset of the beginning of a member with
///		respect to the beginning of the compound data type datum.
///\param	member_num - IN: Zero-based index of the member
///\return	Byte offset
///\exception	H5::DataTypeIException
// Programmer	Binh-Minh Ribler - 2000
// Description
///		Members are stored in no particular order with numbers 0 
///		through N-1, where N is the value returned by the member 
///		function \c CompType::getNmembers.
//
//		Note that byte offset being returned as 0 doesn't indicate
//		a failure. (According to Quincey)
//--------------------------------------------------------------------------
size_t CompType::getMemberOffset( int member_num ) const
{
   size_t offset = H5Tget_member_offset( id, member_num );
   return( offset );
}

// Returns the dimensionality of the member. 
int CompType::getMemberDims( int member_num, size_t* dims, int* perm ) const
{
   throw DataTypeIException( "Error: getMemberDims is no longer supported." );
   return (-1); // unreachable statement; but without it, the compiler 
		// will complain
}

//--------------------------------------------------------------------------
// Function:	CompType::getMemberClass
///\brief	Gets the type class of the specified member.
///\param	member_num - IN: Zero-based index of the member
///\return	Type class of the member
///\exception	H5::DataTypeIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
H5T_class_t CompType::getMemberClass( int member_num ) const
{
   // get the member datatype first
   hid_t member_type_id = H5Tget_member_type( id, member_num );
   if( member_type_id <= 0 )
   {
      throw DataTypeIException("CompType::getMemberClass", 
		"H5Tget_member_type failed");
   }

   // then get its class
   H5T_class_t member_class = H5Tget_class( member_type_id );
   if( member_class == H5T_NO_CLASS )
   {
      throw DataTypeIException("CompType::getMemberClass", 
		"H5Tget_class returns H5T_NO_CLASS");
   }
   return( member_class );
}

// This private member function calls the C API to get the identifier 
// of the specified member.  It provides the id to construct appropriate
// sub-types in the functions getMemberXxxType below, where Xxx indicates
// the sub-types.
hid_t CompType::p_getMemberType( int member_num ) const
{
   // get the id of the specified member first
   hid_t member_type_id = H5Tget_member_type( id, member_num );
   if( member_type_id > 0 )
      return( member_type_id );
   else
   {
	// p_getMemberType is private, use caller's function name for api
      throw DataTypeIException("CompType::getMemberDataType", 
		"H5Tget_member_type failed");
   }
}

//--------------------------------------------------------------------------
// Function:	CompType::getMemberDataType
///\brief	Returns the generic datatype of the specified member in this 
///		compound datatype. 
///\param	member_num - IN: Zero-based index of the member
///\return	DataType instance
///\exception	H5::DataTypeIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
DataType CompType::getMemberDataType( int member_num ) const
{
   DataType datatype( p_getMemberType( member_num )); 
   return( datatype );
}

//--------------------------------------------------------------------------
// Function:	CompType::getMemberDataType
///\brief	Returns the enumeration datatype of the specified member in 
///		this compound datatype. 
///\param	member_num - IN: Zero-based index of the member
///\return	EnumType instance
///\exception	H5::DataTypeIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
EnumType CompType::getMemberEnumType( int member_num ) const
{
   EnumType enumtype( p_getMemberType( member_num )); 
   return( enumtype );
}

//--------------------------------------------------------------------------
// Function:	CompType::getMemberDataType
///\brief	Returns the compound datatype of the specified member in this 
///		compound datatype. 
///\param	member_num - IN: Zero-based index of the member
///\return	CompType instance
///\exception	H5::DataTypeIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
CompType CompType::getMemberCompType( int member_num ) const
{
   CompType comptype( p_getMemberType( member_num )); 
   return( comptype );
}

//--------------------------------------------------------------------------
// Function:	CompType::getMemberDataType
///\brief	Returns the integer datatype of the specified member in this 
///		compound datatype. 
///\param	member_num - IN: Zero-based index of the member
///\return	IntType instance
///\exception	H5::DataTypeIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
IntType CompType::getMemberIntType( int member_num ) const
{
   IntType inttype( p_getMemberType( member_num )); 
   return( inttype );
}

//--------------------------------------------------------------------------
// Function:	CompType::getMemberDataType
///\brief	Returns the floating-point datatype of the specified member 
///		in this compound datatype. 
///\param	member_num - IN: Zero-based index of the member
///\return	FloatType instance
///\exception	H5::DataTypeIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
FloatType CompType::getMemberFloatType( int member_num ) const
{
   FloatType floatype( p_getMemberType( member_num )); 
   return( floatype );
}

//--------------------------------------------------------------------------
// Function:	CompType::getMemberDataType
///\brief	Returns the string datatype of the specified member in this 
///		compound datatype. 
///\param	member_num - IN: Zero-based index of the member
///\return	StrType instance
///\exception	H5::DataTypeIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
StrType CompType::getMemberStrType( int member_num ) const
{
   StrType strtype( p_getMemberType( member_num )); 
   return( strtype );
}

/* old style of getMemberType - using overloads; new style above 
   returns the appropriate datatypes but has different named functions.
   In the old style, a datatype must be passed into the function.
   May, 2004: These should be reconsidered to provide more convenience.
// Returns the datatype of the specified member in this compound datatype. 
// Several overloading of getMemberType are for different datatypes
void CompType::getMemberType( int member_num, EnumType& enumtype ) const
{
   p_getMemberType( member_num, enumtype ); 
}

void CompType::getMemberType( int member_num, CompType& comptype ) const
{
   p_getMemberType( member_num, comptype ); 
}

void CompType::getMemberType( int member_num, IntType& inttype ) const
{
   p_getMemberType( member_num, inttype ); 
}

void CompType::getMemberType( int member_num, FloatType& floatype ) const
{
   p_getMemberType( member_num, floatype ); 
}

void CompType::getMemberType( int member_num, StrType& strtype ) const
{
   p_getMemberType( member_num, strtype ); 
}
// end of overloading of getMemberType
*/

//--------------------------------------------------------------------------
// Function:	CompType::insertMember
///\brief	Inserts a new member to this compound datatype.
///\param	name - IN: Name of the new member
///\param	offset - IN: Offset in memory structure of the field to insert
///\param	new_member - IN: New member to be inserted
///\exception	H5::DataTypeIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
void CompType::insertMember( const string& name, size_t offset, const DataType& new_member ) const
{
   // Convert string to C-string
   const char* name_C;
   name_C = name.c_str();  // name_C refers to the contents of name as a C-str

   hid_t new_member_id = new_member.getId();  // get new_member id for C API

   // Call C routine H5Tinsert to add the new member
   herr_t ret_value = H5Tinsert( id, name_C, offset, new_member_id );
   if( ret_value < 0 )
   {
      throw DataTypeIException("CompType::insertMember", "H5Tinsert failed");
   }
}

//--------------------------------------------------------------------------
// Function:	CompType::pack
///\brief	Recursively removes padding from within a compound datatype. 
///\exception	H5::DataTypeIException
// Programmer	Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
void CompType::pack() const
{
   // Calls C routine H5Tpack to remove padding
   herr_t ret_value = H5Tpack( id );
   if( ret_value < 0 )
   {
      throw DataTypeIException("CompType::pack", "H5Tpack failed");
   }
}

//--------------------------------------------------------------------------
// Function:    CompType destructor
///\brief       Properly terminates access to this compound datatype.
// Programmer   Binh-Minh Ribler - 2000
//--------------------------------------------------------------------------
CompType::~CompType() {}

#ifndef H5_NO_NAMESPACE
} // end namespace
#endif
