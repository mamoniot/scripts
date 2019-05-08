//By Monica Moniot

//MAM_DIM_PREFIX
//MAM_DIM_UINT
//MAM_DIM_T


#ifdef MAM_DIM_PREFIX
 #define MAM__PRE(name) MAM_DIM_PREFIX ## name
#else
 #define MAM__PRE(name) name
#endif

#ifndef MAM_DIM_T
 #define MAM_DIM_T float
#endif

#ifndef MAM_DIM_UINT
 #define MAM_DIM_UINT unsigned int
#endif


#define MAM__ARRAY1D MAM__PRE(array1d)
#define MAM__ARRAY2D MAM__PRE(array2d)
#define MAM__ARRAY3D MAM__PRE(array3d)
#define MAM__ARRAY4D MAM__PRE(array4d)


typedef struct MAM__ARRAY1D {
	MAM_DIM_T* ptr; MAM_DIM_UINT dim[1];
} MAM__ARRAY1D;
typedef struct MAM__ARRAY2D {
	MAM_DIM_T* ptr; MAM_DIM_UINT dim[2];
} MAM__ARRAY1D;
typedef struct MAM__ARRAY3D {
	MAM_DIM_T* ptr; MAM_DIM_UINT dim[3];
} MAM__ARRAY1D;
typedef struct MAM__ARRAY4D {
	MAM_DIM_T* ptr; MAM_DIM_UINT dim[4];
} MAM__ARRAY1D;

#ifdef __cplusplus
extern "C" {
#endif

static inline MAM__ARRAY1D MAM_PRE(init1d)(MAM_DIM_T* ptr, MAM_DIM_UINT dim0) {
	MAM__ARRAY1D arr = {ptr, {dim0}};
	return arr;
}
static inline MAM__ARRAY2D MAM_PRE(init2d)(MAM_DIM_T* ptr, MAM_DIM_UINT dim0, MAM_DIM_UINT dim1) {
	MAM__ARRAY2D arr = {ptr, {dim0, dim1}};
	return arr;
}
static inline MAM__ARRAY3D MAM_PRE(init3d)(MAM_DIM_T* ptr, MAM_DIM_UINT dim0, MAM_DIM_UINT dim1, MAM_DIM_UINT dim2) {
	MAM__ARRAY3D arr = {ptr, {dim0, dim1, dim2}};
	return arr;
}
static inline MAM__ARRAY4D MAM_PRE(init4d)(MAM_DIM_T* ptr, MAM_DIM_UINT dim0, MAM_DIM_UINT dim1, MAM_DIM_UINT dim2, MAM_DIM_UINT dim3) {
	MAM__ARRAY4D arr = {ptr, {dim0, dim1, dim2, dim3}};
	return arr;
}

static inline MAM_DIM_T* MAM_PRE(sub1d)(MAM__ARRAY1D arr, MAM_DIM_UINT i0) {
	MAM_DIM_ASSERT(i0 < arr.dim[0]);
	return &arr.ptr[i0];
}
static inline MAM_DIM_T* MAM_PRE(sub2d)(MAM__ARRAY2D arr, MAM_DIM_UINT i0, MAM_DIM_UINT i1) {
	MAM_DIM_ASSERT(i0 < arr.dim[0]);
	MAM_DIM_ASSERT(i1 < arr.dim[1]);
	return &arr.ptr[arr.dim[1]*i0 + i1];
}
static inline MAM_DIM_T* MAM_PRE(sub3d)(MAM__ARRAY3D arr, MAM_DIM_UINT i0, MAM_DIM_UINT i1, MAM_DIM_UINT i2) {
	MAM_DIM_ASSERT(i0 < arr.dim[0]);
	MAM_DIM_ASSERT(i1 < arr.dim[1]);
	MAM_DIM_ASSERT(i2 < arr.dim[2]);
	return &arr.ptr[(arr.dim[1]*i0 + i1)*arr.dim[2] + i2];
}
static inline MAM_DIM_T* MAM_PRE(sub4d)(MAM__ARRAY4D arr, MAM_DIM_UINT i0, MAM_DIM_UINT i1, MAM_DIM_UINT i2, MAM_DIM_UINT i3) {
	MAM_DIM_ASSERT(i0 < arr.dim[0]);
	MAM_DIM_ASSERT(i1 < arr.dim[1]);
	MAM_DIM_ASSERT(i2 < arr.dim[2]);
	MAM_DIM_ASSERT(i3 < arr.dim[3]);
	return &arr.ptr[((arr.dim[1]*i0 + i1)*arr.dim[2] + i2)*arr.dim[3] + i3];
}

static inline MAM_DIM_T* MAM_PRE(index1d)(MAM__ARRAY1D arr, MAM_DIM_UINT i0) {
	MAM_DIM_ASSERT(i0 < arr.dim[0]);
	return &arr.ptr[i0];
}
static inline MAM__ARRAY1D MAM_PRE(index2d)(MAM__ARRAY2D arr, MAM_DIM_UINT i0) {
	MAM_DIM_ASSERT(i0 < arr.dim[0]);
	MAM__ARRAY1D sub = {&arr.ptr[arr.dim[1]*i0], {arr.dim[1]}};
	return sub;
}
static inline MAM__ARRAY2D MAM_PRE(index3d)(MAM__ARRAY2D arr, MAM_DIM_UINT i0) {
	MAM_DIM_ASSERT(i0 < arr.dim[0]);
	MAM__ARRAY2D sub = {&arr.ptr[(arr.dim[1]*arr.dim[2])*i0], {arr.dim[1], arr.dim[2]}};
	return sub;
}
static inline MAM__ARRAY3D MAM_PRE(index4d)(MAM__ARRAY2D arr, MAM_DIM_UINT i0) {
	MAM_DIM_ASSERT(i0 < arr.dim[0]);
	MAM__ARRAY3D sub = {&arr.ptr[(arr.dim[1]*arr.dim[2]*arr.dim[3])*i0], {arr.dim[1], arr.dim[2], arr.dim[3]}};
	return sub;
}

#ifdef __cplusplus
}

static inline MAM_DIM_T& operator[](MAM__ARRAY1D arr, MAM_DIM_UINT i0) {
	return index1d(arr, i0);
}
static inline MAM__ARRAY1D operator[](MAM__ARRAY2D arr, MAM_DIM_UINT i0) {
	return index2d(arr, i0);
}
static inline MAM__ARRAY2D operator[](MAM__ARRAY3D arr, MAM_DIM_UINT i0) {
	return index3d(arr, i0);
}
static inline MAM__ARRAY3D operator[](MAM__ARRAY4D arr, MAM_DIM_UINT i0) {
	return index4d(arr, i0);
}

#endif


#ifdef MAM_DIM_PREFIX
 #undef MAM_DIM_PREFIX
#endif

#undef MAM_DIM_T float
#undef MAM_DIM_UINT unsigned int

#undef MAM__ARRAY1D
#undef MAM__ARRAY2D
#undef MAM__ARRAY3D
#undef MAM__ARRAY4D
