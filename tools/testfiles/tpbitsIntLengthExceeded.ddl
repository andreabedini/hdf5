#############################
Expected output for 'h5dump -d /DS16BITS -M 10,7 packedbits.h5'
#############################
HDF5 "packedbits.h5" {
DATASET "/DS16BITS" {
   DATATYPE  H5T_STD_I16LE
   DATASPACE  SIMPLE { ( 8, 16 ) / ( 8, 16 ) }
   PACKED_BITS OFFSET=10 LENGTH=7
   DATA {
   (0,0): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   (1,0): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   (2,0): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   (3,0): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   (4,0): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   (5,0): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   (6,0): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   (7,0): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
   }
}
}
h5dump error: Packed Bit offset+length value(17) too large. Max is 16
