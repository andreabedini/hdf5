// C++ informative line for the emacs editor: -*- C++ -*-
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

#ifndef _H5File_H
#define _H5File_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

class H5_DLLCPP H5File : public IdComponent, public CommonFG {
   public:
	// Default constructor
	H5File();

	// copy constructor: makes a copy of the original H5File object.
	H5File(const H5File& original );

	// Creates or opens an HDF5 file.
	H5File( const string& name, unsigned int flags,
	   const FileCreatPropList& create_plist = FileCreatPropList::DEFAULT,
	   const FileAccPropList& access_plist = FileAccPropList::DEFAULT );
	H5File( const char* name, unsigned int flags,
	   const FileCreatPropList& create_plist = FileCreatPropList::DEFAULT,
	   const FileAccPropList& access_plist = FileAccPropList::DEFAULT );

	// Gets the file id
	virtual hid_t getLocId() const;

	// Returns the amount of free space in the file.
	hssize_t getFreeSpace() const;

	// Returns the number of opened object IDs (files, datasets, groups 
	// and datatypes) in the same file.
	int getObjCount(unsigned types) const;
	int getObjCount() const;

	// Retrieves a list of opened object IDs (files, datasets, groups 
	// and datatypes) in the same file.
	void getObjIDs(unsigned types, int max_objs, hid_t *oid_list) const;

	// Returns the pointer to the file handle of the low-level file driver.
	void getVFDHandle(FileAccPropList& fapl, void **file_handle) const;
	void getVFDHandle(void **file_handle) const;

	// Determines if a file, specified by its name, is in HDF5 format
	static bool isHdf5(const string& name );
	static bool isHdf5(const char* name );

	// Creates a reference to a named Hdf5 object in this object.
	void* Reference(const char* name) const;

	// Creates a reference to a named Hdf5 object or to a dataset region 
	// in this object.
	void* Reference(const char* name, DataSpace& dataspace, H5R_type_t ref_type = H5R_DATASET_REGION) const;

	// Retrieves the type of object that an object reference points to.
	H5G_obj_t getObjType(void *ref, H5R_type_t ref_type) const;

	// Retrieves a dataspace with the region pointed to selected.
	DataSpace getRegion(void *ref, H5R_type_t ref_type = H5R_DATASET_REGION) const;

        // Retrieves the file size of an opened file.
        haddr_t getFileSize() const;

	// Reopens this file.
	void reopen();

	// Gets the creation property list of this file.
	FileCreatPropList getCreatePlist() const;

	// Gets the access property list of this file.
	FileAccPropList getAccessPlist() const;

	// Throw file exception.
	virtual void throwException(const string func_name, const string msg) const;

	// Used by the API to appropriately close a file.
	void p_close() const;

	// H5File destructor.
	virtual ~H5File();

   private:
	// This function is private and contains common code between the
	// constructors taking a string or a char*
	void p_get_file( const char* name, unsigned int flags, const FileCreatPropList& create_plist, const FileAccPropList& access_plist );

};
#ifndef H5_NO_NAMESPACE
}
#endif
#endif
