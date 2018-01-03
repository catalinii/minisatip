
#if !defined(COVERITY_COMPAT_d682614b494b1ee17acf3e5fa6fb52f8)
#define COVERITY_COMPAT_d682614b494b1ee17acf3e5fa6fb52f8
#pragma builtin begin

/*
  Copyright (c) 2017 Synopsys, Inc. All rights reserved worldwide.
  The information contained in this file is the proprietary and confidential
  information of Synopsys, Inc. and its licensors, and is supplied subject to,
  and may be used only by Synopsys customers in accordance with the terms and
  conditions of a previously executed license agreement between Synopsys and
  that customer.
*/


/* DO NOT MODIFY THE CONTENTS OF THIS FILE */


#define __COVERITY_GCC_VERSION_AT_LEAST(maj, min) \
    ((__GNUC__ > (maj)) || (__GNUC__ == (maj) && __GNUC_MINOR__ >= (min)))

#if defined(__APPLE__)
#if defined(__BLOCKS__)
#undef __BLOCKS__
#endif

const void* __builtin___CFStringMakeConstantString(const char*);
#endif

#if defined(__GNUC__)

#if defined(__SPU__) || defined(__PPC__)
#if defined(__COVERITY_GCC_DIALECT_C89) || defined(__COVERITY_GCC_DIALECT_GNU89)
  #undef __STDC_VERSION__
#endif
#endif

#if __COVERITY_GCC_VERSION_AT_LEAST(4, 0)
// Taken from the gcc 4.3.0 manual
// definitions for long, long long and their unsigned counterparts may be needed

extern int __sync_fetch_and_add (volatile int*, int, ...);
extern int __sync_fetch_and_sub (volatile int*, int, ...);
extern int __sync_fetch_and_or (volatile int*, int, ...);
extern int __sync_fetch_and_and (volatile int*, int, ...);
extern int __sync_fetch_and_xor (volatile int*, int, ...);
extern int __sync_fetch_and_nand (volatile int*, int, ...);
extern int __sync_add_and_fetch (volatile int*, int, ...);
extern int __sync_sub_and_fetch (volatile int*, int, ...);
extern int __sync_or_and_fetch (volatile int*, int, ...);
extern int __sync_and_and_fetch (volatile int*, int, ...);
extern int __sync_xor_and_fetch (volatile int*, int, ...);
extern int __sync_nand_and_fetch (volatile int*, int, ...);
extern int __sync_bool_compare_and_swap (volatile int *ptr, int, int, ...);
extern int __sync_val_compare_and_swap (volatile int *ptr, int, int, ...);
extern void __sync_synchronize ();
extern int __sync_lock_test_and_set (volatile int*, int, ...);
extern void __sync_lock_release (volatile int *ptr, ...);

#endif //__GNUC__ >= 4

#if __COVERITY_GCC_VERSION_AT_LEAST(5, 0)
extern int __builtin_add_overflow(int, int, int *);
extern int __builtin_sub_overflow(int, int, int *);
extern int __builtin_mul_overflow(int, int, int *);
#endif // GCC 5

#endif // __GNUC__


/* Copyright (c) 2017 Synopsys, Inc. All rights reserved worldwide. */
#ifdef __ARM_NEON__
typedef int __builtin_neon_qi __attribute__((mode(QI)));
typedef int __builtin_neon_hi __attribute__((mode(HI)));
typedef int __builtin_neon_si __attribute__((mode(SI)));
typedef int __builtin_neon_di __attribute__((mode(DI)));
typedef int __builtin_neon_ti __attribute__((mode(TI)));
typedef unsigned int __builtin_neon_uqi __attribute__((mode(QI)));
typedef unsigned int __builtin_neon_uhi __attribute__((mode(HI)));
typedef unsigned int __builtin_neon_usi __attribute__((mode(SI)));
typedef unsigned int __builtin_neon_udi __attribute__((mode(DI)));
typedef int __builtin_neon_ei __attribute__ ((__vector_size__(32)));
typedef int __builtin_neon_oi __attribute__ ((__vector_size__(32)));
typedef int __builtin_neon_ci __attribute__ ((__vector_size__(64)));
typedef int __builtin_neon_xi __attribute__ ((__vector_size__(64)));
typedef int __builtin_neon_poly8 __attribute__((mode(QI)));
typedef int __builtin_neon_poly16 __attribute__((mode(HI)));
typedef float __builtin_neon_sf;
typedef float __builtin_neon_hf;
typedef int __builtin_neon_poly64;
typedef int __builtin_neon_poly128;

typedef int __simd64_int8_t __attribute__((__vector_size__(64)));
typedef int __simd64_int16_t __attribute__((__vector_size__(64)));
typedef int __simd64_int32_t __attribute__((__vector_size__(64)));
typedef unsigned int __simd64_uint8_t __attribute__((__vector_size__(64)));
typedef unsigned int __simd64_uint16_t __attribute__((__vector_size__(64)));
typedef unsigned int __simd64_uint32_t __attribute__((__vector_size__(64)));
typedef float __simd64_float16_t __attribute__((__vector_size__(64)));
typedef float __simd64_float32_t __attribute__((__vector_size__(64)));
typedef float __simd128_float32_t __attribute__((__vector_size__(128)));
typedef int __simd64_poly8_t __attribute__((__vector_size__(64)));
typedef int __simd64_poly16_t __attribute__((__vector_size__(64)));
typedef int __simd128_poly8_t __attribute__((__vector_size__(128)));
typedef int __simd128_poly16_t __attribute__((__vector_size__(128)));
typedef int __simd128_int8_t __attribute__((__vector_size__(128)));
typedef int __simd128_int16_t __attribute__((__vector_size__(128)));
typedef int __simd128_int32_t __attribute__((__vector_size__(128)));
typedef int __simd128_int64_t __attribute__((__vector_size__(128)));
typedef unsigned int __simd128_uint8_t __attribute__((__vector_size__(128)));
typedef unsigned int __simd128_uint16_t __attribute__((__vector_size__(128)));
typedef unsigned int __simd128_uint32_t __attribute__((__vector_size__(128)));
typedef unsigned int __simd128_uint64_t __attribute__((__vector_size__(128)));

typedef int cov_simd_int64_t __attribute__((__vector_size__(64)));
typedef int cov_simd_uint64_t __attribute__((__vector_size__(64)));
typedef int cov_simd_int128_t __attribute__((__vector_size__(128)));
typedef int cov_simd_uint128_t __attribute__((__vector_size__(128)));
typedef float cov_simd_float64_t __attribute__((__vector_size__(64)));
typedef float cov_simd_float128_t __attribute__((__vector_size__(128)));

cov_simd_float64_t  __builtin_neon_vaddv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_float128_t __builtin_neon_vaddv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_int128_t __builtin_neon_vaddlsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vaddlsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vaddlsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint128_t __builtin_neon_vaddluv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vaddluv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vaddluv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_int128_t __builtin_neon_vaddwsv8qi (cov_simd_int128_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vaddwsv4hi (cov_simd_int128_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vaddwsv2si (cov_simd_int128_t __a, cov_simd_int64_t __b);
cov_simd_uint128_t __builtin_neon_vaddwuv8qi (cov_simd_int128_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vaddwuv4hi (cov_simd_int128_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vaddwuv2si (cov_simd_int128_t  __a, cov_simd_int64_t  __b);
cov_simd_int64_t __builtin_neon_vhaddsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vhaddsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vhaddsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vhadduv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vhadduv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vhadduv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_int128_t __builtin_neon_vhaddsv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vhaddsv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vhaddsv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vhadduv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vhadduv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vhadduv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_int64_t __builtin_neon_vrhaddsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vrhaddsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vrhaddsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vrhadduv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vrhadduv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vrhadduv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_int128_t __builtin_neon_vrhaddsv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vrhaddsv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vrhaddsv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vrhadduv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vrhadduv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vrhadduv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_int64_t __builtin_neon_vqaddsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vqaddsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vqaddsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
int __builtin_neon_vqaddsdi (int __a, int __b);
cov_simd_uint64_t __builtin_neon_vqadduv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vqadduv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vqadduv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
unsigned int __builtin_neon_vqaddudi (int  __a, int  __b);
cov_simd_int128_t __builtin_neon_vqaddsv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vqaddsv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vqaddsv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vqaddsv2di (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vqadduv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vqadduv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vqadduv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vqadduv2di (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_int64_t __builtin_neon_vaddhnv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int64_t __builtin_neon_vaddhnv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int64_t __builtin_neon_vaddhnv2di (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint64_t __builtin_neon_vaddhnv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint64_t __builtin_neon_vaddhnv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint64_t __builtin_neon_vaddhnv2di (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_int64_t __builtin_neon_vraddhnv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int64_t __builtin_neon_vraddhnv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int64_t __builtin_neon_vraddhnv2di (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint64_t __builtin_neon_vraddhnv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint64_t __builtin_neon_vraddhnv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint64_t __builtin_neon_vraddhnv2di (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_float64_t  __builtin_neon_vmulfv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_float128_t __builtin_neon_vmulfv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_int64_t __builtin_neon_vmulpv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_int128_t __builtin_neon_vmulpv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_int64_t __builtin_neon_vqdmulhv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vqdmulhv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vqdmulhv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vqdmulhv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int64_t __builtin_neon_vqrdmulhv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vqrdmulhv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vqrdmulhv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vqrdmulhv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vmullsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vmullsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vmullsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint128_t __builtin_neon_vmulluv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vmulluv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vmulluv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_int128_t __builtin_neon_vmullpv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_int128_t __builtin_neon_vqdmullv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vqdmullv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vmlav8qi (cov_simd_int64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int64_t __builtin_neon_vmlav4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int64_t __builtin_neon_vmlav2si (cov_simd_int64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_float64_t __builtin_neon_vmlav2sf (cov_simd_float64_t __a, cov_simd_float64_t __b, cov_simd_float64_t __c);
cov_simd_uint64_t __builtin_neon_vmlav8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_uint64_t __builtin_neon_vmlav4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_uint64_t __builtin_neon_vmlav2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_int128_t __builtin_neon_vmlav16qi (cov_simd_int128_t __a, cov_simd_int128_t __b, cov_simd_int128_t __c);
cov_simd_int128_t __builtin_neon_vmlav8hi (cov_simd_int128_t __a, cov_simd_int128_t __b, cov_simd_int128_t __c);
cov_simd_int128_t __builtin_neon_vmlav4si (cov_simd_int128_t __a, cov_simd_int128_t __b, cov_simd_int128_t __c);
cov_simd_float128_t __builtin_neon_vmlav4sf (cov_simd_float128_t __a, cov_simd_float128_t __b, cov_simd_float128_t __c);
cov_simd_uint128_t __builtin_neon_vmlav16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b, cov_simd_int128_t  __c);
cov_simd_uint128_t __builtin_neon_vmlav8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b, cov_simd_int128_t  __c);
cov_simd_uint128_t __builtin_neon_vmlav4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b, cov_simd_int128_t  __c);
cov_simd_int128_t __builtin_neon_vmlalsv8qi (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int128_t __builtin_neon_vmlalsv4hi (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int128_t __builtin_neon_vmlalsv2si (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_uint128_t __builtin_neon_vmlaluv8qi (cov_simd_int128_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_uint128_t __builtin_neon_vmlaluv4hi (cov_simd_int128_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_uint128_t __builtin_neon_vmlaluv2si (cov_simd_int128_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_int128_t __builtin_neon_vqdmlalv4hi (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int128_t __builtin_neon_vqdmlalv2si (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int64_t __builtin_neon_vmlsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int64_t __builtin_neon_vmlsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int64_t __builtin_neon_vmlsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_float64_t __builtin_neon_vmlsv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b, cov_simd_float64_t __c);
cov_simd_uint64_t __builtin_neon_vmlsv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_uint64_t __builtin_neon_vmlsv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_uint64_t __builtin_neon_vmlsv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_int128_t __builtin_neon_vmlsv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b, cov_simd_int128_t __c);
cov_simd_int128_t __builtin_neon_vmlsv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b, cov_simd_int128_t __c);
cov_simd_int128_t __builtin_neon_vmlsv4si (cov_simd_int128_t __a, cov_simd_int128_t __b, cov_simd_int128_t __c);
cov_simd_float128_t __builtin_neon_vmlsv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b, cov_simd_float128_t __c);
cov_simd_uint128_t __builtin_neon_vmlsv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b, cov_simd_int128_t  __c);
cov_simd_uint128_t __builtin_neon_vmlsv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b, cov_simd_int128_t  __c);
cov_simd_uint128_t __builtin_neon_vmlsv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b, cov_simd_int128_t  __c);
cov_simd_int128_t __builtin_neon_vmlslsv8qi (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int128_t __builtin_neon_vmlslsv4hi (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int128_t __builtin_neon_vmlslsv2si (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_uint128_t __builtin_neon_vmlsluv8qi (cov_simd_int128_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_uint128_t __builtin_neon_vmlsluv4hi (cov_simd_int128_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_uint128_t __builtin_neon_vmlsluv2si (cov_simd_int128_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_int128_t __builtin_neon_vqdmlslv4hi (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int128_t __builtin_neon_vqdmlslv2si (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_float64_t __builtin_neon_vfmav2sf (cov_simd_float64_t __a, cov_simd_float64_t __b, cov_simd_float64_t __c);
cov_simd_float128_t __builtin_neon_vfmav4sf (cov_simd_float128_t __a, cov_simd_float128_t __b, cov_simd_float128_t __c);
cov_simd_float64_t __builtin_neon_vfmsv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b, cov_simd_float64_t __c);
cov_simd_float128_t __builtin_neon_vfmsv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b, cov_simd_float128_t __c);
cov_simd_float64_t __builtin_neon_vrintnv2sf (cov_simd_float64_t __a);
cov_simd_float128_t __builtin_neon_vrintnv4sf (cov_simd_float128_t __a);
cov_simd_float64_t __builtin_neon_vrintav2sf (cov_simd_float64_t __a);
cov_simd_float128_t __builtin_neon_vrintav4sf (cov_simd_float128_t __a);
cov_simd_float64_t __builtin_neon_vrintpv2sf (cov_simd_float64_t __a);
cov_simd_float128_t __builtin_neon_vrintpv4sf (cov_simd_float128_t __a);
cov_simd_float64_t __builtin_neon_vrintmv2sf (cov_simd_float64_t __a);
cov_simd_float128_t __builtin_neon_vrintmv4sf (cov_simd_float128_t __a);
cov_simd_float64_t __builtin_neon_vrintxv2sf (cov_simd_float64_t __a);
cov_simd_float128_t __builtin_neon_vrintxv4sf (cov_simd_float128_t __a);
cov_simd_float64_t __builtin_neon_vrintzv2sf (cov_simd_float64_t __a);
cov_simd_float128_t __builtin_neon_vrintzv4sf (cov_simd_float128_t __a);
cov_simd_float64_t  __builtin_neon_vsubv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_float128_t __builtin_neon_vsubv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_int128_t __builtin_neon_vsublsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vsublsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vsublsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint128_t __builtin_neon_vsubluv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vsubluv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vsubluv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_int128_t __builtin_neon_vsubwsv8qi (cov_simd_int128_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vsubwsv4hi (cov_simd_int128_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vsubwsv2si (cov_simd_int128_t __a, cov_simd_int64_t __b);
cov_simd_uint128_t __builtin_neon_vsubwuv8qi (cov_simd_int128_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vsubwuv4hi (cov_simd_int128_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vsubwuv2si (cov_simd_int128_t  __a, cov_simd_int64_t  __b);
cov_simd_int64_t __builtin_neon_vhsubsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vhsubsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vhsubsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vhsubuv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vhsubuv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vhsubuv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_int128_t __builtin_neon_vhsubsv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vhsubsv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vhsubsv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vhsubuv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vhsubuv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vhsubuv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_int64_t __builtin_neon_vqsubsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vqsubsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vqsubsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
int __builtin_neon_vqsubsdi (int __a, int __b);
cov_simd_uint64_t __builtin_neon_vqsubuv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vqsubuv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vqsubuv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
int __builtin_neon_vqsubudi (int  __a, int  __b);
cov_simd_int128_t __builtin_neon_vqsubsv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vqsubsv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vqsubsv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vqsubsv2di (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vqsubuv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vqsubuv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vqsubuv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vqsubuv2di (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_int64_t __builtin_neon_vsubhnv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int64_t __builtin_neon_vsubhnv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int64_t __builtin_neon_vsubhnv2di (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint64_t __builtin_neon_vsubhnv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint64_t __builtin_neon_vsubhnv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint64_t __builtin_neon_vsubhnv2di (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_int64_t __builtin_neon_vrsubhnv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int64_t __builtin_neon_vrsubhnv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int64_t __builtin_neon_vrsubhnv2di (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint64_t __builtin_neon_vrsubhnv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint64_t __builtin_neon_vrsubhnv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint64_t __builtin_neon_vrsubhnv2di (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint64_t __builtin_neon_vceqv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vceqv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vceqv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vceqv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_uint64_t __builtin_neon_vceqv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vceqv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vceqv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vceqv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vceqv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vceqv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vceqv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vceqv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_uint128_t __builtin_neon_vceqv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vceqv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vceqv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vceqv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint64_t __builtin_neon_vcgev8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vcgev4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vcgev2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vcgev2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_uint64_t __builtin_neon_vcgeuv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vcgeuv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vcgeuv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vcgev16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vcgev8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vcgev4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vcgev4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_uint128_t __builtin_neon_vcgeuv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vcgeuv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vcgeuv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint64_t __builtin_neon_vcgev8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vcgev4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vcgev2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vcgev2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_uint64_t __builtin_neon_vcgeuv8qi (cov_simd_int64_t  __b, cov_simd_int64_t  __a);
cov_simd_uint64_t __builtin_neon_vcgeuv4hi (cov_simd_int64_t  __b, cov_simd_int64_t  __a);
cov_simd_uint64_t __builtin_neon_vcgeuv2si (cov_simd_int64_t  __b, cov_simd_int64_t  __a);
cov_simd_uint128_t __builtin_neon_vcgev16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vcgev8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vcgev4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vcgev4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_uint128_t __builtin_neon_vcgeuv16qi (cov_simd_int128_t  __b, cov_simd_int128_t  __a);
cov_simd_uint128_t __builtin_neon_vcgeuv8hi (cov_simd_int128_t  __b, cov_simd_int128_t  __a);
cov_simd_uint128_t __builtin_neon_vcgeuv4si (cov_simd_int128_t  __b, cov_simd_int128_t  __a);
cov_simd_uint64_t __builtin_neon_vcgtv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vcgtv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vcgtv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vcgtv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_uint64_t __builtin_neon_vcgtuv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vcgtuv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vcgtuv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vcgtv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vcgtv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vcgtv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vcgtv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_uint128_t __builtin_neon_vcgtuv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vcgtuv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vcgtuv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint64_t __builtin_neon_vcgtv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vcgtv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vcgtv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vcgtv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_uint64_t __builtin_neon_vcgtuv8qi (cov_simd_int64_t  __b, cov_simd_int64_t  __a);
cov_simd_uint64_t __builtin_neon_vcgtuv4hi (cov_simd_int64_t  __b, cov_simd_int64_t  __a);
cov_simd_uint64_t __builtin_neon_vcgtuv2si (cov_simd_int64_t  __b, cov_simd_int64_t  __a);
cov_simd_uint128_t __builtin_neon_vcgtv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vcgtv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vcgtv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vcgtv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_uint128_t __builtin_neon_vcgtuv16qi (cov_simd_int128_t  __b, cov_simd_int128_t  __a);
cov_simd_uint128_t __builtin_neon_vcgtuv8hi (cov_simd_int128_t  __b, cov_simd_int128_t  __a);
cov_simd_uint128_t __builtin_neon_vcgtuv4si (cov_simd_int128_t  __b, cov_simd_int128_t  __a);
cov_simd_uint64_t __builtin_neon_vcagev2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_uint128_t __builtin_neon_vcagev4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_uint64_t __builtin_neon_vcagev2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_uint128_t __builtin_neon_vcagev4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_uint64_t __builtin_neon_vcagtv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_uint128_t __builtin_neon_vcagtv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_uint64_t __builtin_neon_vcagtv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_uint128_t __builtin_neon_vcagtv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_uint64_t __builtin_neon_vtstv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vtstv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vtstv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vtstv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vtstv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vtstv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vtstv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vtstv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vtstv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vtstv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vtstv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vtstv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vtstv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vtstv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_int64_t __builtin_neon_vabdsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vabdsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vabdsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_float64_t __builtin_neon_vabdfv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_uint64_t __builtin_neon_vabduv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vabduv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vabduv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_int128_t __builtin_neon_vabdsv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vabdsv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vabdsv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_float128_t __builtin_neon_vabdfv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_uint128_t __builtin_neon_vabduv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vabduv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vabduv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_int128_t __builtin_neon_vabdlsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vabdlsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vabdlsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint128_t __builtin_neon_vabdluv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vabdluv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vabdluv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_int64_t __builtin_neon_vabasv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int64_t __builtin_neon_vabasv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int64_t __builtin_neon_vabasv2si (cov_simd_int64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_uint64_t __builtin_neon_vabauv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_uint64_t __builtin_neon_vabauv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_uint64_t __builtin_neon_vabauv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_int128_t __builtin_neon_vabasv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b, cov_simd_int128_t __c);
cov_simd_int128_t __builtin_neon_vabasv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b, cov_simd_int128_t __c);
cov_simd_int128_t __builtin_neon_vabasv4si (cov_simd_int128_t __a, cov_simd_int128_t __b, cov_simd_int128_t __c);
cov_simd_uint128_t __builtin_neon_vabauv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b, cov_simd_int128_t  __c);
cov_simd_uint128_t __builtin_neon_vabauv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b, cov_simd_int128_t  __c);
cov_simd_uint128_t __builtin_neon_vabauv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b, cov_simd_int128_t  __c);
cov_simd_int128_t __builtin_neon_vabalsv8qi (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int128_t __builtin_neon_vabalsv4hi (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int128_t __builtin_neon_vabalsv2si (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_uint128_t __builtin_neon_vabaluv8qi (cov_simd_int128_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_uint128_t __builtin_neon_vabaluv4hi (cov_simd_int128_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_uint128_t __builtin_neon_vabaluv2si (cov_simd_int128_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_int64_t __builtin_neon_vmaxsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vmaxsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vmaxsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_float64_t __builtin_neon_vmaxfv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_uint64_t __builtin_neon_vmaxuv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vmaxuv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vmaxuv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_int128_t __builtin_neon_vmaxsv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vmaxsv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vmaxsv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_float128_t __builtin_neon_vmaxfv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_uint128_t __builtin_neon_vmaxuv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vmaxuv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vmaxuv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_int64_t __builtin_neon_vminsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vminsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vminsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_float64_t __builtin_neon_vminfv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_uint64_t __builtin_neon_vminuv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vminuv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vminuv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_int128_t __builtin_neon_vminsv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vminsv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vminsv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_float128_t __builtin_neon_vminfv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_uint128_t __builtin_neon_vminuv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vminuv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vminuv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_int64_t __builtin_neon_vpaddv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vpaddv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vpaddv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_float64_t __builtin_neon_vpaddv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_uint64_t __builtin_neon_vpaddv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vpaddv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vpaddv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_int64_t __builtin_neon_vpaddlsv8qi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vpaddlsv4hi (cov_simd_int64_t __a);
int __builtin_neon_vpaddlsv2si (cov_simd_int64_t __a);
cov_simd_uint64_t __builtin_neon_vpaddluv8qi (cov_simd_int64_t  __a);
cov_simd_uint64_t __builtin_neon_vpaddluv4hi (cov_simd_int64_t  __a);
unsigned int __builtin_neon_vpaddluv2si (cov_simd_int64_t  __a);
cov_simd_int128_t __builtin_neon_vpaddlsv16qi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vpaddlsv8hi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vpaddlsv4si (cov_simd_int128_t __a);
cov_simd_uint128_t __builtin_neon_vpaddluv16qi (cov_simd_int128_t  __a);
cov_simd_uint128_t __builtin_neon_vpaddluv8hi (cov_simd_int128_t  __a);
cov_simd_uint128_t __builtin_neon_vpaddluv4si (cov_simd_int128_t  __a);
cov_simd_int64_t __builtin_neon_vpadalsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vpadalsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
int __builtin_neon_vpadalsv2si (int __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vpadaluv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vpadaluv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
unsigned int __builtin_neon_vpadaluv2si (int  __a, cov_simd_int64_t  __b);
cov_simd_int128_t __builtin_neon_vpadalsv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vpadalsv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vpadalsv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vpadaluv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vpadaluv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_uint128_t __builtin_neon_vpadaluv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b);
cov_simd_int64_t __builtin_neon_vpmaxsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vpmaxsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vpmaxsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_float64_t __builtin_neon_vpmaxfv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_uint64_t __builtin_neon_vpmaxuv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vpmaxuv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vpmaxuv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_int64_t __builtin_neon_vpminsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vpminsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vpminsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_float64_t __builtin_neon_vpminfv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_uint64_t __builtin_neon_vpminuv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vpminuv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vpminuv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_float64_t __builtin_neon_vrecpsv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_float128_t __builtin_neon_vrecpsv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_float64_t __builtin_neon_vrsqrtsv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_float128_t __builtin_neon_vrsqrtsv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b);
cov_simd_int64_t __builtin_neon_vshlsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vshlsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vshlsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
int __builtin_neon_vshlsdi (int __a, int __b);
cov_simd_uint64_t __builtin_neon_vshluv8qi (cov_simd_uint64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vshluv4hi (cov_simd_uint64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vshluv2si (cov_simd_uint64_t __a, cov_simd_int64_t __b);
unsigned int __builtin_neon_vshludi (int  __a, int __b);
cov_simd_int128_t __builtin_neon_vshlsv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vshlsv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vshlsv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vshlsv2di (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vshluv16qi (cov_simd_uint128_t  __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vshluv8hi (cov_simd_uint128_t  __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vshluv4si (cov_simd_uint128_t  __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vshluv2di (cov_simd_uint128_t __a, cov_simd_int128_t __b);
cov_simd_int64_t __builtin_neon_vrshlsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vrshlsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vrshlsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
int __builtin_neon_vrshlsdi (int __a, int __b);
cov_simd_uint64_t __builtin_neon_vrshluv8qi (cov_simd_uint64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vrshluv4hi (cov_simd_uint64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vrshluv2si (cov_simd_uint64_t __a, cov_simd_int64_t __b);
unsigned int __builtin_neon_vrshludi (int  __a, int __b);
cov_simd_int128_t __builtin_neon_vrshlsv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vrshlsv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vrshlsv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vrshlsv2di (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vrshluv16qi (cov_simd_uint128_t  __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vrshluv8hi (cov_simd_uint128_t  __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vrshluv4si (cov_simd_uint128_t  __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vrshluv2di (cov_simd_uint128_t __a, cov_simd_int128_t __b);
cov_simd_int64_t __builtin_neon_vqshlsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vqshlsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vqshlsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
int __builtin_neon_vqshlsdi (int __a, int __b);
cov_simd_uint64_t __builtin_neon_vqshluv8qi (cov_simd_uint64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vqshluv4hi (cov_simd_uint64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vqshluv2si (cov_simd_uint64_t __a, cov_simd_int64_t __b);
unsigned int __builtin_neon_vqshludi (int  __a, int __b);
cov_simd_int128_t __builtin_neon_vqshlsv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vqshlsv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vqshlsv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vqshlsv2di (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vqshluv16qi (cov_simd_uint128_t  __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vqshluv8hi (cov_simd_uint128_t  __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vqshluv4si (cov_simd_uint128_t  __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vqshluv2di (cov_simd_uint128_t __a, cov_simd_int128_t __b);
cov_simd_int64_t __builtin_neon_vqrshlsv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vqrshlsv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int64_t __builtin_neon_vqrshlsv2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
int __builtin_neon_vqrshlsdi (int __a, int __b);
cov_simd_uint64_t __builtin_neon_vqrshluv8qi (cov_simd_uint64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vqrshluv4hi (cov_simd_uint64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vqrshluv2si (cov_simd_uint64_t __a, cov_simd_int64_t __b);
unsigned int __builtin_neon_vqrshludi (int  __a, int __b);
cov_simd_int128_t __builtin_neon_vqrshlsv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vqrshlsv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vqrshlsv4si (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_int128_t __builtin_neon_vqrshlsv2di (cov_simd_int128_t __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vqrshluv16qi (cov_simd_uint128_t  __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vqrshluv8hi (cov_simd_uint128_t  __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vqrshluv4si (cov_simd_uint128_t  __a, cov_simd_int128_t __b);
cov_simd_uint128_t __builtin_neon_vqrshluv2di (cov_simd_uint128_t __a, cov_simd_int128_t __b);
cov_simd_int64_t __builtin_neon_vshrs_nv8qi (cov_simd_int64_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vshrs_nv4hi (cov_simd_int64_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vshrs_nv2si (cov_simd_int64_t __a, const int __b);
int __builtin_neon_vshrs_ndi (int __a, const int __b);
cov_simd_uint64_t __builtin_neon_vshru_nv8qi (cov_simd_uint64_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vshru_nv4hi (cov_simd_uint64_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vshru_nv2si (cov_simd_uint64_t __a, const int __b);
unsigned int __builtin_neon_vshru_ndi (int  __a, const int __b);
cov_simd_int128_t __builtin_neon_vshrs_nv16qi (cov_simd_int128_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vshrs_nv8hi (cov_simd_int128_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vshrs_nv4si (cov_simd_int128_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vshrs_nv2di (cov_simd_int128_t __a, const int __b);
cov_simd_uint128_t __builtin_neon_vshru_nv16qi (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint128_t __builtin_neon_vshru_nv8hi (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint128_t __builtin_neon_vshru_nv4si (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint128_t __builtin_neon_vshru_nv2di (cov_simd_uint128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vrshrs_nv8qi (cov_simd_int64_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vrshrs_nv4hi (cov_simd_int64_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vrshrs_nv2si (cov_simd_int64_t __a, const int __b);
int __builtin_neon_vrshrs_ndi (int __a, const int __b);
cov_simd_uint64_t __builtin_neon_vrshru_nv8qi (cov_simd_uint64_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vrshru_nv4hi (cov_simd_uint64_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vrshru_nv2si (cov_simd_uint64_t __a, const int __b);
unsigned int __builtin_neon_vrshru_ndi (int  __a, const int __b);
cov_simd_int128_t __builtin_neon_vrshrs_nv16qi (cov_simd_int128_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vrshrs_nv8hi (cov_simd_int128_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vrshrs_nv4si (cov_simd_int128_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vrshrs_nv2di (cov_simd_int128_t __a, const int __b);
cov_simd_uint128_t __builtin_neon_vrshru_nv16qi (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint128_t __builtin_neon_vrshru_nv8hi (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint128_t __builtin_neon_vrshru_nv4si (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint128_t __builtin_neon_vrshru_nv2di (cov_simd_uint128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vshrn_nv8hi (cov_simd_int128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vshrn_nv4si (cov_simd_int128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vshrn_nv2di (cov_simd_int128_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vshrn_nv8hi (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint64_t __builtin_neon_vshrn_nv4si (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint64_t __builtin_neon_vshrn_nv2di (cov_simd_uint128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vrshrn_nv8hi (cov_simd_int128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vrshrn_nv4si (cov_simd_int128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vrshrn_nv2di (cov_simd_int128_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vrshrn_nv8hi (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint64_t __builtin_neon_vrshrn_nv4si (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint64_t __builtin_neon_vrshrn_nv2di (cov_simd_uint128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vqshrns_nv8hi (cov_simd_int128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vqshrns_nv4si (cov_simd_int128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vqshrns_nv2di (cov_simd_int128_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqshrnu_nv8hi (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqshrnu_nv4si (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqshrnu_nv2di (cov_simd_uint128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vqrshrns_nv8hi (cov_simd_int128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vqrshrns_nv4si (cov_simd_int128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vqrshrns_nv2di (cov_simd_int128_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqrshrnu_nv8hi (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqrshrnu_nv4si (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqrshrnu_nv2di (cov_simd_uint128_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqshrun_nv8hi (cov_simd_int128_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqshrun_nv4si (cov_simd_int128_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqshrun_nv2di (cov_simd_int128_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqrshrun_nv8hi (cov_simd_int128_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqrshrun_nv4si (cov_simd_int128_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqrshrun_nv2di (cov_simd_int128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vshl_nv8qi (cov_simd_int64_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vshl_nv4hi (cov_simd_int64_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vshl_nv2si (cov_simd_int64_t __a, const int __b);
int __builtin_neon_vshl_ndi (int __a, const int __b);
cov_simd_uint64_t __builtin_neon_vshl_nv8qi (cov_simd_uint64_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vshl_nv4hi (cov_simd_uint64_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vshl_nv2si (cov_simd_uint64_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vshl_nv16qi (cov_simd_int128_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vshl_nv8hi (cov_simd_int128_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vshl_nv4si (cov_simd_int128_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vshl_nv2di (cov_simd_int128_t __a, const int __b);
cov_simd_uint128_t __builtin_neon_vshl_nv16qi (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint128_t __builtin_neon_vshl_nv8hi (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint128_t __builtin_neon_vshl_nv4si (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint128_t __builtin_neon_vshl_nv2di (cov_simd_uint128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vqshl_s_nv8qi (cov_simd_int64_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vqshl_s_nv4hi (cov_simd_int64_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vqshl_s_nv2si (cov_simd_int64_t __a, const int __b);
int __builtin_neon_vqshl_s_ndi (int __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqshl_u_nv8qi (cov_simd_uint64_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqshl_u_nv4hi (cov_simd_uint64_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqshl_u_nv2si (cov_simd_uint64_t __a, const int __b);
unsigned int __builtin_neon_vqshl_u_ndi (int  __a, const int __b);
cov_simd_int128_t __builtin_neon_vqshl_s_nv16qi (cov_simd_int128_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vqshl_s_nv8hi (cov_simd_int128_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vqshl_s_nv4si (cov_simd_int128_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vqshl_s_nv2di (cov_simd_int128_t __a, const int __b);
cov_simd_uint128_t __builtin_neon_vqshl_u_nv16qi (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint128_t __builtin_neon_vqshl_u_nv8hi (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint128_t __builtin_neon_vqshl_u_nv4si (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint128_t __builtin_neon_vqshl_u_nv2di (cov_simd_uint128_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqshlu_nv8qi (cov_simd_int64_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqshlu_nv4hi (cov_simd_int64_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vqshlu_nv2si (cov_simd_int64_t __a, const int __b);
unsigned int __builtin_neon_vqshlu_ndi (int __a, const int __b);
cov_simd_uint128_t __builtin_neon_vqshlu_nv16qi (cov_simd_int128_t __a, const int __b);
cov_simd_uint128_t __builtin_neon_vqshlu_nv8hi (cov_simd_int128_t __a, const int __b);
cov_simd_uint128_t __builtin_neon_vqshlu_nv4si (cov_simd_int128_t __a, const int __b);
cov_simd_uint128_t __builtin_neon_vqshlu_nv2di (cov_simd_int128_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vshlls_nv8qi (cov_simd_int64_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vshlls_nv4hi (cov_simd_int64_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vshlls_nv2si (cov_simd_int64_t __a, const int __b);
cov_simd_uint128_t __builtin_neon_vshllu_nv8qi (cov_simd_uint64_t __a, const int __b);
cov_simd_uint128_t __builtin_neon_vshllu_nv4hi (cov_simd_uint64_t __a, const int __b);
cov_simd_uint128_t __builtin_neon_vshllu_nv2si (cov_simd_uint64_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vsras_nv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vsras_nv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vsras_nv2si (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
int __builtin_neon_vsras_ndi (int __a, int __b, const int __c);
cov_simd_uint64_t __builtin_neon_vsrau_nv8qi (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_uint64_t __builtin_neon_vsrau_nv4hi (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_uint64_t __builtin_neon_vsrau_nv2si (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
unsigned int __builtin_neon_vsrau_ndi (int  __a, int  __b, const int __c);
cov_simd_int128_t __builtin_neon_vsras_nv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vsras_nv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vsras_nv4si (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vsras_nv2di (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_uint128_t __builtin_neon_vsrau_nv16qi (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vsrau_nv8hi (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vsrau_nv4si (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vsrau_nv2di (cov_simd_uint128_t __a, cov_simd_uint128_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vrsras_nv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vrsras_nv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vrsras_nv2si (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
int __builtin_neon_vrsras_ndi (int __a, int __b, const int __c);
cov_simd_uint64_t __builtin_neon_vrsrau_nv8qi (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_uint64_t __builtin_neon_vrsrau_nv4hi (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_uint64_t __builtin_neon_vrsrau_nv2si (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
unsigned int __builtin_neon_vrsrau_ndi (int  __a, int  __b, const int __c);
cov_simd_int128_t __builtin_neon_vrsras_nv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vrsras_nv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vrsras_nv4si (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vrsras_nv2di (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_uint128_t __builtin_neon_vrsrau_nv16qi (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vrsrau_nv8hi (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vrsrau_nv4si (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vrsrau_nv2di (cov_simd_uint128_t __a, cov_simd_uint128_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vsri_nv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vsri_nv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vsri_nv2si (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
int __builtin_neon_vsri_ndi (int __a, int __b, const int __c);
cov_simd_uint64_t __builtin_neon_vsri_nv8qi (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_uint64_t __builtin_neon_vsri_nv4hi (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_uint64_t __builtin_neon_vsri_nv2si (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vsri_nv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vsri_nv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vsri_nv4si (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vsri_nv2di (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_uint128_t __builtin_neon_vsri_nv16qi (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vsri_nv8hi (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vsri_nv4si (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vsri_nv2di (cov_simd_uint128_t __a, cov_simd_uint128_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vsli_nv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vsli_nv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vsli_nv2si (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
int __builtin_neon_vsli_ndi (int __a, int __b, const int __c);
cov_simd_uint64_t __builtin_neon_vsli_nv8qi (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_uint64_t __builtin_neon_vsli_nv4hi (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_uint64_t __builtin_neon_vsli_nv2si (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vsli_nv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vsli_nv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vsli_nv4si (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vsli_nv2di (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_uint128_t __builtin_neon_vsli_nv16qi (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vsli_nv8hi (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vsli_nv4si (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vsli_nv2di (cov_simd_uint128_t __a, cov_simd_uint128_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vabsv8qi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vabsv4hi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vabsv2si (cov_simd_int64_t __a);
cov_simd_float64_t __builtin_neon_vabsv2sf (cov_simd_float64_t __a);
cov_simd_int128_t __builtin_neon_vabsv16qi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vabsv8hi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vabsv4si (cov_simd_int128_t __a);
cov_simd_float128_t __builtin_neon_vabsv4sf (cov_simd_float128_t __a);
cov_simd_int64_t __builtin_neon_vqabsv8qi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vqabsv4hi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vqabsv2si (cov_simd_int64_t __a);
cov_simd_int128_t __builtin_neon_vqabsv16qi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vqabsv8hi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vqabsv4si (cov_simd_int128_t __a);
cov_simd_int64_t __builtin_neon_vnegv8qi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vnegv4hi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vnegv2si (cov_simd_int64_t __a);
cov_simd_float64_t __builtin_neon_vnegv2sf (cov_simd_float64_t __a);
cov_simd_int128_t __builtin_neon_vnegv16qi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vnegv8hi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vnegv4si (cov_simd_int128_t __a);
cov_simd_float128_t __builtin_neon_vnegv4sf (cov_simd_float128_t __a);
cov_simd_int64_t __builtin_neon_vqnegv8qi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vqnegv4hi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vqnegv2si (cov_simd_int64_t __a);
cov_simd_int128_t __builtin_neon_vqnegv16qi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vqnegv8hi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vqnegv4si (cov_simd_int128_t __a);
cov_simd_int64_t __builtin_neon_vmvnv8qi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vmvnv4hi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vmvnv2si (cov_simd_int64_t __a);
cov_simd_uint64_t __builtin_neon_vmvnv8qi (cov_simd_int64_t  __a);
cov_simd_uint64_t __builtin_neon_vmvnv4hi (cov_simd_int64_t  __a);
cov_simd_uint64_t __builtin_neon_vmvnv2si (cov_simd_int64_t  __a);
cov_simd_int128_t __builtin_neon_vmvnv16qi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vmvnv8hi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vmvnv4si (cov_simd_int128_t __a);
cov_simd_uint128_t __builtin_neon_vmvnv16qi (cov_simd_int128_t  __a);
cov_simd_uint128_t __builtin_neon_vmvnv8hi (cov_simd_int128_t  __a);
cov_simd_uint128_t __builtin_neon_vmvnv4si (cov_simd_int128_t  __a);
cov_simd_int64_t __builtin_neon_vclsv8qi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vclsv4hi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vclsv2si (cov_simd_int64_t __a);
cov_simd_int128_t __builtin_neon_vclsv16qi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vclsv8hi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vclsv4si (cov_simd_int128_t __a);
cov_simd_int64_t __builtin_neon_vclzv8qi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vclzv4hi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vclzv2si (cov_simd_int64_t __a);
cov_simd_uint64_t __builtin_neon_vclzv8qi (cov_simd_int64_t  __a);
cov_simd_uint64_t __builtin_neon_vclzv4hi (cov_simd_int64_t  __a);
cov_simd_uint64_t __builtin_neon_vclzv2si (cov_simd_int64_t  __a);
cov_simd_int128_t __builtin_neon_vclzv16qi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vclzv8hi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vclzv4si (cov_simd_int128_t __a);
cov_simd_uint128_t __builtin_neon_vclzv16qi (cov_simd_int128_t  __a);
cov_simd_uint128_t __builtin_neon_vclzv8hi (cov_simd_int128_t  __a);
cov_simd_uint128_t __builtin_neon_vclzv4si (cov_simd_int128_t  __a);
cov_simd_int64_t __builtin_neon_vcntv8qi (cov_simd_int64_t __a);
cov_simd_uint64_t __builtin_neon_vcntv8qi (cov_simd_int64_t  __a);
cov_simd_int128_t __builtin_neon_vcntv16qi (cov_simd_int128_t __a);
cov_simd_uint128_t __builtin_neon_vcntv16qi (cov_simd_int128_t  __a);
cov_simd_float64_t __builtin_neon_vrecpev2sf (cov_simd_float64_t __a);
cov_simd_uint64_t __builtin_neon_vrecpev2si (cov_simd_int64_t  __a);
cov_simd_float128_t __builtin_neon_vrecpev4sf (cov_simd_float128_t __a);
cov_simd_uint128_t __builtin_neon_vrecpev4si (cov_simd_int128_t  __a);
cov_simd_float64_t __builtin_neon_vrsqrtev2sf (cov_simd_float64_t __a);
cov_simd_uint64_t __builtin_neon_vrsqrtev2si (cov_simd_int64_t  __a);
cov_simd_float128_t __builtin_neon_vrsqrtev4sf (cov_simd_float128_t __a);
cov_simd_uint128_t __builtin_neon_vrsqrtev4si (cov_simd_int128_t  __a);
int __builtin_neon_vget_lanev8qi (cov_simd_int64_t __a, const int __b);
int __builtin_neon_vget_lanev4hi (cov_simd_int64_t __a, const int __b);
int __builtin_neon_vget_lanev2si (cov_simd_int64_t __a, const int __b);
float __builtin_neon_vget_lanev2sf (cov_simd_float64_t __a, const int __b);
unsigned int __builtin_neon_vget_laneuv8qi (cov_simd_uint64_t __a, const int __b);
unsigned int __builtin_neon_vget_laneuv4hi (cov_simd_uint64_t __a, const int __b);
unsigned int __builtin_neon_vget_laneuv2si (cov_simd_uint64_t __a, const int __b);
int __builtin_neon_vget_lanedi (int __a, const int __b);
int __builtin_neon_vget_lanev16qi (cov_simd_int128_t __a, const int __b);
int __builtin_neon_vget_lanev8hi (cov_simd_int128_t __a, const int __b);
int __builtin_neon_vget_lanev4si (cov_simd_int128_t __a, const int __b);
float __builtin_neon_vget_lanev4sf (cov_simd_float128_t __a, const int __b);
unsigned int __builtin_neon_vget_laneuv16qi (cov_simd_uint128_t  __a, const int __b);
unsigned int __builtin_neon_vget_laneuv8hi (cov_simd_uint128_t  __a, const int __b);
unsigned int __builtin_neon_vget_laneuv4si (cov_simd_uint128_t  __a, const int __b);
int __builtin_neon_vget_lanev2di (cov_simd_int128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vset_lanev8qi (int __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vset_lanev4hi (int __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vset_lanev2si (int __a, cov_simd_int64_t __b, const int __c);
cov_simd_float64_t __builtin_neon_vset_lanev2sf (float __a, cov_simd_float64_t __b, const int __c);
cov_simd_uint64_t __builtin_neon_vset_lanev8qi (unsigned int __a, cov_simd_uint64_t __b, const int __c);
cov_simd_uint64_t __builtin_neon_vset_lanev4hi (unsigned int __a, cov_simd_uint64_t __b, const int __c);
cov_simd_uint64_t __builtin_neon_vset_lanev2si (unsigned int __a, cov_simd_uint64_t __b, const int __c);
int __builtin_neon_vset_lanedi (__builtin_neon_di __a, int __b, const int __c);
cov_simd_int128_t __builtin_neon_vset_lanev16qi (int __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vset_lanev8hi (int __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vset_lanev4si (int __a, cov_simd_int128_t __b, const int __c);
cov_simd_float128_t __builtin_neon_vset_lanev4sf (float __a, cov_simd_float128_t __b, const int __c);
cov_simd_uint128_t __builtin_neon_vset_lanev16qi (unsigned int __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vset_lanev8hi (unsigned int __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vset_lanev4si (unsigned int __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_int128_t __builtin_neon_vset_lanev2di (int __a, cov_simd_int128_t __b, const int __c);
cov_simd_uint128_t __builtin_neon_vset_lanev2di (unsigned int__a, cov_simd_uint128_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vcreatev8qi (__builtin_neon_di __a);
cov_simd_int64_t __builtin_neon_vcreatev4hi (__builtin_neon_di __a);
cov_simd_int64_t __builtin_neon_vcreatev2si (__builtin_neon_di __a);
int __builtin_neon_vcreatedi (__builtin_neon_di __a);
cov_simd_float64_t __builtin_neon_vcreatev2sf (__builtin_neon_di __a);
cov_simd_uint64_t __builtin_neon_vcreatev8qi (__builtin_neon_di __a);
cov_simd_uint64_t __builtin_neon_vcreatev4hi (__builtin_neon_di __a);
cov_simd_uint64_t __builtin_neon_vcreatev2si (__builtin_neon_di __a);
cov_simd_int64_t __builtin_neon_vdup_nv8qi (__builtin_neon_qi  __a);
cov_simd_int64_t __builtin_neon_vdup_nv4hi (__builtin_neon_hi  __a);
cov_simd_int64_t __builtin_neon_vdup_nv2si (__builtin_neon_si  __a);
cov_simd_float64_t __builtin_neon_vdup_nv2sf (__builtin_neon_sf  __a);
cov_simd_uint64_t __builtin_neon_vdup_nv8qi (__builtin_neon_qi  __a);
cov_simd_uint64_t __builtin_neon_vdup_nv4hi (__builtin_neon_hi  __a);
cov_simd_uint64_t __builtin_neon_vdup_nv2si (__builtin_neon_si  __a);
int __builtin_neon_vdup_ndi (__builtin_neon_di __a);
cov_simd_int128_t __builtin_neon_vdup_nv16qi (__builtin_neon_qi  __a);
cov_simd_int128_t __builtin_neon_vdup_nv8hi (__builtin_neon_hi  __a);
cov_simd_int128_t __builtin_neon_vdup_nv4si (__builtin_neon_si  __a);
cov_simd_float128_t __builtin_neon_vdup_nv4sf (__builtin_neon_sf  __a);
cov_simd_uint128_t __builtin_neon_vdup_nv16qi (__builtin_neon_qi  __a);
cov_simd_uint128_t __builtin_neon_vdup_nv8hi (__builtin_neon_hi  __a);
cov_simd_uint128_t __builtin_neon_vdup_nv4si (__builtin_neon_si  __a);
cov_simd_int128_t __builtin_neon_vdup_nv2di (__builtin_neon_di __a);
cov_simd_uint128_t __builtin_neon_vdup_nv2di (__builtin_neon_di __a);
cov_simd_int64_t __builtin_neon_vdup_nv8qi (__builtin_neon_qi  __a);
cov_simd_int64_t __builtin_neon_vdup_nv4hi (__builtin_neon_hi  __a);
cov_simd_int64_t __builtin_neon_vdup_nv2si (__builtin_neon_si  __a);
cov_simd_float64_t __builtin_neon_vdup_nv2sf (__builtin_neon_sf  __a);
cov_simd_uint64_t __builtin_neon_vdup_nv8qi (__builtin_neon_qi  __a);
cov_simd_uint64_t __builtin_neon_vdup_nv4hi (__builtin_neon_hi  __a);
cov_simd_uint64_t __builtin_neon_vdup_nv2si (__builtin_neon_si  __a);
cov_simd_int128_t __builtin_neon_vdup_nv16qi (__builtin_neon_qi  __a);
cov_simd_int128_t __builtin_neon_vdup_nv8hi (__builtin_neon_hi  __a);
cov_simd_int128_t __builtin_neon_vdup_nv4si (__builtin_neon_si  __a);
cov_simd_float128_t __builtin_neon_vdup_nv4sf (__builtin_neon_sf  __a);
cov_simd_uint128_t __builtin_neon_vdup_nv16qi (__builtin_neon_qi  __a);
cov_simd_uint128_t __builtin_neon_vdup_nv8hi (__builtin_neon_hi  __a);
cov_simd_uint128_t __builtin_neon_vdup_nv4si (__builtin_neon_si  __a);
cov_simd_int128_t __builtin_neon_vdup_nv2di (__builtin_neon_di __a);
cov_simd_uint128_t __builtin_neon_vdup_nv2di (__builtin_neon_di __a);
cov_simd_int64_t __builtin_neon_vdup_lanev8qi (cov_simd_int64_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vdup_lanev4hi (cov_simd_int64_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vdup_lanev2si (cov_simd_int64_t __a, const int __b);
cov_simd_float64_t __builtin_neon_vdup_lanev2sf (cov_simd_float64_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vdup_lanev8qi (cov_simd_uint64_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vdup_lanev4hi (cov_simd_uint64_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vdup_lanev2si (cov_simd_uint64_t __a, const int __b);
int __builtin_neon_vdup_lanedi (int __a, const int __b);
cov_simd_int128_t __builtin_neon_vdup_lanev16qi (cov_simd_int64_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vdup_lanev8hi (cov_simd_int64_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vdup_lanev4si (cov_simd_int64_t __a, const int __b);
cov_simd_float128_t __builtin_neon_vdup_lanev4sf (cov_simd_float64_t __a, const int __b);
cov_simd_uint128_t __builtin_neon_vdup_lanev16qi (cov_simd_uint64_t __a, const int __b);
cov_simd_uint128_t __builtin_neon_vdup_lanev8hi (cov_simd_uint64_t __a, const int __b);
cov_simd_uint128_t __builtin_neon_vdup_lanev4si (cov_simd_uint64_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vdup_lanev2di (int __a, const int __b);
cov_simd_int128_t __builtin_neon_vcombinev8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vcombinev4hi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vcombinev2si (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_int128_t __builtin_neon_vcombinedi (int __a, int __b);
cov_simd_float128_t __builtin_neon_vcombinev2sf (cov_simd_float64_t __a, cov_simd_float64_t __b);
cov_simd_uint128_t __builtin_neon_vcombinev8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vcombinev4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint128_t __builtin_neon_vcombinev2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_int64_t __builtin_neon_vget_highv16qi (cov_simd_int128_t __a);
cov_simd_int64_t __builtin_neon_vget_highv8hi (cov_simd_int128_t __a);
cov_simd_int64_t __builtin_neon_vget_highv4si (cov_simd_int128_t __a);
int __builtin_neon_vget_highv2di (cov_simd_int128_t __a);
cov_simd_float64_t __builtin_neon_vget_highv4sf (cov_simd_float128_t __a);
cov_simd_uint64_t __builtin_neon_vget_highv16qi (cov_simd_int128_t  __a);
cov_simd_uint64_t __builtin_neon_vget_highv8hi (cov_simd_int128_t  __a);
cov_simd_uint64_t __builtin_neon_vget_highv4si (cov_simd_int128_t  __a);
cov_simd_int64_t __builtin_neon_vget_lowv16qi (cov_simd_int128_t __a);
cov_simd_int64_t __builtin_neon_vget_lowv8hi (cov_simd_int128_t __a);
cov_simd_int64_t __builtin_neon_vget_lowv4si (cov_simd_int128_t __a);
cov_simd_float64_t __builtin_neon_vget_lowv4sf (cov_simd_float128_t __a);
cov_simd_uint64_t __builtin_neon_vget_lowv16qi (cov_simd_int128_t  __a);
cov_simd_uint64_t __builtin_neon_vget_lowv8hi (cov_simd_int128_t  __a);
cov_simd_uint64_t __builtin_neon_vget_lowv4si (cov_simd_int128_t  __a);
int __builtin_neon_vget_lowv2di (cov_simd_int128_t __a);
cov_simd_int64_t __builtin_neon_vcvtsv2sf (cov_simd_float64_t __a);
cov_simd_float64_t __builtin_neon_vcvtsv2si (cov_simd_int64_t __a);
cov_simd_float64_t __builtin_neon_vcvtuv2si (cov_simd_int64_t  __a);
cov_simd_uint64_t __builtin_neon_vcvtuv2sf (cov_simd_float64_t __a);
cov_simd_int128_t __builtin_neon_vcvtsv4sf (cov_simd_float128_t __a);
cov_simd_float128_t __builtin_neon_vcvtsv4si (cov_simd_int128_t __a);
cov_simd_float128_t __builtin_neon_vcvtuv4si (cov_simd_int128_t  __a);
cov_simd_uint128_t __builtin_neon_vcvtuv4sf (cov_simd_float128_t __a);
cov_simd_float64_t __builtin_neon_vcvtv4hfv4sf (cov_simd_float128_t __a);
cov_simd_float128_t __builtin_neon_vcvtv4sfv4hf (cov_simd_float64_t  __a);
cov_simd_int64_t __builtin_neon_vcvts_nv2sf (cov_simd_float64_t __a, const int __b);
cov_simd_float64_t __builtin_neon_vcvts_nv2si (cov_simd_int64_t __a, const int __b);
cov_simd_float64_t __builtin_neon_vcvtu_nv2si (cov_simd_uint64_t __a, const int __b);
cov_simd_uint64_t __builtin_neon_vcvtu_nv2sf (cov_simd_float64_t __a, const int __b);
cov_simd_int128_t __builtin_neon_vcvts_nv4sf (cov_simd_float128_t __a, const int __b);
cov_simd_float128_t __builtin_neon_vcvts_nv4si (cov_simd_int128_t __a, const int __b);
cov_simd_float128_t __builtin_neon_vcvtu_nv4si (cov_simd_uint128_t  __a, const int __b);
cov_simd_uint128_t __builtin_neon_vcvtu_nv4sf (cov_simd_float128_t __a, const int __b);
cov_simd_int64_t __builtin_neon_vmovnv8hi (cov_simd_int128_t __a);
cov_simd_int64_t __builtin_neon_vmovnv4si (cov_simd_int128_t __a);
cov_simd_int64_t __builtin_neon_vmovnv2di (cov_simd_int128_t __a);
cov_simd_uint64_t __builtin_neon_vmovnv8hi (cov_simd_int128_t  __a);
cov_simd_uint64_t __builtin_neon_vmovnv4si (cov_simd_int128_t  __a);
cov_simd_uint64_t __builtin_neon_vmovnv2di (cov_simd_int128_t  __a);
cov_simd_int64_t __builtin_neon_vqmovnsv8hi (cov_simd_int128_t __a);
cov_simd_int64_t __builtin_neon_vqmovnsv4si (cov_simd_int128_t __a);
cov_simd_int64_t __builtin_neon_vqmovnsv2di (cov_simd_int128_t __a);
cov_simd_uint64_t __builtin_neon_vqmovnuv8hi (cov_simd_int128_t  __a);
cov_simd_uint64_t __builtin_neon_vqmovnuv4si (cov_simd_int128_t  __a);
cov_simd_uint64_t __builtin_neon_vqmovnuv2di (cov_simd_int128_t  __a);
cov_simd_uint64_t __builtin_neon_vqmovunv8hi (cov_simd_int128_t __a);
cov_simd_uint64_t __builtin_neon_vqmovunv4si (cov_simd_int128_t __a);
cov_simd_uint64_t __builtin_neon_vqmovunv2di (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vmovlsv8qi (cov_simd_int64_t __a);
cov_simd_int128_t __builtin_neon_vmovlsv4hi (cov_simd_int64_t __a);
cov_simd_int128_t __builtin_neon_vmovlsv2si (cov_simd_int64_t __a);
cov_simd_uint128_t __builtin_neon_vmovluv8qi (cov_simd_int64_t  __a);
cov_simd_uint128_t __builtin_neon_vmovluv4hi (cov_simd_int64_t  __a);
cov_simd_uint128_t __builtin_neon_vmovluv2si (cov_simd_int64_t  __a);
cov_simd_int64_t __builtin_neon_vtbl1v8qi (cov_simd_int64_t __a, cov_simd_int64_t __b);
cov_simd_uint64_t __builtin_neon_vtbl1v8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vtbl2v8qi (__builtin_neon_ti __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vtbl3v8qi (__builtin_neon_ei __a, cov_simd_int64_t  __b);
cov_simd_uint64_t __builtin_neon_vtbl4v8qi (__builtin_neon_oi __a, cov_simd_int64_t  __b);
cov_simd_int64_t __builtin_neon_vtbx1v8qi (cov_simd_int64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_uint64_t __builtin_neon_vtbx1v8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_int64_t __builtin_neon_vtbx2v8qi (cov_simd_int64_t __a, int __b, cov_simd_int64_t __c);
cov_simd_uint64_t __builtin_neon_vtbx2v8qi (cov_simd_uint64_t __a, unsigned int __b, cov_simd_uint64_t __c);
cov_simd_int64_t __builtin_neon_vtbx3v8qi (cov_simd_int64_t __a, __builtin_neon_ei __b, cov_simd_int64_t __c);
cov_simd_int64_t __builtin_neon_vtbx4v8qi (cov_simd_int64_t __a, __builtin_neon_oi __b, cov_simd_int64_t __c);
cov_simd_int64_t __builtin_neon_vmul_lanev4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vmul_lanev2si (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_float64_t __builtin_neon_vmul_lanev2sf (cov_simd_float64_t __a, cov_simd_float64_t __b, const int __c);
cov_simd_uint64_t __builtin_neon_vmul_lanev4hi (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_uint64_t __builtin_neon_vmul_lanev2si (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vmul_lanev8hi (cov_simd_int128_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vmul_lanev4si (cov_simd_int128_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_float128_t __builtin_neon_vmul_lanev4sf (cov_simd_float128_t __a, cov_simd_float64_t __b, const int __c);
cov_simd_uint128_t __builtin_neon_vmul_lanev8hi (cov_simd_uint128_t  __a, cov_simd_uint64_t __b, const int __c);
cov_simd_uint128_t __builtin_neon_vmul_lanev4si (cov_simd_uint128_t  __a, cov_simd_uint64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vmla_lanev4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c, const int __d);
cov_simd_int64_t __builtin_neon_vmla_lanev2si (cov_simd_int64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c, const int __d);
cov_simd_float64_t __builtin_neon_vmla_lanev2sf (cov_simd_float64_t __a, cov_simd_float64_t __b, cov_simd_float64_t __c, const int __d);
cov_simd_uint64_t __builtin_neon_vmla_lanev4hi (cov_simd_uint64_t __a, cov_simd_uint64_t __b, cov_simd_uint64_t __c, const int __d);
cov_simd_uint64_t __builtin_neon_vmla_lanev2si (cov_simd_uint64_t __a, cov_simd_uint64_t __b, cov_simd_uint64_t __c, const int __d);
cov_simd_int128_t __builtin_neon_vmla_lanev8hi (cov_simd_int128_t __a, cov_simd_int128_t __b, cov_simd_int64_t __c, const int __d);
cov_simd_int128_t __builtin_neon_vmla_lanev4si (cov_simd_int128_t __a, cov_simd_int128_t __b, cov_simd_int64_t __c, const int __d);
cov_simd_float128_t __builtin_neon_vmla_lanev4sf (cov_simd_float128_t __a, cov_simd_float128_t __b, cov_simd_float64_t __c, const int __d);
cov_simd_uint128_t __builtin_neon_vmla_lanev8hi (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, cov_simd_uint64_t __c, const int __d);
cov_simd_uint128_t __builtin_neon_vmla_lanev4si (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, cov_simd_uint64_t __c, const int __d);
cov_simd_int128_t __builtin_neon_vmlals_lanev4hi (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c, const int __d);
cov_simd_int128_t __builtin_neon_vmlals_lanev2si (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c, const int __d);
cov_simd_uint128_t __builtin_neon_vmlalu_lanev4hi (cov_simd_uint128_t  __a, cov_simd_uint64_t __b, cov_simd_uint64_t __c, const int __d);
cov_simd_uint128_t __builtin_neon_vmlalu_lanev2si (cov_simd_uint128_t __a, cov_simd_uint64_t __b, cov_simd_uint64_t __c, const int __d);
cov_simd_int128_t __builtin_neon_vqdmlal_lanev4hi (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c, const int __d);
cov_simd_int128_t __builtin_neon_vqdmlal_lanev2si (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c, const int __d);
cov_simd_int64_t __builtin_neon_vmls_lanev4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c, const int __d);
cov_simd_int64_t __builtin_neon_vmls_lanev2si (cov_simd_int64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c, const int __d);
cov_simd_float64_t __builtin_neon_vmls_lanev2sf (cov_simd_float64_t __a, cov_simd_float64_t __b, cov_simd_float64_t __c, const int __d);
cov_simd_uint64_t __builtin_neon_vmls_lanev4hi (cov_simd_uint64_t __a, cov_simd_uint64_t __b, cov_simd_uint64_t __c, const int __d);
cov_simd_uint64_t __builtin_neon_vmls_lanev2si (cov_simd_uint64_t __a, cov_simd_uint64_t __b, cov_simd_uint64_t __c, const int __d);
cov_simd_int128_t __builtin_neon_vmls_lanev8hi (cov_simd_int128_t __a, cov_simd_int128_t __b, cov_simd_int64_t __c, const int __d);
cov_simd_int128_t __builtin_neon_vmls_lanev4si (cov_simd_int128_t __a, cov_simd_int128_t __b, cov_simd_int64_t __c, const int __d);
cov_simd_float128_t __builtin_neon_vmls_lanev4sf (cov_simd_float128_t __a, cov_simd_float128_t __b, cov_simd_float64_t __c, const int __d);
cov_simd_uint128_t __builtin_neon_vmls_lanev8hi (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, cov_simd_uint64_t __c, const int __d);
cov_simd_uint128_t __builtin_neon_vmls_lanev4si (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, cov_simd_uint64_t __c, const int __d);
cov_simd_int128_t __builtin_neon_vmlsls_lanev4hi (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c, const int __d);
cov_simd_int128_t __builtin_neon_vmlsls_lanev2si (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c, const int __d);
cov_simd_uint128_t __builtin_neon_vmlslu_lanev4hi (cov_simd_uint128_t  __a, cov_simd_uint64_t __b, cov_simd_uint64_t __c, const int __d);
cov_simd_uint128_t __builtin_neon_vmlslu_lanev2si (cov_simd_uint128_t __a, cov_simd_uint64_t __b, cov_simd_uint64_t __c, const int __d);
cov_simd_int128_t __builtin_neon_vqdmlsl_lanev4hi (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c, const int __d);
cov_simd_int128_t __builtin_neon_vqdmlsl_lanev2si (cov_simd_int128_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c, const int __d);
cov_simd_int128_t __builtin_neon_vmulls_lanev4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vmulls_lanev2si (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_uint128_t __builtin_neon_vmullu_lanev4hi (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_uint128_t __builtin_neon_vmullu_lanev2si (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vqdmull_lanev4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vqdmull_lanev2si (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vqdmulh_lanev8hi (cov_simd_int128_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vqdmulh_lanev4si (cov_simd_int128_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vqdmulh_lanev4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vqdmulh_lanev2si (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vqrdmulh_lanev8hi (cov_simd_int128_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vqrdmulh_lanev4si (cov_simd_int128_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vqrdmulh_lanev4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vqrdmulh_lanev2si (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vmul_nv4hi (cov_simd_int64_t __a, int __b);
cov_simd_int64_t __builtin_neon_vmul_nv2si (cov_simd_int64_t __a, int __b);
cov_simd_float64_t __builtin_neon_vmul_nv2sf (cov_simd_float64_t __a, float __b);
cov_simd_uint64_t __builtin_neon_vmul_nv4hi (cov_simd_int64_t  __a, __builtin_neon_hi  __b);
cov_simd_uint64_t __builtin_neon_vmul_nv2si (cov_simd_int64_t  __a, __builtin_neon_si  __b);
cov_simd_int128_t __builtin_neon_vmul_nv8hi (cov_simd_int128_t __a, int __b);
cov_simd_int128_t __builtin_neon_vmul_nv4si (cov_simd_int128_t __a, int __b);
cov_simd_float128_t __builtin_neon_vmul_nv4sf (cov_simd_float128_t __a, float __b);
cov_simd_uint128_t __builtin_neon_vmul_nv8hi (cov_simd_int128_t  __a, __builtin_neon_hi  __b);
cov_simd_uint128_t __builtin_neon_vmul_nv4si (cov_simd_int128_t  __a, __builtin_neon_si  __b);
cov_simd_int128_t __builtin_neon_vmulls_nv4hi (cov_simd_int64_t __a, int __b);
cov_simd_int128_t __builtin_neon_vmulls_nv2si (cov_simd_int64_t __a, int __b);
cov_simd_uint128_t __builtin_neon_vmullu_nv4hi (cov_simd_int64_t  __a, __builtin_neon_hi  __b);
cov_simd_uint128_t __builtin_neon_vmullu_nv2si (cov_simd_int64_t  __a, __builtin_neon_si  __b);
cov_simd_int128_t __builtin_neon_vqdmull_nv4hi (cov_simd_int64_t __a, int __b);
cov_simd_int128_t __builtin_neon_vqdmull_nv2si (cov_simd_int64_t __a, int __b);
cov_simd_int128_t __builtin_neon_vqdmulh_nv8hi (cov_simd_int128_t __a, int __b);
cov_simd_int128_t __builtin_neon_vqdmulh_nv4si (cov_simd_int128_t __a, int __b);
cov_simd_int64_t __builtin_neon_vqdmulh_nv4hi (cov_simd_int64_t __a, int __b);
cov_simd_int64_t __builtin_neon_vqdmulh_nv2si (cov_simd_int64_t __a, int __b);
cov_simd_int128_t __builtin_neon_vqrdmulh_nv8hi (cov_simd_int128_t __a, int __b);
cov_simd_int128_t __builtin_neon_vqrdmulh_nv4si (cov_simd_int128_t __a, int __b);
cov_simd_int64_t __builtin_neon_vqrdmulh_nv4hi (cov_simd_int64_t __a, int __b);
cov_simd_int64_t __builtin_neon_vqrdmulh_nv2si (cov_simd_int64_t __a, int __b);
cov_simd_int64_t __builtin_neon_vmla_nv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, int __c);
cov_simd_int64_t __builtin_neon_vmla_nv2si (cov_simd_int64_t __a, cov_simd_int64_t __b, int __c);
cov_simd_float64_t __builtin_neon_vmla_nv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b, float __c);
cov_simd_uint64_t __builtin_neon_vmla_nv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b, __builtin_neon_hi  __c);
cov_simd_uint64_t __builtin_neon_vmla_nv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b, __builtin_neon_si  __c);
cov_simd_int128_t __builtin_neon_vmla_nv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b, int __c);
cov_simd_int128_t __builtin_neon_vmla_nv4si (cov_simd_int128_t __a, cov_simd_int128_t __b, int __c);
cov_simd_float128_t __builtin_neon_vmla_nv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b, float __c);
cov_simd_uint128_t __builtin_neon_vmla_nv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b, __builtin_neon_hi  __c);
cov_simd_uint128_t __builtin_neon_vmla_nv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b, __builtin_neon_si  __c);
cov_simd_int128_t __builtin_neon_vmlals_nv4hi (cov_simd_int128_t __a, cov_simd_int64_t __b, int __c);
cov_simd_int128_t __builtin_neon_vmlals_nv2si (cov_simd_int128_t __a, cov_simd_int64_t __b, int __c);
cov_simd_uint128_t __builtin_neon_vmlalu_nv4hi (cov_simd_int128_t  __a, cov_simd_int64_t  __b, __builtin_neon_hi  __c);
cov_simd_uint128_t __builtin_neon_vmlalu_nv2si (cov_simd_int128_t  __a, cov_simd_int64_t  __b, __builtin_neon_si  __c);
cov_simd_int128_t __builtin_neon_vqdmlal_nv4hi (cov_simd_int128_t __a, cov_simd_int64_t __b, int __c);
cov_simd_int128_t __builtin_neon_vqdmlal_nv2si (cov_simd_int128_t __a, cov_simd_int64_t __b, int __c);
cov_simd_int64_t __builtin_neon_vmls_nv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, int __c);
cov_simd_int64_t __builtin_neon_vmls_nv2si (cov_simd_int64_t __a, cov_simd_int64_t __b, int __c);
cov_simd_float64_t __builtin_neon_vmls_nv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b, float __c);
cov_simd_uint64_t __builtin_neon_vmls_nv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b, __builtin_neon_hi  __c);
cov_simd_uint64_t __builtin_neon_vmls_nv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b, __builtin_neon_si  __c);
cov_simd_int128_t __builtin_neon_vmls_nv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b, int __c);
cov_simd_int128_t __builtin_neon_vmls_nv4si (cov_simd_int128_t __a, cov_simd_int128_t __b, int __c);
cov_simd_float128_t __builtin_neon_vmls_nv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b, float __c);
cov_simd_uint128_t __builtin_neon_vmls_nv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b, __builtin_neon_hi  __c);
cov_simd_uint128_t __builtin_neon_vmls_nv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b, __builtin_neon_si  __c);
cov_simd_int128_t __builtin_neon_vmlsls_nv4hi (cov_simd_int128_t __a, cov_simd_int64_t __b, int __c);
cov_simd_int128_t __builtin_neon_vmlsls_nv2si (cov_simd_int128_t __a, cov_simd_int64_t __b, int __c);
cov_simd_uint128_t __builtin_neon_vmlslu_nv4hi (cov_simd_int128_t  __a, cov_simd_int64_t  __b, __builtin_neon_hi  __c);
cov_simd_uint128_t __builtin_neon_vmlslu_nv2si (cov_simd_int128_t  __a, cov_simd_int64_t  __b, __builtin_neon_si  __c);
cov_simd_int128_t __builtin_neon_vqdmlsl_nv4hi (cov_simd_int128_t __a, cov_simd_int64_t __b, int __c);
cov_simd_int128_t __builtin_neon_vqdmlsl_nv2si (cov_simd_int128_t __a, cov_simd_int64_t __b, int __c);
cov_simd_int64_t __builtin_neon_vextv8qi (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vextv4hi (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vextv2si (cov_simd_int64_t __a, cov_simd_int64_t __b, const int __c);
int __builtin_neon_vextdi (int __a, int __b, const int __c);
cov_simd_float64_t __builtin_neon_vextv2sf (cov_simd_float64_t __a, cov_simd_float64_t __b, const int __c);
cov_simd_uint64_t __builtin_neon_vextv8qi (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_uint64_t __builtin_neon_vextv4hi (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_uint64_t __builtin_neon_vextv2si (cov_simd_uint64_t __a, cov_simd_uint64_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vextv16qi (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vextv8hi (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vextv4si (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vextv2di (cov_simd_int128_t __a, cov_simd_int128_t __b, const int __c);
cov_simd_float128_t __builtin_neon_vextv4sf (cov_simd_float128_t __a, cov_simd_float128_t __b, const int __c);
cov_simd_uint128_t __builtin_neon_vextv16qi (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vextv8hi (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vextv4si (cov_simd_uint128_t  __a, cov_simd_uint128_t  __b, const int __c);
cov_simd_uint128_t __builtin_neon_vextv2di (cov_simd_uint128_t __a, cov_simd_uint128_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vbslv8qi (cov_simd_uint64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int64_t __builtin_neon_vbslv4hi (cov_simd_uint64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
cov_simd_int64_t __builtin_neon_vbslv2si (cov_simd_uint64_t __a, cov_simd_int64_t __b, cov_simd_int64_t __c);
int __builtin_neon_vbsldi (int  __a, int __b, int __c);
cov_simd_float64_t __builtin_neon_vbslv2sf (cov_simd_uint64_t __a, cov_simd_float64_t __b, cov_simd_float64_t __c);
cov_simd_uint64_t __builtin_neon_vbslv8qi (cov_simd_int64_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_uint64_t __builtin_neon_vbslv4hi (cov_simd_int64_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_uint64_t __builtin_neon_vbslv2si (cov_simd_int64_t  __a, cov_simd_int64_t  __b, cov_simd_int64_t  __c);
cov_simd_int128_t __builtin_neon_vbslv16qi (cov_simd_uint128_t  __a, cov_simd_int128_t __b, cov_simd_int128_t __c);
cov_simd_int128_t __builtin_neon_vbslv8hi (cov_simd_uint128_t  __a, cov_simd_int128_t __b, cov_simd_int128_t __c);
cov_simd_int128_t __builtin_neon_vbslv4si (cov_simd_uint128_t  __a, cov_simd_int128_t __b, cov_simd_int128_t __c);
cov_simd_int128_t __builtin_neon_vbslv2di (cov_simd_uint128_t __a, cov_simd_int128_t __b, cov_simd_int128_t __c);
cov_simd_float128_t __builtin_neon_vbslv4sf (cov_simd_uint128_t  __a, cov_simd_float128_t __b, cov_simd_float128_t __c);
cov_simd_uint128_t __builtin_neon_vbslv16qi (cov_simd_int128_t  __a, cov_simd_int128_t  __b, cov_simd_int128_t  __c);
cov_simd_uint128_t __builtin_neon_vbslv8hi (cov_simd_int128_t  __a, cov_simd_int128_t  __b, cov_simd_int128_t  __c);
cov_simd_uint128_t __builtin_neon_vbslv4si (cov_simd_int128_t  __a, cov_simd_int128_t  __b, cov_simd_int128_t  __c);
cov_simd_uint128_t __builtin_neon_vbslv2di (cov_simd_int128_t  __a, cov_simd_int128_t  __b, cov_simd_int128_t  __c);
cov_simd_int64_t __builtin_neon_vld1v8qi (const __builtin_neon_qi *  __a);
cov_simd_int64_t __builtin_neon_vld1v4hi (const __builtin_neon_hi *  __a);
cov_simd_int64_t __builtin_neon_vld1v2si (const __builtin_neon_si *  __a);
int __builtin_neon_vld1di (const __builtin_neon_di *  __a);
cov_simd_float64_t __builtin_neon_vld1v2sf (const __builtin_neon_sf *  __a);
cov_simd_uint64_t __builtin_neon_vld1v8qi (const __builtin_neon_qi *  __a);
cov_simd_uint64_t __builtin_neon_vld1v4hi (const __builtin_neon_hi *  __a);
cov_simd_uint64_t __builtin_neon_vld1v2si (const __builtin_neon_si *  __a);
cov_simd_int128_t __builtin_neon_vld1v16qi (const __builtin_neon_qi *  __a);
cov_simd_int128_t __builtin_neon_vld1v8hi (const __builtin_neon_hi *  __a);
cov_simd_int128_t __builtin_neon_vld1v4si (const __builtin_neon_si *  __a);
cov_simd_int128_t __builtin_neon_vld1v2di (const __builtin_neon_di *  __a);
cov_simd_float128_t __builtin_neon_vld1v4sf (const __builtin_neon_sf *  __a);
cov_simd_uint128_t __builtin_neon_vld1v16qi (const __builtin_neon_qi *  __a);
cov_simd_uint128_t __builtin_neon_vld1v8hi (const __builtin_neon_hi *  __a);
cov_simd_uint128_t __builtin_neon_vld1v4si (const __builtin_neon_si *  __a);
cov_simd_uint128_t __builtin_neon_vld1v2di (const __builtin_neon_di *  __a);
cov_simd_int64_t __builtin_neon_vld1_lanev8qi (const __builtin_neon_qi * __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vld1_lanev4hi (const __builtin_neon_hi * __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vld1_lanev2si (const int * __a, cov_simd_int64_t __b, const int __c);
cov_simd_float64_t __builtin_neon_vld1_lanev2sf (const float * __a, cov_simd_float64_t __b, const int __c);
int  __builtin_neon_vld1_lanedi (const __builtin_neon_di* __a, int __b, const int __c);
cov_simd_int128_t __builtin_neon_vld1_lanev16qi (const __builtin_neon_qi * __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vld1_lanev8hi (const __builtin_neon_hi * __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vld1_lanev4si (const int * __a, cov_simd_int128_t __b, const int __c);
cov_simd_float128_t __builtin_neon_vld1_lanev4sf (const float * __a, cov_simd_float128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vld1_lanev2di (const __builtin_neon_di* __a, cov_simd_int128_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vld1_dupv8qi (const __builtin_neon_qi *  __a);
cov_simd_int64_t __builtin_neon_vld1_dupv4hi (const __builtin_neon_hi *  __a);
cov_simd_int64_t __builtin_neon_vld1_dupv2si (const __builtin_neon_si *  __a);
cov_simd_float64_t __builtin_neon_vld1_dupv2sf (const __builtin_neon_sf *  __a);
cov_simd_uint64_t __builtin_neon_vld1_dupv8qi (const __builtin_neon_qi *  __a);
cov_simd_uint64_t __builtin_neon_vld1_dupv4hi (const __builtin_neon_hi *  __a);
cov_simd_uint64_t __builtin_neon_vld1_dupv2si (const __builtin_neon_si *  __a);
int __builtin_neon_vld1_dupdi (const __builtin_neon_di *  __a);
cov_simd_int128_t __builtin_neon_vld1_dupv16qi (const __builtin_neon_qi *  __a);
cov_simd_int128_t __builtin_neon_vld1_dupv8hi (const __builtin_neon_hi *  __a);
cov_simd_int128_t __builtin_neon_vld1_dupv4si (const __builtin_neon_si *  __a);
cov_simd_float128_t __builtin_neon_vld1_dupv4sf (const __builtin_neon_sf *  __a);
cov_simd_uint128_t __builtin_neon_vld1_dupv16qi (const __builtin_neon_qi *  __a);
cov_simd_uint128_t __builtin_neon_vld1_dupv8hi (const __builtin_neon_hi *  __a);
cov_simd_uint128_t __builtin_neon_vld1_dupv4si (const __builtin_neon_si *  __a);
cov_simd_int128_t __builtin_neon_vld1_dupv2di (const __builtin_neon_di *  __a);
cov_simd_uint128_t __builtin_neon_vld1_dupv2di (const __builtin_neon_di *  __a);
cov_simd_float64_t __builtin_neon_vst1v2sf (float * __a, cov_simd_float64_t __b);
cov_simd_int64_t __builtin_neon_vst1v8qi (__builtin_neon_qi *  __a, cov_simd_int64_t  __b);
cov_simd_int64_t __builtin_neon_vst1v4hi (__builtin_neon_hi *  __a, cov_simd_int64_t  __b);
cov_simd_int64_t __builtin_neon_vst1v2si (__builtin_neon_si *  __a, cov_simd_int64_t  __b);
cov_simd_int64_t __builtin_neon_vst1di (__builtin_neon_di *  __a, int  __b);
cov_simd_int64_t __builtin_neon_vst1v16qi (__builtin_neon_qi * __a, cov_simd_int128_t __b);
cov_simd_float128_t __builtin_neon_vst1v4sf (float * __a, cov_simd_float128_t __b);
cov_simd_int128_t __builtin_neon_vst1v8hi (__builtin_neon_hi *  __a, cov_simd_int128_t  __b);
cov_simd_int128_t __builtin_neon_vst1v4si (__builtin_neon_si *  __a, cov_simd_int128_t  __b);
cov_simd_int128_t __builtin_neon_vst1v2di (__builtin_neon_di *  __a, cov_simd_int128_t  __b);
cov_simd_int64_t __builtin_neon_vst1_lanev8qi (__builtin_neon_qi * __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vst1_lanev4hi (__builtin_neon_hi * __a, cov_simd_int64_t __b, const int __c);
cov_simd_int64_t __builtin_neon_vst1_lanev2si (__builtin_neon_si * __a, cov_simd_int64_t __b, const int __c);
cov_simd_float64_t __builtin_neon_vst1_lanev2sf (float * __a, cov_simd_float64_t __b, const int __c);
int __builtin_neon_vst1_lanedi (__builtin_neon_di * __a, int __b, const int __c);
cov_simd_int128_t __builtin_neon_vst1_lanev16qi (__builtin_neon_qi * __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vst1_lanev8hi (__builtin_neon_hi * __a, cov_simd_int128_t __b, const int __c);
cov_simd_int128_t __builtin_neon_vst1_lanev4si (__builtin_neon_si * __a, cov_simd_int128_t __b, const int __c);
cov_simd_float128_t __builtin_neon_vst1_lanev4sf (float * __a, cov_simd_float128_t __b, const int __c);
cov_simd_uint128_t __builtin_neon_vst1_lanev2di (__builtin_neon_di* __a, cov_simd_uint128_t __b, const int __c);
__builtin_neon_ti __builtin_neon_vld2v2sf (const __builtin_neon_sf *  __a);
__builtin_neon_ti __builtin_neon_vld2v8qi (const __builtin_neon_qi *  __a);
__builtin_neon_ti __builtin_neon_vld2v4hi (const __builtin_neon_hi *  __a);
__builtin_neon_ti __builtin_neon_vld2v2si (const __builtin_neon_si *  __a);
__builtin_neon_ti __builtin_neon_vld2di (const __builtin_neon_di *  __a);
__builtin_neon_oi __builtin_neon_vld2v16qi (const __builtin_neon_qi *  __a);
__builtin_neon_oi __builtin_neon_vld2v8hi (const __builtin_neon_hi *  __a);
__builtin_neon_oi __builtin_neon_vld2v4si (const __builtin_neon_si *  __a);
__builtin_neon_oi __builtin_neon_vld2v4sf (const __builtin_neon_sf *  __a);
__builtin_neon_ti __builtin_neon_vld2_lanev8qi (const __builtin_neon_qi * __a, int __b, const int __c);
__builtin_neon_ti __builtin_neon_vld2_lanev4hi (const __builtin_neon_hi * __a, int __b, const int __c);
__builtin_neon_ti __builtin_neon_vld2_lanev2si (const __builtin_neon_si * __a, int __b, const int __c);
__builtin_neon_ti __builtin_neon_vld2_lanev2sf (const __builtin_neon_sf * __a, float __b, const int __c);
__builtin_neon_oi __builtin_neon_vld2_lanev8hi (const __builtin_neon_hi * __a, __builtin_neon_oi __b, const int __c);
__builtin_neon_oi __builtin_neon_vld2_lanev4si (const __builtin_neon_si * __a, __builtin_neon_oi __b, const int __c);
__builtin_neon_oi __builtin_neon_vld2_lanev4sf (const __builtin_neon_sf * __a, __builtin_neon_oi __b, const int __c);
__builtin_neon_ti __builtin_neon_vld2_dupv8qi (const __builtin_neon_qi *  __a);
__builtin_neon_ti __builtin_neon_vld2_dupv4hi (const __builtin_neon_hi *  __a);
__builtin_neon_ti __builtin_neon_vld2_dupv2si (const __builtin_neon_si *  __a);
__builtin_neon_ti __builtin_neon_vld2_dupv2sf (const __builtin_neon_sf *  __a);
__builtin_neon_ti __builtin_neon_vld2_dupdi (const __builtin_neon_di *  __a);
int __builtin_neon_vst2v8qi (__builtin_neon_qi * __a, int __b);
int __builtin_neon_vst2v4hi (__builtin_neon_hi * __a, int __b);
int __builtin_neon_vst2v2si (__builtin_neon_si * __a, int __b);
int __builtin_neon_vst2v2sf (__builtin_neon_sf * __a, float __b);
int __builtin_neon_vst2di (__builtin_neon_di * __a, int __b);
int __builtin_neon_vst2v16qi (__builtin_neon_qi * __a, __builtin_neon_oi __b);
int __builtin_neon_vst2v8hi (__builtin_neon_hi * __a, __builtin_neon_oi __b);
int __builtin_neon_vst2v4si (__builtin_neon_si * __a, __builtin_neon_oi __b);
int __builtin_neon_vst2v4sf (__builtin_neon_sf * __a, __builtin_neon_oi __b);
int __builtin_neon_vst2_lanev8qi (__builtin_neon_qi * __a, int __b, const int __c);
int __builtin_neon_vst2_lanev4hi (__builtin_neon_hi * __a, int __b, const int __c);
int __builtin_neon_vst2_lanev2si (__builtin_neon_si * __a, int __b, const int __c);
int __builtin_neon_vst2_lanev2sf (__builtin_neon_sf * __a, float __b, const int __c);
int __builtin_neon_vst2_lanev4sf (__builtin_neon_sf * __a, __builtin_neon_oi __b, const int __c);
int __builtin_neon_vst2_lanev8hi (__builtin_neon_hi* __a, __builtin_neon_oi __b, const int __c);
int __builtin_neon_vst2_lanev4si (__builtin_neon_si * __a, __builtin_neon_oi __b, const int __c);
__builtin_neon_ei __builtin_neon_vld3v8qi (const __builtin_neon_qi *  __a);
__builtin_neon_ei __builtin_neon_vld3v4hi (const __builtin_neon_hi *  __a);
__builtin_neon_ei __builtin_neon_vld3v2si (const __builtin_neon_si *  __a);
__builtin_neon_ei __builtin_neon_vld3v2sf (const __builtin_neon_sf *  __a);
__builtin_neon_ei __builtin_neon_vld3di (const __builtin_neon_di *  __a);
__builtin_neon_ci __builtin_neon_vld3v16qi (const __builtin_neon_qi *  __a);
__builtin_neon_ci __builtin_neon_vld3v8hi (const __builtin_neon_hi *  __a);
__builtin_neon_ci __builtin_neon_vld3v4si (const __builtin_neon_si *  __a);
__builtin_neon_ci __builtin_neon_vld3v4sf (const __builtin_neon_sf *  __a);
__builtin_neon_ci __builtin_neon_vld3_lanev8hi (const __builtin_neon_hi * __a, __builtin_neon_ci __b, const int __c);
__builtin_neon_ei __builtin_neon_vld3_lanev8qi (const __builtin_neon_qi * __a, __builtin_neon_ei __b, const int __c);
__builtin_neon_ei __builtin_neon_vld3_lanev4hi (const __builtin_neon_hi * __a, __builtin_neon_ei __b, const int __c);
__builtin_neon_ei __builtin_neon_vld3_lanev2si (const __builtin_neon_si * __a, __builtin_neon_ei __b, const int __c);
__builtin_neon_ei __builtin_neon_vld3_lanev2sf (const __builtin_neon_sf * __a, __builtin_neon_ei __b, const int __c);
__builtin_neon_ci __builtin_neon_vld3_lanev4si (const __builtin_neon_si * __a, __builtin_neon_ci __b, const int __c);
__builtin_neon_ci __builtin_neon_vld3_lanev4sf (const __builtin_neon_sf * __a, __builtin_neon_ci __b, const int __c);
__builtin_neon_ei __builtin_neon_vld3_dupv8qi (const __builtin_neon_qi *  __a);
__builtin_neon_ei __builtin_neon_vld3_dupv4hi (const __builtin_neon_hi *  __a);
__builtin_neon_ei __builtin_neon_vld3_dupv2si (const __builtin_neon_si *  __a);
__builtin_neon_ei __builtin_neon_vld3_dupv2sf (const __builtin_neon_sf *  __a);
__builtin_neon_ei __builtin_neon_vld3_dupv4hi (const __builtin_neon_hi *  __a);
__builtin_neon_ei __builtin_neon_vld3_dupdi (const __builtin_neon_di *  __a);
int __builtin_neon_vst3v8qi (__builtin_neon_qi * __a, __builtin_neon_ei __b);
int __builtin_neon_vst3v4hi (__builtin_neon_hi * __a, __builtin_neon_ei __b);
int __builtin_neon_vst3v2si (__builtin_neon_si * __a, __builtin_neon_ei __b);
int __builtin_neon_vst3v2sf (__builtin_neon_sf * __a, __builtin_neon_ei __b);
int __builtin_neon_vst3di (__builtin_neon_di * __a, __builtin_neon_ei __b);
int __builtin_neon_vst3v16qi (__builtin_neon_qi * __a, __builtin_neon_ci __b);
int __builtin_neon_vst3v8hi (__builtin_neon_hi * __a, __builtin_neon_ci __b);
int __builtin_neon_vst3v4si (__builtin_neon_si * __a, __builtin_neon_ci __b);
int __builtin_neon_vst3v4sf (__builtin_neon_sf * __a, __builtin_neon_ci __b);
int __builtin_neon_vst3_lanev8qi (__builtin_neon_qi * __a, __builtin_neon_ei __b, const int __c);
int __builtin_neon_vst3_lanev4hi (__builtin_neon_hi * __a, __builtin_neon_ei __b, const int __c);
int __builtin_neon_vst3_lanev2si (__builtin_neon_si * __a, __builtin_neon_ei __b, const int __c);
int __builtin_neon_vst3_lanev2sf (__builtin_neon_sf * __a, __builtin_neon_ei __b, const int __c);
int __builtin_neon_vst3_lanev4sf (__builtin_neon_sf * __a, __builtin_neon_ci __b, const int __c);
int __builtin_neon_vst3_lanev8hi (__builtin_neon_hi* __a, __builtin_neon_ci __b, const int __c);
int __builtin_neon_vst3_lanev4si (__builtin_neon_si * __a, __builtin_neon_ci __b, const int __c);
__builtin_neon_ei __builtin_neon_vld4v8qi (const __builtin_neon_qi *  __a);
__builtin_neon_ei __builtin_neon_vld4v4hi (const __builtin_neon_hi *  __a);
__builtin_neon_ei __builtin_neon_vld4v2si (const __builtin_neon_si *  __a);
__builtin_neon_ei __builtin_neon_vld4v2sf (const __builtin_neon_sf *  __a);
__builtin_neon_ei __builtin_neon_vld4di (const __builtin_neon_di *  __a);
__builtin_neon_ci __builtin_neon_vld4v16qi (const __builtin_neon_qi *  __a);
__builtin_neon_ci __builtin_neon_vld4v8hi (const __builtin_neon_hi *  __a);
__builtin_neon_ci __builtin_neon_vld4v4si (const __builtin_neon_si *  __a);
__builtin_neon_ci __builtin_neon_vld4v4sf (const __builtin_neon_sf *  __a);
__builtin_neon_ci __builtin_neon_vld4_lanev8hi (const __builtin_neon_hi * __a, __builtin_neon_ci __b, const int __c);
__builtin_neon_ei __builtin_neon_vld4_lanev8qi (const __builtin_neon_qi * __a, __builtin_neon_ei __b, const int __c);
__builtin_neon_ei __builtin_neon_vld4_lanev4hi (const __builtin_neon_hi * __a, __builtin_neon_ei __b, const int __c);
__builtin_neon_ei __builtin_neon_vld4_lanev2si (const __builtin_neon_si * __a, __builtin_neon_ei __b, const int __c);
__builtin_neon_ei __builtin_neon_vld4_lanev2sf (const __builtin_neon_sf * __a, __builtin_neon_ei __b, const int __c);
__builtin_neon_ci __builtin_neon_vld4_lanev4si (const __builtin_neon_si * __a, __builtin_neon_ci __b, const int __c);
__builtin_neon_ci __builtin_neon_vld4_lanev4sf (const __builtin_neon_sf * __a, __builtin_neon_ci __b, const int __c);
__builtin_neon_ei __builtin_neon_vld4_dupv8qi (const __builtin_neon_qi *  __a);
__builtin_neon_ei __builtin_neon_vld4_dupv4hi (const __builtin_neon_hi *  __a);
__builtin_neon_ei __builtin_neon_vld4_dupv2si (const __builtin_neon_si *  __a);
__builtin_neon_ei __builtin_neon_vld4_dupv2sf (const __builtin_neon_sf *  __a);
__builtin_neon_ei __builtin_neon_vld4_dupv4hi (const __builtin_neon_hi *  __a);
__builtin_neon_ei __builtin_neon_vld4_dupdi (const __builtin_neon_di *  __a);
int __builtin_neon_vst4v8qi (__builtin_neon_qi * __a, __builtin_neon_ei __b);
int __builtin_neon_vst4v4hi (__builtin_neon_hi * __a, __builtin_neon_ei __b);
int __builtin_neon_vst4v2si (__builtin_neon_si * __a, __builtin_neon_ei __b);
int __builtin_neon_vst4v2sf (__builtin_neon_sf * __a, __builtin_neon_ei __b);
int __builtin_neon_vst4di (__builtin_neon_di * __a, __builtin_neon_ei __b);
int __builtin_neon_vst4v16qi (__builtin_neon_qi * __a, __builtin_neon_ci __b);
int __builtin_neon_vst4v8hi (__builtin_neon_hi * __a, __builtin_neon_ci __b);
int __builtin_neon_vst4v4si (__builtin_neon_si * __a, __builtin_neon_ci __b);
int __builtin_neon_vst4v4sf (__builtin_neon_sf * __a, __builtin_neon_ci __b);
int __builtin_neon_vst4_lanev8qi (__builtin_neon_qi * __a, __builtin_neon_ei __b, const int __c);
int __builtin_neon_vst4_lanev4hi (__builtin_neon_hi * __a, __builtin_neon_ei __b, const int __c);
int __builtin_neon_vst4_lanev2si (__builtin_neon_si * __a, __builtin_neon_ei __b, const int __c);
int __builtin_neon_vst4_lanev2sf (__builtin_neon_sf * __a, __builtin_neon_ei __b, const int __c);
int __builtin_neon_vst4_lanev4sf (__builtin_neon_sf * __a, __builtin_neon_ci __b, const int __c);
int __builtin_neon_vst4_lanev8hi (__builtin_neon_hi* __a, __builtin_neon_ci __b, const int __c);
int __builtin_neon_vst4_lanev4si (__builtin_neon_si * __a, __builtin_neon_ci __b, const int __c);
cov_simd_int64_t __builtin_neon_vreinterpretv8qiv4hi (cov_simd_int64_t  __a);
cov_simd_int64_t __builtin_neon_vreinterpretv8qiv2sf (cov_simd_float64_t __a);
cov_simd_int64_t __builtin_neon_vreinterpretv8qidi (int  __a);
cov_simd_int64_t __builtin_neon_vreinterpretv8qiv8qi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vreinterpretv8qiv2si (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vreinterpretv4hiv8qi (cov_simd_int64_t  __a);
cov_simd_int64_t __builtin_neon_vreinterpretv4hiv2sf (cov_simd_float64_t __a);
cov_simd_int64_t __builtin_neon_vreinterpretv4hidi (int  __a);
cov_simd_int64_t __builtin_neon_vreinterpretv4hiv4hi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vreinterpretv4hiv2si (cov_simd_int64_t  __a);
cov_simd_int64_t __builtin_neon_vreinterpretv2sfv8qi (cov_simd_int64_t  __a);
cov_simd_int64_t __builtin_neon_vreinterpretv2sfv2sf (cov_simd_float64_t __a);
cov_simd_int64_t __builtin_neon_vreinterpretv2sfdi (int  __a);
cov_simd_int64_t __builtin_neon_vreinterpretv2sfv4hi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vreinterpretv2sfv2si (cov_simd_int64_t  __a);
int __builtin_neon_vreinterpretdiv8qi (cov_simd_int64_t  __a);
int __builtin_neon_vreinterpretdiv2sf (cov_simd_float64_t __a);
int __builtin_neon_vreinterpretdidi (int  __a);
int __builtin_neon_vreinterpretdiv4hi (cov_simd_int64_t __a);
int __builtin_neon_vreinterpretdiv2si (cov_simd_int64_t  __a);
cov_simd_int64_t __builtin_neon_vreinterpretv2siv8qi (cov_simd_int64_t  __a);
cov_simd_int64_t __builtin_neon_vreinterpretv2siv2sf (cov_simd_float64_t __a);
cov_simd_int64_t __builtin_neon_vreinterpretv2sidi (int  __a);
cov_simd_int64_t __builtin_neon_vreinterpretv2siv4hi (cov_simd_int64_t __a);
cov_simd_int64_t __builtin_neon_vreinterpretv2siv2si (cov_simd_int64_t  __a);
cov_simd_int128_t __builtin_neon_vreinterpretv16qiv8hi (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterpretv16qiv4sf (cov_simd_float128_t __a);
cov_simd_int128_t __builtin_neon_vreinterpretv16qiv2di (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterpretv16qiti (__builtin_neon_ti __a);
cov_simd_int128_t __builtin_neon_vreinterpretv16qiv16qi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vreinterpretv16qiv4si (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vreinterpretv8hiv16qi (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterpretv8hiv4sf (cov_simd_float128_t __a);
cov_simd_int128_t __builtin_neon_vreinterpretv8hiv2di (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterpretv8hiti (__builtin_neon_ti __a);
cov_simd_int128_t __builtin_neon_vreinterpretv8hiv8hi (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterpretv8hiv4si (cov_simd_int128_t  __a);
cov_simd_float128_t __builtin_neon_vreinterpretv4sfv16qi (cov_simd_int128_t  __a);
cov_simd_float128_t __builtin_neon_vreinterpretv4sfv8hi (cov_simd_int128_t  __a);
cov_simd_float128_t __builtin_neon_vreinterpretv4sfv2di (cov_simd_int128_t  __a);
cov_simd_float128_t __builtin_neon_vreinterpretv4sfti (__builtin_neon_ti __a);
cov_simd_float128_t __builtin_neon_vreinterpretv4sfv4si (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vreinterpretv2div16qi (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterpretv2div8hi (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterpretv2div4sf (cov_simd_float128_t __a);
cov_simd_int128_t __builtin_neon_vreinterpretv2diti (__builtin_neon_ti __a);
cov_simd_int128_t __builtin_neon_vreinterpretv2div2di (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vreinterpretv2div4si (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterprettiv16qi (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterprettiv8hi (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterprettiv4sf (cov_simd_float128_t __a);
cov_simd_int128_t __builtin_neon_vreinterprettiv2di (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterprettiv2di (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vreinterprettiv2di (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterprettiv16qi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vreinterprettiv8hi (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vreinterprettiv4si (cov_simd_int128_t __a);
cov_simd_int128_t __builtin_neon_vreinterprettiv16qi (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterprettiv8hi (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterprettiv4si (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterpretv4siv16qi (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterpretv4siv8hi (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterpretv4siv4sf (cov_simd_float128_t __a);
cov_simd_int128_t __builtin_neon_vreinterpretv4siv2di (cov_simd_int128_t  __a);
cov_simd_int128_t __builtin_neon_vreinterpretv4siti (__builtin_neon_ti __a);
cov_simd_int128_t __builtin_neon_vreinterpretv4siv4si (cov_simd_int128_t  __a);
#endif // __ARM_NEON__

/* aarch64 support */
#ifdef __aarch64__
typedef int __builtin_aarch64_simd_qi __attribute__((mode(QI)));
typedef int __builtin_aarch64_simd_hi __attribute__((mode(HI)));
typedef int __builtin_aarch64_simd_si __attribute__((mode(SI)));
typedef int __builtin_aarch64_simd_di __attribute__((mode(DI)));
typedef int __builtin_aarch64_simd_poly8 __attribute__((mode(QI)));
typedef int __builtin_aarch64_simd_poly16 __attribute__((mode(HI)));
typedef int __builtin_aarch64_simd_poly64 __attribute__ ((mode (V2DI)));
typedef unsigned int __builtin_aarch64_simd_poly128 __attribute__ ((mode (TI)));
typedef float __builtin_aarch64_simd_df __attribute__ ((mode (DF)));
typedef float __builtin_aarch64_simd_sf __attribute__ ((mode (SF)));
typedef unsigned int __builtin_aarch64_simd_udi __attribute__((mode(DI)));
typedef unsigned int __builtin_aarch64_simd_uqi __attribute__((mode(QI)));
typedef unsigned int __builtin_aarch64_simd_uhi __attribute__((mode(HI)));
typedef unsigned int __builtin_aarch64_simd_usi __attribute__((mode(SI)));
#endif

#ifdef __IWMMXT__
typedef unsigned long long __mmx_m64;
typedef int __mmx_v2si __attribute__ ((vector_size (8)));
typedef short __mmx_v4hi __attribute__ ((vector_size (8)));

#ifdef __ANDROID__
typedef char __mmx_v8qi __attribute__ ((vector_size (8)));
__mmx_m64 __builtin_arm_wsadb (__mmx_v8qi  __A, __mmx_v8qi __B);
__mmx_m64 __builtin_arm_wsadh (__mmx_v4hi  __A, __mmx_v4hi __B);
__mmx_m64 __builtin_arm_walign (__mmx_v8qi __a, __mmx_v8qi __b, int __C);
void __builtin_arm_setwcx(int x, int y);
int __builtin_arm_getwcx(int x);
#else
typedef signed char __mmx_v8qi __attribute__ ((vector_size (8)));
__mmx_m64 __builtin_arm_wsadb (__mmx_v2si  __A, __mmx_v8qi __B, __mmx_v8qi __C);
__mmx_m64 __builtin_arm_wsadh (__mmx_v2si  __A, __mmx_v4hi __B, __mmx_v4hi __C);
#endif // __ANDROID__

__mmx_m64 __builtin_arm_wpackhss (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_wpackwss (__mmx_v2si  __m1, __mmx_v2si  __m2);
__mmx_m64 __builtin_arm_wpackdss (long long __m1, long long __m2);
__mmx_m64 __builtin_arm_wpackhus (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_wpackwus (__mmx_v2si  __m1, __mmx_v2si  __m2);
__mmx_m64 __builtin_arm_wpackdus (long long __m1, long long __m2);
__mmx_m64 __builtin_arm_wunpckihb (__mmx_v8qi __m1, __mmx_v8qi __m2);
__mmx_m64 __builtin_arm_wunpckihh (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_wunpckihw (__mmx_v2si  __m1, __mmx_v2si  __m2);
__mmx_m64 __builtin_arm_wunpckilb (__mmx_v8qi __m1, __mmx_v8qi __m2);
__mmx_m64 __builtin_arm_wunpckilh (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_wunpckilw (__mmx_v2si  __m1, __mmx_v2si  __m2);
__mmx_m64 __builtin_arm_wunpckelsb (__mmx_v8qi __m1);
__mmx_m64 __builtin_arm_wunpckelsh (__mmx_v4hi __m1);
__mmx_m64 __builtin_arm_wunpckelsw (__mmx_v2si  __m1);
__mmx_m64 __builtin_arm_wunpckehsb (__mmx_v8qi __m1);
__mmx_m64 __builtin_arm_wunpckehsh (__mmx_v4hi __m1);
__mmx_m64 __builtin_arm_wunpckehsw (__mmx_v2si  __m1);
__mmx_m64 __builtin_arm_wunpckelub (__mmx_v8qi __m1);
__mmx_m64 __builtin_arm_wunpckeluh (__mmx_v4hi __m1);
__mmx_m64 __builtin_arm_wunpckeluw (__mmx_v2si  __m1);
__mmx_m64 __builtin_arm_wunpckehub (__mmx_v8qi __m1);
__mmx_m64 __builtin_arm_wunpckehuh (__mmx_v4hi __m1);
__mmx_m64 __builtin_arm_wunpckehuw (__mmx_v2si  __m1);
__mmx_m64 __builtin_arm_waddb (__mmx_v8qi __m1, __mmx_v8qi __m2);
__mmx_m64 __builtin_arm_waddh (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_waddw (__mmx_v2si  __m1, __mmx_v2si  __m2);
__mmx_m64 __builtin_arm_waddbss (__mmx_v8qi __m1, __mmx_v8qi __m2);
__mmx_m64 __builtin_arm_waddhss (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_waddwss (__mmx_v2si  __m1, __mmx_v2si  __m2);
__mmx_m64 __builtin_arm_waddbus (__mmx_v8qi __m1, __mmx_v8qi __m2);
__mmx_m64 __builtin_arm_waddhus (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_waddwus (__mmx_v2si  __m1, __mmx_v2si  __m2);
__mmx_m64 __builtin_arm_wsubb (__mmx_v8qi __m1, __mmx_v8qi __m2);
__mmx_m64 __builtin_arm_wsubh (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_wsubw (__mmx_v2si  __m1, __mmx_v2si  __m2);
__mmx_m64 __builtin_arm_wsubbss (__mmx_v8qi __m1, __mmx_v8qi __m2);
__mmx_m64 __builtin_arm_wsubhss (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_wsubwss (__mmx_v2si  __m1, __mmx_v2si  __m2);
__mmx_m64 __builtin_arm_wsubbus (__mmx_v8qi __m1, __mmx_v8qi __m2);
__mmx_m64 __builtin_arm_wsubhus (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_wsubwus (__mmx_v2si  __m1, __mmx_v2si  __m2);
__mmx_m64 __builtin_arm_wmadds (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_wmaddu (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_wmulsm (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_wmulum (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_wmulul (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_wsllh (__mmx_v4hi __m, int __count);
__mmx_m64 __builtin_arm_wsllhi (__mmx_v4hi __m, int __count);
__mmx_m64 __builtin_arm_wsllw (__mmx_v2si  __m, int __count);
__mmx_m64 __builtin_arm_wsllwi (__mmx_v2si  __m, int __count);
__mmx_m64 __builtin_arm_wslld (int __m, int __count);
__mmx_m64 __builtin_arm_wslldi (int __m, int __count);
__mmx_m64 __builtin_arm_wsrah (__mmx_v4hi __m, int __count);
__mmx_m64 __builtin_arm_wsrahi (__mmx_v4hi __m, int __count);
__mmx_m64 __builtin_arm_wsraw (__mmx_v2si  __m, int __count);
__mmx_m64 __builtin_arm_wsrawi (__mmx_v2si  __m, int __count);
__mmx_m64 __builtin_arm_wsrad (int __m, int __count);
__mmx_m64 __builtin_arm_wsradi (int __m, int __count);
__mmx_m64 __builtin_arm_wsrlh (__mmx_v4hi __m, int __count);
__mmx_m64 __builtin_arm_wsrlhi (__mmx_v4hi __m, int __count);
__mmx_m64 __builtin_arm_wsrlw (__mmx_v2si  __m, int __count);
__mmx_m64 __builtin_arm_wsrlwi (__mmx_v2si  __m, int __count);
__mmx_m64 __builtin_arm_wsrld (int __m, int __count);
__mmx_m64 __builtin_arm_wsrldi (int __m, int __count);
__mmx_m64 __builtin_arm_wrorh (__mmx_v4hi __m, int __count);
__mmx_m64 __builtin_arm_wrorhi (__mmx_v4hi __m, int __count);
__mmx_m64 __builtin_arm_wrorw (__mmx_v2si  __m, int __count);
__mmx_m64 __builtin_arm_wrorwi (__mmx_v2si  __m, int __count);
__mmx_m64 __builtin_arm_wrord (int __m, int __count);
__mmx_m64 __builtin_arm_wrordi (int __m, int __count);
__mmx_m64 __builtin_arm_wand (int __m1, int __m2);
__mmx_m64 __builtin_arm_wandn (int __m2, int __m1);
__mmx_m64 __builtin_arm_wor (int __m1, int __m2);
__mmx_m64 __builtin_arm_wxor (int __m1, int __m2);
__mmx_m64 __builtin_arm_wcmpeqb (__mmx_v8qi __m1, __mmx_v8qi __m2);
__mmx_m64 __builtin_arm_wcmpgtsb (__mmx_v8qi __m1, __mmx_v8qi __m2);
__mmx_m64 __builtin_arm_wcmpgtub (__mmx_v8qi __m1, __mmx_v8qi __m2);
__mmx_m64 __builtin_arm_wcmpeqh (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_wcmpgtsh (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_wcmpgtuh (__mmx_v4hi __m1, __mmx_v4hi __m2);
__mmx_m64 __builtin_arm_wcmpeqw (__mmx_v2si  __m1, __mmx_v2si  __m2);
__mmx_m64 __builtin_arm_wcmpgtsw (__mmx_v2si  __m1, __mmx_v2si  __m2);
__mmx_m64 __builtin_arm_wcmpgtuw (__mmx_v2si  __m1, __mmx_v2si  __m2);
__mmx_m64 __builtin_arm_wmacu (int __A, __mmx_v4hi __B, __mmx_v4hi __C);
__mmx_m64 __builtin_arm_wmacs (int __A, __mmx_v4hi __B, __mmx_v4hi __C);
__mmx_m64 __builtin_arm_wmacuz (__mmx_v4hi __A, __mmx_v4hi __B);
__mmx_m64 __builtin_arm_wmacsz (__mmx_v4hi __A, __mmx_v4hi __B);
__mmx_m64 __builtin_arm_waccb (__mmx_v8qi __A);
__mmx_m64 __builtin_arm_wacch (__mmx_v4hi __A);
__mmx_m64 __builtin_arm_waccw (__mmx_v2si  __A);
__mmx_m64 __builtin_arm_tmia (int __A, int __B, int __C);
__mmx_m64 __builtin_arm_tmiaph (int __A, int __B, int __C);
__mmx_m64 __builtin_arm_tmiabb (int __A, int __B, int __C);
__mmx_m64 __builtin_arm_tmiabt (int __A, int __B, int __C);
__mmx_m64 __builtin_arm_tmiatb (int __A, int __B, int __C);
__mmx_m64 __builtin_arm_tmiatt (int __A, int __B, int __C);
__mmx_m64 __builtin_arm_wmaxsb (__mmx_v8qi __A, __mmx_v8qi __B);
__mmx_m64 __builtin_arm_wmaxsh (__mmx_v4hi __A, __mmx_v4hi __B);
__mmx_m64 __builtin_arm_wmaxsw (__mmx_v2si  __A, __mmx_v2si  __B);
__mmx_m64 __builtin_arm_wmaxub (__mmx_v8qi __A, __mmx_v8qi __B);
__mmx_m64 __builtin_arm_wmaxuh (__mmx_v4hi __A, __mmx_v4hi __B);
__mmx_m64 __builtin_arm_wmaxuw (__mmx_v2si  __A, __mmx_v2si  __B);
__mmx_m64 __builtin_arm_wminsb (__mmx_v8qi __A, __mmx_v8qi __B);
__mmx_m64 __builtin_arm_wminsh (__mmx_v4hi __A, __mmx_v4hi __B);
__mmx_m64 __builtin_arm_wminsw (__mmx_v2si  __A, __mmx_v2si  __B);
__mmx_m64 __builtin_arm_wminub (__mmx_v8qi __A, __mmx_v8qi __B);
__mmx_m64 __builtin_arm_wminuh (__mmx_v4hi __A, __mmx_v4hi __B);
__mmx_m64 __builtin_arm_wminuw (__mmx_v2si  __A, __mmx_v2si  __B);
__mmx_m64 __builtin_arm_tmovmskb (__mmx_v8qi __A);
__mmx_m64 __builtin_arm_tmovmskh (__mmx_v4hi __A);
__mmx_m64 __builtin_arm_tmovmskw (__mmx_v2si  __A);
__mmx_m64 __builtin_arm_wavg2br (__mmx_v8qi __A, __mmx_v8qi __B);
__mmx_m64 __builtin_arm_wavg2hr (__mmx_v4hi __A, __mmx_v4hi __B);
__mmx_m64 __builtin_arm_wavg2b (__mmx_v8qi __A, __mmx_v8qi __B);
__mmx_m64 __builtin_arm_wavg2h (__mmx_v4hi __A, __mmx_v4hi __B);
__mmx_m64 __builtin_arm_wsadbz (__mmx_v8qi __A, __mmx_v8qi __B);
__mmx_m64 __builtin_arm_wsadhz (__mmx_v4hi __A, __mmx_v4hi __B);
__mmx_m64 __builtin_arm_wsadbz (__mmx_v8qi __A, __mmx_v8qi __B);
__mmx_m64 __builtin_arm_wsadhz (__mmx_v4hi __A, __mmx_v4hi __B);
__mmx_m64 __builtin_arm_wzero ();
__mmx_m64 __builtin_arm_setwcgr0 (int __value);
__mmx_m64 __builtin_arm_setwcgr1 (int __value);
__mmx_m64 __builtin_arm_setwcgr2 (int __value);
__mmx_m64 __builtin_arm_setwcgr3 (int __value);
__mmx_m64 __builtin_arm_getwcgr0 ();
__mmx_m64 __builtin_arm_getwcgr1 ();
__mmx_m64 __builtin_arm_getwcgr2 ();
__mmx_m64 __builtin_arm_getwcgr3 ();
__mmx_m64 __builtin_arm_wabsb (__mmx_v8qi m1);
__mmx_m64 __builtin_arm_wabsh (__mmx_v4hi __m1);
__mmx_m64 __builtin_arm_wabsw (__mmx_v2si  __m1);
__mmx_m64 __builtin_arm_waddsubhx (__mmx_v4hi __a, __mmx_v4hi __b);
__mmx_m64 __builtin_arm_wabsdiffb (__mmx_v8qi __a, __mmx_v8qi __b);
__mmx_m64 __builtin_arm_wabsdiffh (__mmx_v4hi __a, __mmx_v4hi __b);
__mmx_m64 __builtin_arm_wabsdiffw (__mmx_v2si __a, __mmx_v2si __b);
__mmx_m64 __builtin_arm_wavg4 (__mmx_v8qi __a, __mmx_v8qi __b);
__mmx_m64 __builtin_arm_wavg4r (__mmx_v8qi __a, __mmx_v8qi __b);
__mmx_m64 __builtin_arm_wmaddsx (__mmx_v4hi __a, __mmx_v4hi __b);
__mmx_m64 __builtin_arm_wmaddux (__mmx_v4hi __a, __mmx_v4hi __b);
__mmx_m64 __builtin_arm_wmaddsn (__mmx_v4hi __a, __mmx_v4hi __b);
__mmx_m64 __builtin_arm_wmaddun (__mmx_v4hi __a, __mmx_v4hi __b);
__mmx_m64 __builtin_arm_wmulwsm (__mmx_v2si __a, __mmx_v2si __b);
__mmx_m64 __builtin_arm_wmulwum (__mmx_v2si __a, __mmx_v2si __b);
__mmx_m64 __builtin_arm_wmulsmr (__mmx_v4hi __a, __mmx_v4hi __b);
__mmx_m64 __builtin_arm_wmulwsmr (__mmx_v2si __a, __mmx_v2si __b);
__mmx_m64 __builtin_arm_wmulumr (__mmx_v4hi __a, __mmx_v4hi __b);
__mmx_m64 __builtin_arm_wmulwumr (__mmx_v2si __a, __mmx_v2si __b);
__mmx_m64 __builtin_arm_wmulwl (__mmx_v2si __a, __mmx_v2si __b);
__mmx_m64 __builtin_arm_wqmulm (__mmx_v4hi __a, __mmx_v4hi __b);
__mmx_m64 __builtin_arm_wqmulwm (__mmx_v2si __a, __mmx_v2si __b);
__mmx_m64 __builtin_arm_wqmulmr (__mmx_v4hi __a, __mmx_v4hi __b);
__mmx_m64 __builtin_arm_wqmulwmr (__mmx_v2si __a, __mmx_v2si __b);
__mmx_m64 __builtin_arm_wsubaddhx (__mmx_v4hi __a, __mmx_v4hi __b);
__mmx_m64 __builtin_arm_waddbhusl (__mmx_v4hi __a, __mmx_v8qi __b);
__mmx_m64 __builtin_arm_waddbhusm (__mmx_v4hi __a, __mmx_v8qi __b);
__mmx_m64 __builtin_arm_walignr0 (__mmx_v8qi __a, __mmx_v8qi __b);
__mmx_m64 __builtin_arm_walignr1 (__mmx_v8qi __a, __mmx_v8qi __b);
__mmx_m64 __builtin_arm_walignr2 (__mmx_v8qi __a, __mmx_v8qi __b);
__mmx_m64 __builtin_arm_walignr3 (__mmx_v8qi __a, __mmx_v8qi __b);
__mmx_m64 __builtin_arm_tbcstb (signed char value);
__mmx_m64 __builtin_arm_tbcsth (short value);
__mmx_m64 __builtin_arm_tbcstw (int value);
#endif // __IWMMX__
/*
  Copyright (c) 2017 Synopsys, Inc. All rights reserved worldwide.
  The information contained in this file is the proprietary and confidential
  information of Synopsys, Inc. and its licensors, and is supplied subject to,
  and may be used only by Synopsys customers in accordance with the terms and
  conditions of a previously executed license agreement between Synopsys and
  that customer.
*/


#define __COVERITY_GCC_VERSION_AT_LEAST(maj, min) \
    ((__GNUC__ > (maj)) || (__GNUC__ == (maj) && __GNUC_MINOR__ >= (min)))

#if __COVERITY_GCC_VERSION_AT_LEAST(4, 9)
// Starting with GCC 4.9, instruction set intrinsics are always visible
// regardless of whether or not the instruction set is enabled.
#define __COVERITY_GCC49_INTRINSICS 1
#else // GCC <4.9
#define __COVERITY_GCC49_INTRINSICS 0
#endif

#ifdef __IA64__
typedef __coverity___fpreg long double __fpreg;
#endif

//#ifdef __clang__
//#define __has_include __coverity_has_include
//#define __has_include_next __coverity_has_include_next
//#endif

#ifdef __cplusplus
extern "C" {
#endif

// The following macros are used in the Linux Kernel
#nodef BUG_ON(x) do { if (x) __coverity_panic__(); } while (0)
#nodef WARN_ON(x) ({ int result = !!(x); if (result) __coverity_panic__(); result; })
#nodef BUG() __coverity_panic__()
void __coverity_panic__();

#nodef setjmp
int setjmp(void *);

#ifndef __COVERITY_NO_STRING_NODEFS__
// Function list obtained from "cstring"+memrchr+stpcopy (from bits/string.h)

#nodef memcpy
void *memcpy(void *, const void *, __COVERITY_SIZE_TYPE__);

#nodef memmove
void *memmove(void *, const void *, __COVERITY_SIZE_TYPE__);

#nodef strcpy
char *strcpy(char *, const char *);

#nodef strncpy
char *strncpy(char *, const char *, __COVERITY_SIZE_TYPE__);

#nodef strcat
char *strcat(char *, const char *);

#nodef strncat
char *strncat(char *, const char *, __COVERITY_SIZE_TYPE__);

#nodef memcmp
int memcmp(const void *, const void *, __COVERITY_SIZE_TYPE__ n);

#nodef strcmp
int strcmp(const char *, const char *);

#nodef strcoll
int strcoll(const char *, const char *);

#nodef strncmp
int strncmp(const char *, const char *, __COVERITY_SIZE_TYPE__);

#nodef strxfrm
__COVERITY_SIZE_TYPE__
strxfrm(char *, const char *, __COVERITY_SIZE_TYPE__);

#nodef memchr
void *memchr(const void *, int, __COVERITY_SIZE_TYPE__);

#nodef strchr
char *strchr(const char *, int);

#nodef strcspn
__COVERITY_SIZE_TYPE__ strcspn(const char *, const char *);

#nodef strpbrk
char *strpbrk(const char *, const char *);

#nodef strrchr
char *strrchr(const char *, int);

#nodef strspn
__COVERITY_SIZE_TYPE__ strspn(const char *, const char *);

#nodef strstr
char *strstr(const char *, const char *);

#nodef strtok
char *strtok(char *, const char *);

#nodef memset
void *memset(void *, int, __COVERITY_SIZE_TYPE__);

#nodef strlen
__COVERITY_SIZE_TYPE__ strlen(const char *);

#nodef strerror
char *strerror(int);

#nodef memrchr
void *memrchr(const void *, int, __COVERITY_SIZE_TYPE__);

#nodef stpcpy
char *stpcpy(char *, const char *);

#nodef strdup
char *strdup(const char *);
#endif // __COVERITY_NO_STRING_NODEFS__

#ifdef __cplusplus
}
#endif

#if __COVERITY_GCC_VERSION_AT_LEAST(3, 4)
extern char *__builtin_stpcpy(char *, const char *);
#endif

#if __COVERITY_GCC_VERSION_AT_LEAST(4, 4)
#define __builtin_ms_va_list __builtin_va_list
#define __builtin_ms_va_copy __builtin_va_copy
#define __builtin_ms_va_start __builtin_va_start
#define __builtin_ms_va_end __builtin_va_end

#define __builtin_sysv_va_list __builtin_va_list
#define __builtin_sysv_va_copy __builtin_va_copy
#define __builtin_sysv_va_start __builtin_va_start
#define __builtin_sysv_va_end __builtin_va_end

#define __ms_va_copy(__d,__s) __builtin_ms_va_copy(__d,__s)
#define __ms_va_start(__v,__l) __builtin_ms_va_start(__v,__l)
#define __ms_va_arg(__v,__l)    __builtin_va_arg(__v,__l)
#define __ms_va_end(__v) __builtin_ms_va_end(__v)

#define __sysv_va_copy(__d,__s) __builtin_sysv_va_copy(__d,__s)
#define __sysv_va_start(__v,__l) __builtin_sysv_va_start(__v,__l)
#define __sysv_va_arg(__v,__l)  __builtin_va_arg(__v,__l)
#define __sysv_va_end(__v) __builtin_sysv_va_end(__v)
#endif // 4.4+

#if __COVERITY_GCC_VERSION_AT_LEAST(4, 0)

#if __COVERITY_GCC_VERSION_AT_LEAST(4, 2) && (defined(__x86_64__))
typedef unsigned int __coverity_uint128 __attribute__ ((mode(TI)));
extern __coverity_uint128 __sync_fetch_and_add_16 (volatile void*, __coverity_uint128);
#endif // 4.2+ && (defined(__x86_64__))

#if __COVERITY_GCC_VERSION_AT_LEAST(4, 1)
#if defined(__x86_64__) // 64 bit
extern long unsigned int __sync_fetch_and_add_8 (volatile void*, long unsigned int);
#else // 32 bit
extern long long unsigned int __sync_fetch_and_add_8 (volatile void*, long long unsigned int);
#endif // defined(__x86_64__)
extern unsigned int __sync_fetch_and_add_4 (volatile void*, unsigned int);
extern short unsigned int __sync_fetch_and_add_2 (volatile void*, short unsigned int);
extern unsigned char __sync_fetch_and_add_1 (volatile void*, unsigned char);
#endif // 4.1+

extern double __builtin_powi(double,int);
extern float __builtin_powif(float,int);
extern long double __builtin_powil(long double,int);

extern float __builtin_cabsf(__complex__ float);
extern double __builtin_cabs(__complex__ double);
extern long double __builtin_cabsl(__complex__ long double);
extern float __builtin_cargf(__complex__ float);
extern double __builtin_carg(__complex__ double);
extern long double __builtin_cargl(__complex__ long double);

#define __builtin_memchr(dst, src, len) memchr(dst, src, len)
#define __builtin_memcpy(dst, src, len) memcpy(dst, src, len)
#define __builtin_memmove(dst, src, len) memmove(dst, src, len)
#define __builtin_memset(dst, val, len) memset(dst, val, len)
#define __builtin_strcpy(dst, src) strcpy(dst, src)
#define __builtin_strcat(dst, src) strcat(dst, src)
#define __builtin_strncpy(dst, src, len) strncpy(dst, src, len)
#define __builtin_strncat(dst, src, len) strncat(dst, src, len)
#define __builtin_mempcpy(dst, src, len) mempcpy (dst, src, len)
#if (__GNUC__ == 4) && (__GNUC_MINOR__ == 9) && ((__GNUC_PATCHLEVEL__ == 2) || (__GNUC_PATCHLEVEL__ == 3))
int __coverity_builtin_sprintf();
#define __builtin_sprintf(dst, ...) __coverity_builtin_sprintf();
#else
#define __builtin_sprintf(dst, ...) sprintf (dst, __VA_ARGS__)
#endif
#define __builtin_snprintf(dst, len, ...) snprintf (dst, len, __VA_ARGS__)

int __builtin_vsprintf(char *, const char *, __builtin_va_list);
int __builtin_vsnprintf(char *, int, const char *, __builtin_va_list);

typedef float __coverity_decimal _Decimal32;
typedef double __coverity_decimal _Decimal64;
typedef long double __coverity_decimal _Decimal128;
#endif

#ifndef __COVERITY_DISABLE_BUILTIN_DECLS

/* mmx, sse compat */

#if (__GNUC__ <3)
typedef int __cov_m64 __attribute__ ((__mode__ (__V2SI__)));
typedef float __cov_m128 __attribute__ ((__mode__(__V4SF__)));

/* Internal data types for implementing the intrinsics.  */
typedef int __cov_v2si __attribute__ ((__mode__ (__V2SI__)));
typedef int __cov_v4hi __attribute__ ((__mode__ (__V4HI__)));
typedef int __cov_v8qi __attribute__ ((__mode__ (__V8QI__)));

/* Internal data types for implementing the intrinsics.  */
typedef float __cov_v4sf __attribute__ ((__mode__(__V4SF__)));
typedef int __cov_v4si __attribute__ ((mode (V4SI)));

typedef double __cov_v2df __attribute__ ((mode (V2DF)));
typedef int __cov_v8hi __attribute__ ((mode (V8HI)));
typedef int __cov_v2di __attribute__ ((mode (V2DI)));
typedef int __cov_v16qi __attribute__ ((mode (V16QI)));

typedef int __cov_v1di __attribute__ ((mode (V1DI)));

#define __cov_m128i __cov_v2di
#define __cov_m128d __cov_v2df
#endif

#if __GNUC__ == 3

typedef int __cov_m64 __attribute__ ((__mode__ (__V2SI__)));

/* Internal data types for implementing the intrinsics.  */
typedef int __cov_v2si __attribute__ ((__mode__ (__V2SI__)));
typedef int __cov_v4hi __attribute__ ((__mode__ (__V4HI__)));
typedef int __cov_v8qi __attribute__ ((__mode__ (__V8QI__)));

/* The data type intended for user use.  */
typedef float __cov_m128 __attribute__ ((__mode__(__V4SF__)));

/* Internal data types for implementing the intrinsics.  */
typedef float __cov_v4sf __attribute__ ((__mode__(__V4SF__)));
typedef int __cov_v4si __attribute__ ((mode (V4SI)));

typedef double __cov_v2df __attribute__ ((mode (V2DF)));
typedef int __cov_v2di __attribute__ ((mode (V2DI)));
typedef int __cov_v8hi __attribute__ ((mode (V8HI)));
typedef int __cov_v16qi __attribute__ ((mode (V16QI)));
typedef int __cov_v1di __attribute__ ((mode (V1DI)));

#define __cov_m128i __cov_v2di
#define __cov_m128d __cov_v2df

#endif // gnu 3.x 

#if __COVERITY_GCC_VERSION_AT_LEAST(4, 0)

/* The Intel API is flexible enough that we must allow aliasing with other
vector types, and their scalar components.  */
#if defined(__APPLE__)
typedef long long __cov_m64 __attribute__ ((__vector_size__ (8)));
#else
typedef int __cov_m64 __attribute__ ((__vector_size__ (8), __may_alias__));
#endif

/* Internal data types for implementing the intrinsics.  */
typedef int __cov_v2si __attribute__ ((__vector_size__ (8)));
typedef short __cov_v4hi __attribute__ ((__vector_size__ (8)));
typedef char __cov_v8qi __attribute__ ((__vector_size__ (8)));
typedef long long __cov_v1di __attribute__ ((__vector_size__ (8)));

/* The Intel API is flexible enough that we must allow aliasing with other
vector types, and their scalar components.  */
typedef float __cov_m128 __attribute__ ((__vector_size__ (16), __may_alias__));
/* Internal data types for implementing the intrinsics.  */
typedef float __cov_v4sf __attribute__ ((__vector_size__ (16)));
typedef int __cov_v4si __attribute__ ((__vector_size__ (16)));
typedef float __cov_v2sf __attribute__ ((__vector_size__ (8)));

/* SSE2 */
typedef double __cov_v2df __attribute__ ((__vector_size__ (16)));
typedef long long __cov_v2di __attribute__ ((__vector_size__ (16)));
typedef short __cov_v8hi __attribute__ ((__vector_size__ (16)));
typedef char __cov_v16qi __attribute__ ((__vector_size__ (16)));

/* The Intel API is flexible enough that we must allow aliasing with other
vector types, and their scalar components.  */
typedef long long __cov_m128i __attribute__ ((__vector_size__ (16), __may_alias__));
typedef double __cov_m128d __attribute__ ((__vector_size__ (16), __may_alias__));

typedef double     __cov_v4df  __attribute__ ((__vector_size__ (32)));
typedef float      __cov_v8sf  __attribute__ ((__vector_size__ (32)));
typedef long long  __cov_v4di  __attribute__ ((__vector_size__ (32)));
typedef int        __cov_v8si  __attribute__ ((__vector_size__ (32)));
typedef short      __cov_v16hi __attribute__ ((__vector_size__ (32)));
typedef char       __cov_v32qi __attribute__ ((__vector_size__ (32)));
typedef float      __cov_m256  __attribute__ ((__vector_size__ (32),
                                              __may_alias__));
typedef long long  __cov_m256i __attribute__ ((__vector_size__ (32),
                                               __may_alias__));
typedef double     __cov_m256d __attribute__ ((__vector_size__ (32),
                                               __may_alias__));
#endif // GNU 4+

#if defined(__GNUC__)

int __builtin_ia32_bsrsi(int);
void __builtin_ia32_fxsave(void *);
void __builtin_ia32_fxsave64(void *);
void __builtin_ia32_fxrstor(void *);
void __builtin_ia32_fxrstor64(void *);
unsigned long long __builtin_ia32_rdpmc(int);
unsigned long long __builtin_ia32_rdtsc();
unsigned long long __builtin_ia32_rdtscp(unsigned int *);
unsigned char __builtin_ia32_rolqi(unsigned char,int);
unsigned short __builtin_ia32_rolhi(unsigned short,int);
unsigned char __builtin_ia32_rorqi(unsigned char,int);
unsigned short __builtin_ia32_rorhi(unsigned short,int);
int __builtin_ia32_bsrdi(long long);
unsigned char __builtin_ia32_addcarryx_u32(unsigned char,unsigned int,unsigned int,unsigned int *);
unsigned char __builtin_ia32_addcarryx_u64(unsigned char,unsigned long,unsigned long,unsigned long long *);

//SSE
void __builtin_ia32_pause();
__cov_m64 __builtin_ia32_pshufw(__cov_v4hi,int const);
__cov_m64 __builtin_ia32_vec_set_v4hi(__cov_v4hi, int const, int const);
int __builtin_ia32_vec_ext_v4hi(__cov_v4hi,int const);
__cov_m128i __builtin_ia32_vec_set_v8hi(__cov_v8hi,int const,int const);
unsigned short __builtin_ia32_vec_ext_v8hi(__cov_v8hi,int const);

// generic MMX
void __builtin_ia32_emms ();

__cov_v4hi __builtin_ia32_packsswb (__cov_v4hi, __cov_v4hi);
__cov_v2si __builtin_ia32_packssdw (__cov_v2si, __cov_v2si);
__cov_v4hi __builtin_ia32_packuswb (__cov_v4hi, __cov_v4hi);
__cov_v8qi __builtin_ia32_punpckhbw (__cov_v8qi, __cov_v8qi);
__cov_v4hi __builtin_ia32_punpckhwd (__cov_v4hi, __cov_v4hi);
__cov_v2si __builtin_ia32_punpckhdq (__cov_v2si, __cov_v2si);
__cov_v8qi __builtin_ia32_punpcklbw (__cov_v8qi, __cov_v8qi);
__cov_v4hi __builtin_ia32_punpcklwd (__cov_v4hi, __cov_v4hi);
__cov_v2si __builtin_ia32_punpckldq (__cov_v2si, __cov_v2si);
__cov_v8qi __builtin_ia32_paddb (__cov_v8qi, __cov_v8qi);
__cov_v4hi __builtin_ia32_paddw (__cov_v4hi, __cov_v4hi);
__cov_v2si __builtin_ia32_paddd (__cov_v2si, __cov_v2si);
__cov_v8qi __builtin_ia32_paddsb (__cov_v8qi, __cov_v8qi);
__cov_v4hi __builtin_ia32_paddsw (__cov_v4hi, __cov_v4hi);
__cov_v8qi __builtin_ia32_paddusb (__cov_v8qi, __cov_v8qi);
__cov_v4hi __builtin_ia32_paddusw (__cov_v4hi, __cov_v4hi);
__cov_v8qi __builtin_ia32_psubb (__cov_v8qi, __cov_v8qi);
__cov_v4hi __builtin_ia32_psubw (__cov_v4hi, __cov_v4hi);
__cov_v2si __builtin_ia32_psubd (__cov_v2si, __cov_v2si);
__cov_v8qi __builtin_ia32_psubsb (__cov_v8qi, __cov_v8qi);
__cov_v4hi __builtin_ia32_psubsw (__cov_v4hi, __cov_v4hi);
__cov_v8qi __builtin_ia32_psubusb (__cov_v8qi, __cov_v8qi);
__cov_v4hi __builtin_ia32_psubusw (__cov_v4hi, __cov_v4hi);
__cov_v4hi __builtin_ia32_pmaddwd (__cov_v4hi, __cov_v4hi);
__cov_v4hi __builtin_ia32_pmulhw (__cov_v4hi, __cov_v4hi);
__cov_v4hi __builtin_ia32_pmullw (__cov_v4hi, __cov_v4hi);
__cov_v8qi __builtin_ia32_pcmpeqb (__cov_v8qi, __cov_v8qi);
__cov_v8qi __builtin_ia32_pcmpgtb (__cov_v8qi, __cov_v8qi);
__cov_v4hi __builtin_ia32_pcmpeqw (__cov_v4hi, __cov_v4hi);
__cov_v4hi __builtin_ia32_pcmpgtw (__cov_v4hi, __cov_v4hi);
__cov_v2si __builtin_ia32_pcmpeqd (__cov_v2si, __cov_v2si);
__cov_v2si __builtin_ia32_pcmpgtd (__cov_v2si, __cov_v2si);
__cov_v2si __builtin_ia32_mmx_zero ();

#if __COVERITY_GCC_VERSION_AT_LEAST(4, 4)
long long __builtin_ia32_paddq (__cov_v1di, __cov_v1di);
long long __builtin_ia32_psubq (__cov_v1di, __cov_v1di);
__cov_m64 __builtin_ia32_psrlq (__cov_v1di, __cov_v1di);
__cov_v4hi __builtin_ia32_psllw (__cov_v4hi, __cov_v4hi);
__cov_v2si __builtin_ia32_pslld (__cov_v2si, __cov_v2si);
__cov_m64 __builtin_ia32_psllq (__cov_v1di, __cov_v1di);
__cov_v4hi __builtin_ia32_psraw (__cov_v4hi, __cov_v4hi);
__cov_v2si __builtin_ia32_psrad (__cov_v2si, __cov_v2si);
__cov_m64 __builtin_ia32_psrlw (__cov_v4hi, __cov_v4hi);
__cov_v2si __builtin_ia32_psrld (__cov_v2si, __cov_v2si);
#else
long long __builtin_ia32_paddq (long long, long long);
long long __builtin_ia32_psubq (long long, long long);
long long __builtin_ia32_psrlq (long long, long long);
__cov_v4hi __builtin_ia32_psllw (__cov_v4hi, int);
__cov_v2si __builtin_ia32_pslld (__cov_v2si, int);
long long __builtin_ia32_psllq (long long, long long);
__cov_v4hi __builtin_ia32_psraw (__cov_v4hi, int);
__cov_v2si __builtin_ia32_psrad (__cov_v2si, int);
__cov_v4hi __builtin_ia32_psrlw (__cov_v4hi, int);
__cov_v2si __builtin_ia32_psrld (__cov_v2si, int);
#endif

#if __COVERITY_GCC_VERSION_AT_LEAST(4, 0)
// MMX G++ 4.x specific
__cov_m64 __builtin_ia32_vec_init_v2si (int, int);
int __builtin_ia32_vec_ext_v2si (__cov_v2si, int);
__cov_m64 __builtin_ia32_pand (__cov_m64, __cov_m64);
__cov_m64 __builtin_ia32_pandn (__cov_m64, __cov_m64);
__cov_m64 __builtin_ia32_por (__cov_m64, __cov_m64);
__cov_m64 __builtin_ia32_pxor (__cov_m64, __cov_m64);
__cov_m64 __builtin_ia32_vec_init_v4hi (short, short, short, short);
__cov_m64 __builtin_ia32_vec_init_v8qi (char, char, char, char,
										char, char, char, char);
__cov_m64 _mm_set_pi32 (int, int);
__cov_m64 _mm_set_pi16 (short, short, short, short);
__cov_m64 _mm_set_pi8(char, char, char, char,
					  char, char, char, char);

#if __COVERITY_GCC_VERSION_AT_LEAST(4, 4)
__cov_m64 __builtin_ia32_psllwi (__cov_v4hi, int);
__cov_m64 __builtin_ia32_pslldi (__cov_v2si, int);
__cov_m64 __builtin_ia32_psllqi (__cov_v1di, int);
__cov_m64 __builtin_ia32_psrlwi(__cov_v4hi, int);
__cov_m64 __builtin_ia32_psrldi (__cov_v2si, int);
__cov_m64 __builtin_ia32_psrlqi (__cov_v1di, int);
__cov_m64 __builtin_ia32_psrawi (__cov_v4hi, int);
__cov_m64 __builtin_ia32_psradi (__cov_v2si, int);
#endif

#else  // GNU 3.xx
long long __builtin_ia32_pand (long long, long long);
long long __builtin_ia32_pandn (long long, long long);
long long __builtin_ia32_por (long long, long long);
long long __builtin_ia32_pxor (long long, long long);
#endif  // 4.0+

int __builtin_ia32_comieq (__cov_v4sf, __cov_v4sf);
int __builtin_ia32_comineq (__cov_v4sf, __cov_v4sf);
int __builtin_ia32_comilt (__cov_v4sf, __cov_v4sf);
int __builtin_ia32_comile (__cov_v4sf, __cov_v4sf);
int __builtin_ia32_comigt (__cov_v4sf, __cov_v4sf);
int __builtin_ia32_comige (__cov_v4sf, __cov_v4sf);
int __builtin_ia32_ucomieq (__cov_v4sf, __cov_v4sf);
int __builtin_ia32_ucomineq (__cov_v4sf, __cov_v4sf);
int __builtin_ia32_ucomilt (__cov_v4sf, __cov_v4sf);
int __builtin_ia32_ucomile (__cov_v4sf, __cov_v4sf);
int __builtin_ia32_ucomigt (__cov_v4sf, __cov_v4sf);
int __builtin_ia32_ucomige (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_addps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_subps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_mulps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_divps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_addss (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_subss (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_mulss (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_divss (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpeqps (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpltps (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpleps (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpgtps (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpgeps (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpunordps (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpneqps (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpnltps (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpnleps (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpngtps (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpngeps (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpordps (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpeqss (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpltss (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpless (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpunordss (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpneqss (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpnlts (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpnless (__cov_v4sf, __cov_v4sf);
__cov_v4si __builtin_ia32_cmpordss (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_maxps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_maxss (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_minps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_minss (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_andps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_andnps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_orps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_xorps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_movss (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_movhlps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_movlhps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_unpckhps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_unpcklps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_cvtpi2ps (__cov_v4sf, __cov_v2si);
__cov_v4sf __builtin_ia32_cvtsi2ss (__cov_v4sf, int);
__cov_v2si __builtin_ia32_cvtps2pi (__cov_v4sf);
int __builtin_ia32_cvtss2si (__cov_v4sf);
__cov_v2si __builtin_ia32_cvttps2pi (__cov_v4sf);
int __builtin_ia32_cvttss2si (__cov_v4sf);
#ifdef __x86_64__
long long __builtin_ia32_cvtss2si64(__cov_v4sf);
long long __builtin_ia32_cvttss2si64(__cov_v4sf);
#endif
__cov_v4sf __builtin_ia32_rcpps (__cov_v4sf);
__cov_v4sf __builtin_ia32_rsqrtps (__cov_v4sf);
__cov_v4sf __builtin_ia32_sqrtps (__cov_v4sf);
__cov_v4sf __builtin_ia32_rcpss (__cov_v4sf);
__cov_v4sf __builtin_ia32_rsqrtss (__cov_v4sf);
__cov_v4sf __builtin_ia32_sqrtss (__cov_v4sf);
__cov_v4sf __builtin_ia32_shufps (__cov_v4sf, __cov_v4sf, int);
void __builtin_ia32_movntps (float *, __cov_v4sf);
int __builtin_ia32_movmskps (__cov_v4sf);

// extracted out of warnings in xmmintrin.h
__cov_v4sf __builtin_ia32_loadaps (float const *);
void __builtin_ia32_storeaps (float *, __cov_v4sf);  
__cov_m128 __builtin_ia32_cmpnltss(__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_setzerops ();
unsigned int  __builtin_ia32_stmxcsr ();
void __builtin_ia32_ldmxcsr (unsigned int);
__cov_v4sf __builtin_ia32_loadss (float const *);
__cov_m128 __builtin_ia32_loadups (float const *);
__cov_v4sf __builtin_ia32_loadss (float const *);
void __builtin_ia32_storess (float *, __cov_v4sf);
void __builtin_ia32_storeups (float *, __cov_v4sf);
__cov_m64 __builtin_ia32_pmaxsw (__cov_v4hi, __cov_v4hi);
__cov_m64 __builtin_ia32_pmaxub (__cov_v8qi, __cov_v8qi);
__cov_m64 __builtin_ia32_pminsw (__cov_v4hi, __cov_v4hi);
__cov_m64 __builtin_ia32_pminub (__cov_v8qi, __cov_v8qi);
int __builtin_ia32_pmovmskb (__cov_v8qi);
__cov_m64 __builtin_ia32_pmulhuw (__cov_v4hi, __cov_v4hi);
__cov_v8qi __builtin_ia32_maskmovq (__cov_v8qi, __cov_v8qi, char *);
__cov_m64 __builtin_ia32_pavgb (__cov_v8qi, __cov_v8qi);
__cov_m64 __builtin_ia32_pavgw (__cov_v4hi, __cov_v4hi);
__cov_m64 __builtin_ia32_psadbw (__cov_v8qi, __cov_v8qi);
void __builtin_ia32_movntq (unsigned long long *, unsigned long long);
void __builtin_ia32_sfence ();

#if __COVERITY_GCC_VERSION_AT_LEAST(4, 0)
float __builtin_ia32_vec_ext_v4sf (__cov_v4sf, int);
#endif // 4.0+

#if __COVERITY_GCC_VERSION_AT_LEAST(4, 4)
__cov_m128 __builtin_ia32_loadhps(__cov_v4sf, const __cov_v2sf *);
void __builtin_ia32_storehps (__cov_v2sf *, __cov_v4sf);
__cov_m128 __builtin_ia32_loadlps (__cov_v4sf, const __cov_v2sf *);
void __builtin_ia32_storelps (__cov_v2sf *, __cov_v4sf);
#else
__cov_m128 __builtin_ia32_loadhps(__cov_v4sf, __cov_v2si *);
void __builtin_ia32_storehps (__cov_v2si *, __cov_v4sf);
__cov_m128 __builtin_ia32_loadlps (__cov_v4sf, __cov_v2si *);
void __builtin_ia32_storelps (__cov_v2si *, __cov_v4sf);
#endif

int __builtin_ia32_comisdeq (__cov_v2df, __cov_v2df);
int __builtin_ia32_comisdlt (__cov_v2df, __cov_v2df);
int __builtin_ia32_comisdle (__cov_v2df, __cov_v2df);
int __builtin_ia32_comisdgt (__cov_v2df, __cov_v2df);
int __builtin_ia32_comisdge (__cov_v2df, __cov_v2df);
int __builtin_ia32_comisdneq (__cov_v2df, __cov_v2df);
int __builtin_ia32_ucomisdeq (__cov_v2df, __cov_v2df);
int __builtin_ia32_ucomisdlt (__cov_v2df, __cov_v2df);
int __builtin_ia32_ucomisdle (__cov_v2df, __cov_v2df);
int __builtin_ia32_ucomisdgt (__cov_v2df, __cov_v2df);
int __builtin_ia32_ucomisdge (__cov_v2df, __cov_v2df);
int __builtin_ia32_ucomisdneq (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpeqpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpltpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmplepd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpgtpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpgepd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpunordpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpneqpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpnltpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpnlepd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpngtpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpngepd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpordpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpeqsd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpltsd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmplesd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpunordsd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpneqsd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpnltsd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpnlesd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_cmpordsd (__cov_v2df, __cov_v2df);
//  __cov_v2di __builtin_ia32_paddq (__cov_v2di, __cov_v2di); // causes warning in gcc -msse2
//  __cov_v2di __builtin_ia32_psubq (__cov_v2di, __cov_v2di); // causes warning in gcc -mssse3
__cov_v2df __builtin_ia32_addpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_subpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_mulpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_divpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_addsd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_subsd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_mulsd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_divsd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_minpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_maxpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_minsd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_maxsd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_andpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_andnpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_orpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_xorpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_movsd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_unpckhpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_unpcklpd (__cov_v2df, __cov_v2df);
__cov_v16qi __builtin_ia32_paddb128 (__cov_v16qi, __cov_v16qi);
__cov_v8hi __builtin_ia32_paddw128 (__cov_v8hi, __cov_v8hi);
__cov_v4si __builtin_ia32_paddd128 (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_paddq128 (__cov_v2di, __cov_v2di);
__cov_v16qi __builtin_ia32_psubb128 (__cov_v16qi, __cov_v16qi);
__cov_v8hi __builtin_ia32_psubw128 (__cov_v8hi, __cov_v8hi);
__cov_v4si __builtin_ia32_psubd128 (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_psubq128 (__cov_v2di, __cov_v2di);
__cov_v8hi __builtin_ia32_pmullw128 (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_pmulhw128 (__cov_v8hi, __cov_v8hi);
__cov_v2di __builtin_ia32_pand128 (__cov_v2di, __cov_v2di);
__cov_v2di __builtin_ia32_pandn128 (__cov_v2di, __cov_v2di);
__cov_v2di __builtin_ia32_por128 (__cov_v2di, __cov_v2di);
__cov_v2di __builtin_ia32_pxor128 (__cov_v2di, __cov_v2di);
__cov_v16qi __builtin_ia32_pavgb128 (__cov_v16qi, __cov_v16qi);
__cov_v8hi __builtin_ia32_pavgw128 (__cov_v8hi, __cov_v8hi);
__cov_v16qi __builtin_ia32_pcmpeqb128 (__cov_v16qi, __cov_v16qi);
__cov_v8hi __builtin_ia32_pcmpeqw128 (__cov_v8hi, __cov_v8hi);
__cov_v4si __builtin_ia32_pcmpeqd128 (__cov_v4si, __cov_v4si);
__cov_v16qi __builtin_ia32_pcmpgtb128 (__cov_v16qi, __cov_v16qi);
__cov_v8hi __builtin_ia32_pcmpgtw128 (__cov_v8hi, __cov_v8hi);
__cov_v4si __builtin_ia32_pcmpgtd128 (__cov_v4si, __cov_v4si);
__cov_v16qi __builtin_ia32_pmaxub128 (__cov_v16qi, __cov_v16qi);
__cov_v8hi __builtin_ia32_pmaxsw128 (__cov_v8hi, __cov_v8hi);
__cov_v16qi __builtin_ia32_pminub128 (__cov_v16qi, __cov_v16qi);
__cov_v8hi __builtin_ia32_pminsw128 (__cov_v8hi, __cov_v8hi);
__cov_v16qi __builtin_ia32_punpckhbw128 (__cov_v16qi, __cov_v16qi);
__cov_v8hi __builtin_ia32_punpckhwd128 (__cov_v8hi, __cov_v8hi);
__cov_v4si __builtin_ia32_punpckhdq128 (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_punpckhqdq128 (__cov_v2di, __cov_v2di);
__cov_v16qi __builtin_ia32_punpcklbw128 (__cov_v16qi, __cov_v16qi);
__cov_v8hi __builtin_ia32_punpcklwd128 (__cov_v8hi, __cov_v8hi);
__cov_v4si __builtin_ia32_punpckldq128 (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_punpcklqdq128 (__cov_v2di, __cov_v2di);
__cov_v16qi __builtin_ia32_packsswb128 (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_packssdw128 (__cov_v4si, __cov_v4si);
__cov_v16qi __builtin_ia32_packuswb128 (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_pmulhuw128 (__cov_v8hi, __cov_v8hi);
void __builtin_ia32_maskmovdqu (__cov_v16qi, __cov_v16qi, char * c ); // char * c=0
void __builtin_ia32_storeupd (double *, __cov_v2df);
#if __COVERITY_GCC_VERSION_AT_LEAST(4, 0)
__cov_v2df __builtin_ia32_loadhpd (__cov_v2df, double const *);
__cov_v2df __builtin_ia32_loadlpd (__cov_v2df, double const *);
__cov_m128d __builtin_ia32_loadupd (double const *);
#else
__cov_v2df __builtin_ia32_loadhpd (__cov_v2df, __cov_v2si *);
__cov_v2df __builtin_ia32_loadlpd (__cov_v2df, __cov_v2si *);
__cov_v2df __builtin_ia32_loadupd (double const *);
#endif
int __builtin_ia32_movmskpd (__cov_v2df);
int __builtin_ia32_pmovmskb128 (__cov_v16qi);
void __builtin_ia32_movnti (int *, int);
void __builtin_ia32_movnti64 (long long int *, long long int);
void __builtin_ia32_movntpd (double *, __cov_v2df);
void __builtin_ia32_movntdq (__cov_v2di *, __cov_v2di); 
__cov_v4si __builtin_ia32_pshufd (__cov_v4si, int);
__cov_v8hi __builtin_ia32_pshuflw (__cov_v8hi, int);
__cov_v8hi __builtin_ia32_pshufhw (__cov_v8hi, int);
__cov_v2di __builtin_ia32_psadbw128 (__cov_v16qi, __cov_v16qi);
__cov_v2df __builtin_ia32_sqrtpd (__cov_v2df);
__cov_v2df __builtin_ia32_sqrtsd (__cov_v2df);
__cov_v2df __builtin_ia32_shufpd (__cov_v2df, __cov_v2df, int);
__cov_v2df __builtin_ia32_cvtdq2pd (__cov_v4si);
__cov_v4sf __builtin_ia32_cvtdq2ps (__cov_v4si);
__cov_v4si __builtin_ia32_cvtpd2dq (__cov_v2df);
__cov_v2si __builtin_ia32_cvtpd2pi (__cov_v2df);
__cov_v4sf __builtin_ia32_cvtpd2ps (__cov_v2df);
__cov_v4si __builtin_ia32_cvttpd2dq (__cov_v2df);
__cov_v2si __builtin_ia32_cvttpd2pi (__cov_v2df);
__cov_v2df __builtin_ia32_cvtpi2pd (__cov_v2si);
int __builtin_ia32_cvtsd2si (__cov_v2df);
int __builtin_ia32_cvttsd2si (__cov_v2df);
long long __builtin_ia32_cvtsd2si64 (__cov_v2df);
long long __builtin_ia32_cvttsd2si64 (__cov_v2df);
__cov_v4si __builtin_ia32_cvtps2dq (__cov_v4sf);
__cov_v2df __builtin_ia32_cvtps2pd (__cov_v4sf);
__cov_v4si __builtin_ia32_cvttps2dq (__cov_v4sf);
__cov_v2df __builtin_ia32_cvtsi2sd (__cov_v2df, int);
__cov_v2df __builtin_ia32_cvtsi642sd (__cov_v2df, long long);
__cov_v4sf __builtin_ia32_cvtsi642ss (__cov_v4sf, long long);
__cov_v4sf __builtin_ia32_cvtsd2ss (__cov_v4sf, __cov_v2df);
__cov_v2df __builtin_ia32_cvtss2sd (__cov_v2df, __cov_v4sf);
void __builtin_ia32_clflush (const void *);
void __builtin_ia32_lfence (void);
void __builtin_ia32_mfence (void);
__cov_v16qi __builtin_ia32_loaddqu (const char *);
void __builtin_ia32_storedqu (char *, __cov_v16qi);
__cov_v1di __builtin_ia32_pmuludq (__cov_v2si, __cov_v2si); // was ret type v1di
__cov_v2di __builtin_ia32_pmuludq128 (__cov_v4si, __cov_v4si);
#if __COVERITY_GCC_VERSION_AT_LEAST(4, 0)
__cov_v8hi __builtin_ia32_psllw128 (__cov_v8hi, __cov_v8hi);
__cov_v4si __builtin_ia32_pslld128 (__cov_v4si, __cov_v4si);
__cov_v8hi __builtin_ia32_psrlw128 (__cov_v8hi, __cov_v8hi);
__cov_v4si __builtin_ia32_psrld128 (__cov_v4si, __cov_v4si);
__cov_v8hi __builtin_ia32_psraw128 (__cov_v8hi, __cov_v8hi);
__cov_v4si __builtin_ia32_psrad128 (__cov_v4si, __cov_v4si);
#else
// The following intrinsics deviate from the manual in pre4, see BZ 18359
__cov_v8hi __builtin_ia32_psllw128 (__cov_v8hi, __cov_v2di);
__cov_v4si __builtin_ia32_pslld128 (__cov_v4si, __cov_v2di);
__cov_v8hi __builtin_ia32_psrlw128 (__cov_v8hi, __cov_v2di);
__cov_v4si __builtin_ia32_psrld128 (__cov_v4si, __cov_v2di);
__cov_v8hi __builtin_ia32_psraw128 (__cov_v8hi, __cov_v2di);
__cov_v4si __builtin_ia32_psrad128 (__cov_v4si, __cov_v2di);
#endif
__cov_v2di __builtin_ia32_psllq128 (__cov_v2di, __cov_v2di);
__cov_v2di __builtin_ia32_psrlq128 (__cov_v2di, __cov_v2di);
__cov_v2di __builtin_ia32_pslldqi128 (__cov_v2di, int);
__cov_v8hi __builtin_ia32_psllwi128 (__cov_v8hi, int);
__cov_v4si __builtin_ia32_pslldi128 (__cov_v4si, int);
__cov_v2di __builtin_ia32_psllqi128 (__cov_v2di, int);
__cov_v2di __builtin_ia32_psrldqi128 (__cov_v2di, int);
__cov_v8hi __builtin_ia32_psrlwi128 (__cov_v8hi, int);
__cov_v4si __builtin_ia32_psrldi128 (__cov_v4si, int);
__cov_v2di __builtin_ia32_psrlqi128 (__cov_v2di, int);
__cov_v8hi __builtin_ia32_psrawi128 (__cov_v8hi, int);
__cov_v4si __builtin_ia32_psradi128 (__cov_v4si, int);
__cov_v4si __builtin_ia32_pmaddwd128 (__cov_v8hi, __cov_v8hi);
__cov_v2di __builtin_ia32_movq128 (__cov_v2di);

__cov_v2df  __builtin_ia32_loadsd (double const *);
__cov_v2df  __builtin_ia32_loadapd (double const *);
__cov_v4si  __builtin_ia32_loadd (int *);
__cov_m128i __builtin_ia32_loaddqa (char const *);
/* __cov_m128i __builtin_ia32_loaddqu (char const *); */
#if defined(__APPLE__)
__cov_m128i __builtin_ia32_loadlv4si(__cov_v2si*);
void __builtin_ia32_storelv4si (__cov_v2si *,__cov_m128i);
__cov_m128i __builtin_ia32_movqv4si (__cov_v4si);
#endif
__cov_m128d __builtin_ia32_setzeropd ();
__cov_m128i __builtin_ia32_setzero128 ();

void __builtin_ia32_storesd (double *, __cov_v2df);
void __builtin_ia32_storeapd (double *, __cov_v2df);
void __builtin_ia32_storedqa (char *, __cov_v16qi);
void __builtin_ia32_storedqu (char *, __cov_v16qi);

__cov_v2di __builtin_ia32_movq2dq (unsigned long long);
long long  __builtin_ia32_movdq2q (__cov_v2di); 
/*__cov_m64 __builtin_ia32_movdq2q (__cov_v2di);  */
__cov_m128i __builtin_ia32_movq (__cov_v2di);


void __builtin_ia32_storehpd (__cov_v2si *, __cov_v2df);
void __builtin_ia32_storelpd (__cov_v2si *, __cov_v2df);
void __builtin_ia32_stored (int *, __cov_v4si);
__cov_m128i __builtin_ia32_paddsb128 (__cov_v16qi, __cov_v16qi);
__cov_m128i __builtin_ia32_paddsw128 (__cov_v8hi, __cov_v8hi);
__cov_m128i __builtin_ia32_paddusb128 (__cov_v16qi, __cov_v16qi);
__cov_m128i __builtin_ia32_paddusw128 (__cov_v8hi, __cov_v8hi);
__cov_m128i __builtin_ia32_psubsb128 (__cov_v16qi, __cov_v16qi);
__cov_m128i __builtin_ia32_psubsw128 (__cov_v8hi, __cov_v8hi);
__cov_m128i __builtin_ia32_psubusb128 (__cov_v16qi, __cov_v16qi);
__cov_m128i __builtin_ia32_psubusw128 (__cov_v8hi, __cov_v8hi);

#if __COVERITY_GCC_VERSION_AT_LEAST(4, 0)
double __builtin_ia32_vec_ext_v2df (__cov_m128d, int);
int __builtin_ia32_vec_ext_v4si (__cov_v4si, int);
long long __builtin_ia32_vec_ext_v2di (__cov_v2di, int); // ret: __m64 at line emmintrin.h:717
#endif // 4.0+


__cov_v2df __builtin_ia32_addsubpd (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_addsubps (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_haddpd (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_haddps (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_hsubpd (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_hsubps (__cov_v4sf, __cov_v4sf);
__cov_v16qi __builtin_ia32_lddqu (char const *);
void __builtin_ia32_monitor (const void *, unsigned int, unsigned int);
__cov_v2df __builtin_ia32_movddup (__cov_v2df);
__cov_v4sf __builtin_ia32_movshdup (__cov_v4sf);
__cov_v4sf __builtin_ia32_movsldup (__cov_v4sf);
void __builtin_ia32_mwait (unsigned int, unsigned int);

__cov_v2df __builtin_ia32_loadddup (double const *);

// MMX regs
__cov_v2si __builtin_ia32_phaddd (__cov_v2si, __cov_v2si);
__cov_v4hi __builtin_ia32_phaddw (__cov_v4hi, __cov_v4hi);
__cov_v4hi __builtin_ia32_phaddsw (__cov_v4hi, __cov_v4hi);
__cov_v2si __builtin_ia32_phsubd (__cov_v2si, __cov_v2si);
__cov_v4hi __builtin_ia32_phsubw (__cov_v4hi, __cov_v4hi);
__cov_v4hi __builtin_ia32_phsubsw (__cov_v4hi, __cov_v4hi);
__cov_v4hi __builtin_ia32_pmaddubsw (__cov_v8qi, __cov_v8qi);
__cov_v4hi __builtin_ia32_pmulhrsw (__cov_v4hi, __cov_v4hi);
__cov_v8qi __builtin_ia32_pshufb (__cov_v8qi, __cov_v8qi);
__cov_v8qi __builtin_ia32_psignb (__cov_v8qi, __cov_v8qi);
__cov_v2si __builtin_ia32_psignd (__cov_v2si, __cov_v2si);
__cov_v4hi __builtin_ia32_psignw (__cov_v4hi, __cov_v4hi);
__cov_v1di __builtin_ia32_palignr (__cov_v1di, __cov_v1di, int);
__cov_v8qi __builtin_ia32_pabsb (__cov_v8qi);
__cov_v2si __builtin_ia32_pabsd (__cov_v2si);
__cov_v4hi __builtin_ia32_pabsw (__cov_v4hi);

// SSE reg
__cov_v4si __builtin_ia32_phaddd128 (__cov_v4si, __cov_v4si);
__cov_v8hi __builtin_ia32_phaddw128 (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_phaddsw128 (__cov_v8hi, __cov_v8hi);
__cov_v4si __builtin_ia32_phsubd128 (__cov_v4si, __cov_v4si);
__cov_v8hi __builtin_ia32_phsubw128 (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_phsubsw128 (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_pmaddubsw128 (__cov_v16qi, __cov_v16qi);
__cov_v8hi __builtin_ia32_pmulhrsw128 (__cov_v8hi, __cov_v8hi);
__cov_v16qi __builtin_ia32_pshufb128 (__cov_v16qi, __cov_v16qi);
__cov_v16qi __builtin_ia32_psignb128 (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_psignd128 (__cov_v4si, __cov_v4si);
__cov_v8hi __builtin_ia32_psignw128 (__cov_v8hi, __cov_v8hi);
__cov_v2di __builtin_ia32_palignr128 (__cov_v2di, __cov_v2di, int);
__cov_v16qi __builtin_ia32_pabsb128 (__cov_v16qi);
__cov_v4si __builtin_ia32_pabsd128 (__cov_v4si);
__cov_v8hi __builtin_ia32_pabsw128 (__cov_v8hi);

__cov_v2si __builtin_ia32_phaddd (__cov_v2si, __cov_v2si);
__cov_v4hi __builtin_ia32_phaddw (__cov_v4hi, __cov_v4hi);
__cov_v4hi __builtin_ia32_phaddsw (__cov_v4hi, __cov_v4hi);
__cov_v2si __builtin_ia32_phsubd (__cov_v2si, __cov_v2si);
__cov_v4hi __builtin_ia32_phsubw (__cov_v4hi, __cov_v4hi);
__cov_v4hi __builtin_ia32_phsubsw (__cov_v4hi, __cov_v4hi);
__cov_v4hi __builtin_ia32_pmaddubsw (__cov_v8qi, __cov_v8qi);
__cov_v4hi __builtin_ia32_pmulhrsw (__cov_v4hi, __cov_v4hi);
__cov_v8qi __builtin_ia32_pshufb (__cov_v8qi, __cov_v8qi);
__cov_v8qi __builtin_ia32_psignb (__cov_v8qi, __cov_v8qi);
__cov_v2si __builtin_ia32_psignd (__cov_v2si, __cov_v2si);
__cov_v4hi __builtin_ia32_psignw (__cov_v4hi, __cov_v4hi);
__cov_v1di __builtin_ia32_palignr (__cov_v1di, __cov_v1di, int);
__cov_v8qi __builtin_ia32_pabsb (__cov_v8qi);
__cov_v2si __builtin_ia32_pabsd (__cov_v2si);
__cov_v4hi __builtin_ia32_pabsw (__cov_v4hi);

__cov_v4si __builtin_ia32_phaddd128 (__cov_v4si, __cov_v4si);
__cov_v8hi __builtin_ia32_phaddw128 (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_phaddsw128 (__cov_v8hi, __cov_v8hi);
__cov_v4si __builtin_ia32_phsubd128 (__cov_v4si, __cov_v4si);
__cov_v8hi __builtin_ia32_phsubw128 (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_phsubsw128 (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_pmaddubsw128 (__cov_v16qi, __cov_v16qi);
__cov_v8hi __builtin_ia32_pmulhrsw128 (__cov_v8hi, __cov_v8hi);
__cov_v16qi __builtin_ia32_pshufb128 (__cov_v16qi, __cov_v16qi);
__cov_v16qi __builtin_ia32_psignb128 (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_psignd128 (__cov_v4si, __cov_v4si);
__cov_v8hi __builtin_ia32_psignw128 (__cov_v8hi, __cov_v8hi);
__cov_v2di __builtin_ia32_palignr128 (__cov_v2di, __cov_v2di, int);
__cov_v16qi __builtin_ia32_pabsb128 (__cov_v16qi);
__cov_v4si __builtin_ia32_pabsd128 (__cov_v4si);
__cov_v8hi __builtin_ia32_pabsw128 (__cov_v8hi);

#if __COVERITY_GCC_VERSION_AT_LEAST(4, 0)
__cov_v2df __builtin_ia32_blendpd (__cov_v2df, __cov_v2df, const int);
__cov_v4sf __builtin_ia32_blendps (__cov_v4sf, __cov_v4sf, const int);
__cov_v2df __builtin_ia32_blendvpd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_blendvps (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_dppd (__cov_v2df, __cov_v2df, const int);
__cov_v4sf __builtin_ia32_dpps (__cov_v4sf, __cov_v4sf, const int);
__cov_v4sf __builtin_ia32_insertps128 (__cov_v4sf, __cov_v4sf, const int);
__cov_v2di __builtin_ia32_movntdqa (__cov_v2di *);
__cov_v16qi __builtin_ia32_mpsadbw128 (__cov_v16qi, __cov_v16qi, const int);
__cov_v8hi __builtin_ia32_packusdw128 (__cov_v4si, __cov_v4si);
__cov_v16qi __builtin_ia32_pblendvb128 (__cov_v16qi, __cov_v16qi, __cov_v16qi);
__cov_v8hi __builtin_ia32_pblendw128 (__cov_v8hi, __cov_v8hi, const int);
__cov_v2di __builtin_ia32_pcmpeqq (__cov_v2di, __cov_v2di);
__cov_v8hi __builtin_ia32_phminposuw128 (__cov_v8hi);
__cov_v16qi __builtin_ia32_pmaxsb128 (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pmaxsd128 (__cov_v4si, __cov_v4si);
__cov_v4si __builtin_ia32_pmaxud128 (__cov_v4si, __cov_v4si);
__cov_v8hi __builtin_ia32_pmaxuw128 (__cov_v8hi, __cov_v8hi);
__cov_v16qi __builtin_ia32_pminsb128 (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pminsd128 (__cov_v4si, __cov_v4si);
__cov_v4si __builtin_ia32_pminud128 (__cov_v4si, __cov_v4si);
__cov_v8hi __builtin_ia32_pminuw128 (__cov_v8hi, __cov_v8hi);
__cov_v4si __builtin_ia32_pmovsxbd128 (__cov_v16qi);
__cov_v2di __builtin_ia32_pmovsxbq128 (__cov_v16qi);
__cov_v8hi __builtin_ia32_pmovsxbw128 (__cov_v16qi);
__cov_v2di __builtin_ia32_pmovsxdq128 (__cov_v4si);
__cov_v4si __builtin_ia32_pmovsxwd128 (__cov_v8hi);
__cov_v2di __builtin_ia32_pmovsxwq128 (__cov_v8hi);
__cov_v4si __builtin_ia32_pmovzxbd128 (__cov_v16qi);
__cov_v2di __builtin_ia32_pmovzxbq128 (__cov_v16qi);
__cov_v8hi __builtin_ia32_pmovzxbw128 (__cov_v16qi);
__cov_v2di __builtin_ia32_pmovzxdq128 (__cov_v4si);
__cov_v4si __builtin_ia32_pmovzxwd128 (__cov_v8hi);
__cov_v2di __builtin_ia32_pmovzxwq128 (__cov_v8hi);
__cov_v2di __builtin_ia32_pmuldq128 (__cov_v4si, __cov_v4si);
__cov_v4si __builtin_ia32_pmulld128 (__cov_v4si, __cov_v4si);


__cov_v4sf __builtin_ia32_vec_set_v4sf (__cov_v4sf, float, const int);
//    Generates the insertps machine instruction.
int __builtin_ia32_vec_ext_v16qi (__cov_v16qi, const int);
//    Generates the pextrb machine instruction.
__cov_v16qi __builtin_ia32_vec_set_v16qi (__cov_v16qi, int, const int);
//    Generates the pinsrb machine instruction.
__cov_v4si __builtin_ia32_vec_set_v4si (__cov_v4si, int, const int);
//    Generates the pinsrd machine instruction.
__cov_v2di __builtin_ia32_vec_set_v2di (__cov_v2di, long long, const int);
//    Generates the pinsrq machine instruction in 64bit mode. 

float __builtin_ia32_vec_ext_v4sf (__cov_v4sf, const int);
//    Generates the extractps machine instruction.
int __builtin_ia32_vec_ext_v4si (__cov_v4si, const int);
//    Generates the pextrd machine instruction.
long long __builtin_ia32_vec_ext_v2di (__cov_v2di, const int);
//    Generates the pextrq machine instruction in 64bit mode. 
#endif // GNU4

#if __COVERITY_GCC_VERSION_AT_LEAST(4, 0)
int __builtin_ia32_ptestc128 (__cov_v2di, __cov_v2di);
int __builtin_ia32_ptestnzc128 (__cov_v2di, __cov_v2di);
int __builtin_ia32_ptestz128 (__cov_v2di, __cov_v2di); //common SSE 5
__cov_v2df __builtin_ia32_roundpd (__cov_v2df, const int);
__cov_v4sf __builtin_ia32_roundps (__cov_v4sf, const int);
__cov_v2df __builtin_ia32_roundsd (__cov_v2df, __cov_v2df, const int);
__cov_v4sf __builtin_ia32_roundss (__cov_v4sf, __cov_v4sf, const int);

__cov_v16qi __builtin_ia32_pcmpestrm128 (__cov_v16qi, int, __cov_v16qi, int, const int);
int __builtin_ia32_pcmpestri128 (__cov_v16qi, int, __cov_v16qi, int, const int);
int __builtin_ia32_pcmpestria128 (__cov_v16qi, int, __cov_v16qi, int, const int);
int __builtin_ia32_pcmpestric128 (__cov_v16qi, int, __cov_v16qi, int, const int);
int __builtin_ia32_pcmpestrio128 (__cov_v16qi, int, __cov_v16qi, int, const int);
int __builtin_ia32_pcmpestris128 (__cov_v16qi, int, __cov_v16qi, int, const int);
int __builtin_ia32_pcmpestriz128 (__cov_v16qi, int, __cov_v16qi, int, const int);
__cov_v16qi __builtin_ia32_pcmpistrm128 (__cov_v16qi, __cov_v16qi, const int);
int __builtin_ia32_pcmpistri128 (__cov_v16qi, __cov_v16qi, const int);
int __builtin_ia32_pcmpistria128 (__cov_v16qi, __cov_v16qi, const int);
int __builtin_ia32_pcmpistric128 (__cov_v16qi, __cov_v16qi, const int);
int __builtin_ia32_pcmpistrio128 (__cov_v16qi, __cov_v16qi, const int);
int __builtin_ia32_pcmpistris128 (__cov_v16qi, __cov_v16qi, const int);
int __builtin_ia32_pcmpistriz128 (__cov_v16qi, __cov_v16qi, const int);
__cov_v2di __builtin_ia32_pcmpgtq (__cov_v2di, __cov_v2di);
unsigned int __builtin_ia32_crc32qi (unsigned int, unsigned char);
unsigned int __builtin_ia32_crc32hi (unsigned int, unsigned short);
unsigned int __builtin_ia32_crc32si (unsigned int, unsigned int);
unsigned long long __builtin_ia32_crc32di (unsigned long long, unsigned long long);

int __builtin_popcount (unsigned int);
int __builtin_popcountl (unsigned long);
int __builtin_popcountll (unsigned long long);

void __builtin_ia32_movntsd (double *, __cov_v2df);
void __builtin_ia32_movntss (float *, __cov_v4sf);
__cov_v2di __builtin_ia32_extrq  (__cov_v2di, __cov_v16qi);
__cov_v2di __builtin_ia32_extrqi (__cov_v2di, const unsigned int, const unsigned int);
__cov_v2di __builtin_ia32_insertq (__cov_v2di, __cov_v2di);
__cov_v2di __builtin_ia32_insertqi (__cov_v2di, __cov_v2di, const unsigned int, const unsigned int);
__cov_v2df __builtin_ia32_comeqpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_comeqps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comeqsd (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_comeqss (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_comfalsepd (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_comfalsesd (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_comfalsess (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_comgepd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_comgeps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comgesd (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_comgess (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_comgtpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_comgtps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comgtsd (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_comgtss (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_comlepd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_comleps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comless (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_comltpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_comltps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comltsd (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_comltss (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_comnepd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_comneps (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_comnesd (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comness (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_comordpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_comordps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comordss (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_comtruepd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_comtrueps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comtruesd (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_comtruess (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_comueqpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_comueqps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comueqss (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_comugepd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_comugeps (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_comugesd (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comugess (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_comugtpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_comugtps (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_comugtsd (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comugtss (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_comulepd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_comuleps (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_comulesd (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comuless (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_comultpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_comultps (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_comultsd (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comultss (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_comunepd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_comuneps (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_comunesd (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comuness (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_comunordpd (__cov_v2df, __cov_v2df);
__cov_v2df __builtin_ia32_comunordps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comunordsd (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_comunordss (__cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_fmaddpd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_fmaddps (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_fmaddsd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_fmaddss (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_fmsubpd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_fmsubps (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_fmsubsd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_fmsubss (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_fnmaddpd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_fnmaddps (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_fnmaddsd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_fnmaddss (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_fnmsubpd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_fnmsubps (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_fnmsubsd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_fnmsubss (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_v2df __builtin_ia32_frczpd (__cov_v2df);
__cov_v4sf __builtin_ia32_frczps (__cov_v4sf);
__cov_v2df __builtin_ia32_frczsd (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_frczss (__cov_v4sf, __cov_v4sf);
__cov_v2di __builtin_ia32_pcmov (__cov_v2di, __cov_v2di, __cov_v2di);
__cov_v2di __builtin_ia32_pcmov_v2di (__cov_v2di, __cov_v2di, __cov_v2di);
__cov_v4si __builtin_ia32_pcmov_v4si (__cov_v4si, __cov_v4si, __cov_v4si);
__cov_v8hi __builtin_ia32_pcmov_v8hi (__cov_v8hi, __cov_v8hi, __cov_v8hi);
__cov_v16qi __builtin_ia32_pcmov_v16qi (__cov_v16qi, __cov_v16qi, __cov_v16qi);
__cov_v2df __builtin_ia32_pcmov_v2df (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_pcmov_v4sf (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_v16qi __builtin_ia32_pcomeqb (__cov_v16qi, __cov_v16qi);
__cov_v8hi __builtin_ia32_pcomeqw (__cov_v8hi, __cov_v8hi);
__cov_v4si __builtin_ia32_pcomeqd (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pcomeqq (__cov_v2di, __cov_v2di);
__cov_v16qi __builtin_ia32_pcomequb (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pcomequd (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pcomequq (__cov_v2di, __cov_v2di);
__cov_v8hi __builtin_ia32_pcomequw (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_pcomeqw (__cov_v8hi, __cov_v8hi);
__cov_v16qi __builtin_ia32_pcomfalseb (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pcomfalsed (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pcomfalseq (__cov_v2di, __cov_v2di);
__cov_v16qi __builtin_ia32_pcomfalseub (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pcomfalseud (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pcomfalseuq (__cov_v2di, __cov_v2di);
__cov_v8hi __builtin_ia32_pcomfalseuw (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_pcomfalsew (__cov_v8hi, __cov_v8hi);
__cov_v16qi __builtin_ia32_pcomgeb (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pcomged (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pcomgeq (__cov_v2di, __cov_v2di);
__cov_v16qi __builtin_ia32_pcomgeub (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pcomgeud (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pcomgeuq (__cov_v2di, __cov_v2di);
__cov_v8hi __builtin_ia32_pcomgeuw (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_pcomgew (__cov_v8hi, __cov_v8hi);
__cov_v16qi __builtin_ia32_pcomgtb (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pcomgtd (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pcomgtq (__cov_v2di, __cov_v2di);
__cov_v16qi __builtin_ia32_pcomgtub (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pcomgtud (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pcomgtuq (__cov_v2di, __cov_v2di);
__cov_v8hi __builtin_ia32_pcomgtuw (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_pcomgtw (__cov_v8hi, __cov_v8hi);
__cov_v16qi __builtin_ia32_pcomleb (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pcomled (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pcomleq (__cov_v2di, __cov_v2di);
__cov_v16qi __builtin_ia32_pcomleub (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pcomleud (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pcomleuq (__cov_v2di, __cov_v2di);
__cov_v8hi __builtin_ia32_pcomleuw (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_pcomlew (__cov_v8hi, __cov_v8hi);
__cov_v16qi __builtin_ia32_pcomltb (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pcomltd (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pcomltq (__cov_v2di, __cov_v2di);
__cov_v16qi __builtin_ia32_pcomltub (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pcomltud (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pcomltuq (__cov_v2di, __cov_v2di);
__cov_v8hi __builtin_ia32_pcomltuw (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_pcomltw (__cov_v8hi, __cov_v8hi);
__cov_v16qi __builtin_ia32_pcomneb (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pcomned (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pcomneq (__cov_v2di, __cov_v2di);
__cov_v16qi __builtin_ia32_pcomneub (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pcomneud (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pcomneuq (__cov_v2di, __cov_v2di);
__cov_v8hi __builtin_ia32_pcomneuw (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_pcomnew (__cov_v8hi, __cov_v8hi);
__cov_v16qi __builtin_ia32_pcomtrueb (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pcomtrued (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pcomtrueq (__cov_v2di, __cov_v2di);
__cov_v16qi __builtin_ia32_pcomtrueub (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pcomtrueud (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pcomtrueuq (__cov_v2di, __cov_v2di);
__cov_v8hi __builtin_ia32_pcomtrueuw (__cov_v8hi, __cov_v8hi);
__cov_v8hi __builtin_ia32_pcomtruew (__cov_v8hi, __cov_v8hi);
__cov_m128d __builtin_ia32_permpd (__cov_v2df, __cov_v2df, __cov_v16qi);
__cov_v4sf __builtin_ia32_permps (__cov_v4sf, __cov_v4sf, __cov_v16qi);
__cov_v4si __builtin_ia32_phaddbd (__cov_v16qi);
__cov_v2di __builtin_ia32_phaddbq (__cov_v16qi);
__cov_v8hi __builtin_ia32_phaddbw (__cov_v16qi);
__cov_v2di __builtin_ia32_phadddq (__cov_v4si);
__cov_v4si __builtin_ia32_phaddubd (__cov_v16qi);
__cov_v2di __builtin_ia32_phaddubq (__cov_v16qi);
__cov_v8hi __builtin_ia32_phaddubw (__cov_v16qi);
__cov_v2di __builtin_ia32_phaddudq (__cov_v4si);
__cov_v4si __builtin_ia32_phadduwd (__cov_v8hi);
__cov_v2di __builtin_ia32_phadduwq (__cov_v8hi);
__cov_v4si __builtin_ia32_phaddwd (__cov_v8hi);
__cov_v2di __builtin_ia32_phaddwq (__cov_v8hi);
__cov_v8hi __builtin_ia32_phsubbw (__cov_v16qi);
__cov_v2di __builtin_ia32_phsubdq (__cov_v4si);
__cov_v4si __builtin_ia32_phsubwd (__cov_v8hi);
__cov_v4si __builtin_ia32_pmacsdd (__cov_v4si, __cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pmacsdqh (__cov_v4si, __cov_v4si, __cov_v2di);
__cov_v2di __builtin_ia32_pmacsdql (__cov_v4si, __cov_v4si, __cov_v2di);
__cov_v4si __builtin_ia32_pmacssdd (__cov_v4si, __cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pmacssdqh (__cov_v4si, __cov_v4si, __cov_v2di);
__cov_v2di __builtin_ia32_pmacssdql (__cov_v4si, __cov_v4si, __cov_v2di);
__cov_v4si __builtin_ia32_pmacsswd (__cov_v8hi, __cov_v8hi, __cov_v4si);
__cov_v8hi __builtin_ia32_pmacssww (__cov_v8hi, __cov_v8hi, __cov_v8hi);
__cov_v4si __builtin_ia32_pmacswd (__cov_v8hi, __cov_v8hi, __cov_v4si);
__cov_v8hi __builtin_ia32_pmacsww (__cov_v8hi, __cov_v8hi, __cov_v8hi);
__cov_v4si __builtin_ia32_pmadcsswd (__cov_v8hi, __cov_v8hi, __cov_v4si);
__cov_v4si __builtin_ia32_pmadcswd (__cov_v8hi, __cov_v8hi, __cov_v4si);
__cov_v16qi __builtin_ia32_pperm (__cov_v16qi, __cov_v16qi, __cov_v16qi);
__cov_v16qi __builtin_ia32_protb (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_protd (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_protq (__cov_v2di, __cov_v2di);
__cov_v8hi __builtin_ia32_protw (__cov_v8hi, __cov_v8hi);
__cov_v16qi __builtin_ia32_pshab (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pshad (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pshaq (__cov_v2di, __cov_v2di);
__cov_v8hi __builtin_ia32_pshaw (__cov_v8hi, __cov_v8hi);
__cov_v16qi __builtin_ia32_pshlb (__cov_v16qi, __cov_v16qi);
__cov_v4si __builtin_ia32_pshld (__cov_v4si, __cov_v4si);
__cov_v2di __builtin_ia32_pshlq (__cov_v2di, __cov_v2di);
__cov_v8hi __builtin_ia32_pshlw (__cov_v8hi, __cov_v8hi);

__cov_v16qi __builtin_ia32_protb_int (__cov_v16qi, int);
__cov_v4si __builtin_ia32_protd_int (__cov_v4si, int);
__cov_v2di __builtin_ia32_protq_int (__cov_v2di, int);
__cov_v8hi __builtin_ia32_protw_int (__cov_v8hi, int);

// These have different possibilities for parameters, take a guess based on
//   OS or allow all if C++
#if defined(__unix) || defined(__cplusplus)
__cov_v2df __builtin_ia32_comfalseps (__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_comlesd (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comordsd (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comueqsd (__cov_v4sf, __cov_v4sf);
#endif

#if !defined(__unix) || defined(__cplusplus)
__cov_m128 __builtin_ia32_comfalseps (__cov_v4sf, __cov_v4sf);
__cov_v4sf __builtin_ia32_comlesd (__cov_v2df, __cov_v2df);
__cov_m128d __builtin_ia32_comordsd (__cov_v2df, __cov_v2df);
__cov_m128d __builtin_ia32_comueqsd (__cov_v2df, __cov_v2df);
#endif

// from headers
__cov_m128 __builtin_ia32_comunltps (__cov_v4sf, __cov_v4sf);
__cov_m128 __builtin_ia32_comuneqps (__cov_v4sf, __cov_v4sf);
__cov_m128  __builtin_ia32_comunleps (__cov_v4sf, __cov_v4sf);
__cov_m128 __builtin_ia32_comungeps (__cov_v4sf, __cov_v4sf);
__cov_m128 __builtin_ia32_comungtps (__cov_v4sf, __cov_v4sf);
__cov_m128 __builtin_ia32_comneqps (__cov_v4sf, __cov_v4sf);
__cov_m128d __builtin_ia32_comuneqpd (__cov_v2df, __cov_v2df);
__cov_m128d __builtin_ia32_comunltpd (__cov_v2df, __cov_v2df);
__cov_m128d __builtin_ia32_comunlepd (__cov_v2df, __cov_v2df);
__cov_m128d __builtin_ia32_comungepd (__cov_v2df, __cov_v2df);
__cov_m128d __builtin_ia32_comungtpd (__cov_v2df, __cov_v2df);
__cov_m128d __builtin_ia32_comneqpd (__cov_v2df, __cov_v2df);
__cov_m128 __builtin_ia32_comuneqss (__cov_v4sf, __cov_v4sf);
__cov_m128 __builtin_ia32_comunltss (__cov_v4sf, __cov_v4sf);
__cov_m128 __builtin_ia32_comunless (__cov_v4sf, __cov_v4sf);
__cov_m128 __builtin_ia32_comungess (__cov_v4sf, __cov_v4sf);
__cov_m128 __builtin_ia32_comungtss (__cov_v4sf, __cov_v4sf);
__cov_m128 __builtin_ia32_comneqss (__cov_v4sf, __cov_v4sf);
__cov_m128d __builtin_ia32_comuneqsd (__cov_v2df, __cov_v2df);
__cov_m128d __builtin_ia32_comunltsd (__cov_v2df, __cov_v2df);
__cov_m128d __builtin_ia32_comunlesd (__cov_v2df, __cov_v2df);
__cov_m128d __builtin_ia32_comungesd (__cov_v2df, __cov_v2df);
__cov_m128d __builtin_ia32_comungtsd (__cov_v2df, __cov_v2df);
__cov_m128d __builtin_ia32_comneqsd (__cov_v2df, __cov_v2df);
__cov_m128i __builtin_ia32_pcomnequb (__cov_v16qi, __cov_v16qi);
__cov_m128i __builtin_ia32_pcomnequw (__cov_v8hi, __cov_v8hi);
__cov_m128i __builtin_ia32_pcomnequd (__cov_v4si, __cov_v4si);
__cov_m128i __builtin_ia32_pcomnequq (__cov_v2di, __cov_v2di);
__cov_m128i __builtin_ia32_pcomneqb (__cov_v16qi, __cov_v16qi);
__cov_m128i __builtin_ia32_pcomneqw (__cov_v8hi, __cov_v8hi);
__cov_m128i __builtin_ia32_pcomneqd (__cov_v4si, __cov_v4si);
__cov_m128i __builtin_ia32_pcomneqq (__cov_v2di, __cov_v2di);


void __builtin_ia32_femms(void);
__cov_m64 __builtin_ia32_pavgusb (__cov_v8qi, __cov_v8qi);
__cov_m64 __builtin_ia32_pf2id (__cov_v2sf);
__cov_m64 __builtin_ia32_pfacc (__cov_v2sf, __cov_v2sf);
__cov_m64 __builtin_ia32_pfadd (__cov_v2sf, __cov_v2sf);
__cov_m64 __builtin_ia32_pfcmpeq (__cov_v2sf, __cov_v2sf);
__cov_m64 __builtin_ia32_pfcmpge (__cov_v2sf, __cov_v2sf);
__cov_m64 __builtin_ia32_pfcmpgt (__cov_v2sf, __cov_v2sf);
__cov_m64 __builtin_ia32_pfmax (__cov_v2sf, __cov_v2sf);
__cov_m64 __builtin_ia32_pfmin (__cov_v2sf, __cov_v2sf);
__cov_m64 __builtin_ia32_pfmul (__cov_v2sf, __cov_v2sf);
__cov_m64 __builtin_ia32_pfrcp (__cov_v2sf );
__cov_m64 __builtin_ia32_pfrcpit1 (__cov_v2sf, __cov_v2sf);
__cov_m64 __builtin_ia32_pfrcpit2 (__cov_v2sf, __cov_v2sf);
__cov_m64 __builtin_ia32_pfrsqrt (__cov_v2sf);
__cov_m64 __builtin_ia32_pfrsqit1 (__cov_v2sf, __cov_v2sf);
__cov_m64 __builtin_ia32_pfsub (__cov_v2sf, __cov_v2sf);
__cov_m64 __builtin_ia32_pfsubr (__cov_v2sf, __cov_v2sf);
__cov_m64 __builtin_ia32_pi2fd (__cov_v2si);
__cov_m64 __builtin_ia32_pmulhrw (__cov_v4hi, __cov_v4hi);

__cov_m64 __builtin_ia32_pf2iw (__cov_v2sf);
__cov_m64 __builtin_ia32_pfnacc (__cov_v2sf, __cov_v2sf);
__cov_m64 __builtin_ia32_pfpnacc (__cov_v2sf, __cov_v2sf);
__cov_m64 __builtin_ia32_pi2fw (__cov_v2si);
__cov_m64 __builtin_ia32_pswapdsf (__cov_v2sf);

#if defined(__AVX__) || __COVERITY_GCC49_INTRINSICS
/* These were interpretted from avxintrin.h */
__cov_m256d __builtin_ia32_addpd256(__cov_v4df, __cov_v4df);
__cov_m256  __builtin_ia32_addps256(__cov_v8sf, __cov_v8sf);
__cov_m256d __builtin_ia32_addsubpd256(__cov_v4df, __cov_v4df);
__cov_m256  __builtin_ia32_addsubps256(__cov_v8sf, __cov_v8sf);
__cov_m256d __builtin_ia32_andpd256(__cov_v4df, __cov_v4df);
__cov_m256  __builtin_ia32_andps256(__cov_v8sf, __cov_v8sf);
__cov_m256d __builtin_ia32_andnpd256(__cov_v4df, __cov_v4df);
__cov_m256  __builtin_ia32_andnps256(__cov_v8sf, __cov_v8sf);
__cov_m256d __builtin_ia32_blendpd256(__cov_v4df, __cov_v4df, int);
__cov_m256  __builtin_ia32_blendps256(__cov_v8sf, __cov_v8sf, int);
__cov_m256d __builtin_ia32_blendvpd256(__cov_v4df, __cov_v4df, __cov_v4df);
__cov_m256  __builtin_ia32_blendvps256(__cov_v8sf, __cov_v8sf, __cov_v8sf);
__cov_m256d __builtin_ia32_divpd256(__cov_v4df, __cov_v4df);
__cov_m256  __builtin_ia32_divps256(__cov_v8sf, __cov_v8sf);
__cov_m256  __builtin_ia32_dpps256(__cov_v8sf, __cov_v8sf, int);
__cov_m256d __builtin_ia32_haddpd256(__cov_v4df, __cov_v4df);
__cov_m256  __builtin_ia32_haddps256(__cov_v8sf, __cov_v8sf);
__cov_m256d __builtin_ia32_hsubpd256(__cov_v4df, __cov_v4df);
__cov_m256  __builtin_ia32_hsubps256(__cov_v8sf, __cov_v8sf);
__cov_m256d __builtin_ia32_maxpd256(__cov_v4df, __cov_v4df);
__cov_m256  __builtin_ia32_maxps256(__cov_v8sf, __cov_v8sf);
__cov_m256d __builtin_ia32_minpd256(__cov_v4df, __cov_v4df);
__cov_m256  __builtin_ia32_minps256(__cov_v8sf, __cov_v8sf);
__cov_m256d __builtin_ia32_mulpd256(__cov_v4df, __cov_v4df);
__cov_m256  __builtin_ia32_mulps256(__cov_v8sf, __cov_v8sf);
__cov_m256d __builtin_ia32_orpd256(__cov_v4df, __cov_v4df);
__cov_m256  __builtin_ia32_orps256(__cov_v8sf, __cov_v8sf);
__cov_m256d __builtin_ia32_shufpd256(__cov_v4df, __cov_v4df, int);
__cov_m256  __builtin_ia32_shufps256(__cov_v8sf, __cov_v8sf, int);
__cov_m256d __builtin_ia32_subpd256(__cov_v4df, __cov_v4df);
__cov_m256  __builtin_ia32_subps256(__cov_v8sf, __cov_v8sf);
__cov_m256d __builtin_ia32_xorpd256(__cov_v4df, __cov_v4df);
__cov_m256  __builtin_ia32_xorps256(__cov_v8sf, __cov_v8sf);
__cov_m128d __builtin_ia32_cmppd(__cov_v2df, __cov_v2df, int);
__cov_m128  __builtin_ia32_cmpps(__cov_v4sf, __cov_v4sf, int);
__cov_m256d __builtin_ia32_cmppd256(__cov_v4df, __cov_v4df, int);
__cov_m256  __builtin_ia32_cmpps256(__cov_v8sf, __cov_v8sf, int);
__cov_m128d __builtin_ia32_cmpsd(__cov_v2df, __cov_v2df, int);
__cov_m128  __builtin_ia32_cmpss(__cov_v4sf, __cov_v4sf, int);
__cov_m256d __builtin_ia32_cvtdq2pd256(__cov_v4si);
__cov_m256  __builtin_ia32_cvtdq2ps256(__cov_v8si);
__cov_m128  __builtin_ia32_cvtpd2ps256(__cov_v4df);
__cov_m256i __builtin_ia32_cvtps2dq256(__cov_v8sf);
__cov_m256d __builtin_ia32_cvtps2pd256(__cov_v4sf);
__cov_m128i __builtin_ia32_cvttpd2dq256(__cov_v4df);
__cov_m128i __builtin_ia32_cvtpd2dq256(__cov_v4df);
__cov_m256i __builtin_ia32_cvttps2dq256(__cov_v8sf);
__cov_m128d __builtin_ia32_vextractf128_pd256(__cov_v4df, int);
__cov_m128  __builtin_ia32_vextractf128_ps256(__cov_v8sf, int);
__cov_m128i __builtin_ia32_vextractf128_si256(__cov_v8si, int);
__cov_m128d __builtin_ia32_vpermilvarpd(__cov_v2df, __cov_v2di);
__cov_m256d __builtin_ia32_vpermilvarpd256(__cov_v4df, __cov_v4di);
__cov_m128  __builtin_ia32_vpermilvarps(__cov_v4sf, __cov_v4si);
__cov_m256  __builtin_ia32_vpermilvarps256(__cov_v8sf, __cov_v8si);
__cov_m128d __builtin_ia32_vpermilpd(__cov_v2df, int);
__cov_m256d __builtin_ia32_vpermilpd256(__cov_v4df, int);
__cov_m128  __builtin_ia32_vpermilps(__cov_v4sf, int);
__cov_m256  __builtin_ia32_vpermilps256(__cov_v8sf, int);
__cov_m256d __builtin_ia32_vperm2f128_pd256(__cov_v4df, __cov_v4df, int);
__cov_m256  __builtin_ia32_vperm2f128_ps256(__cov_v8sf, __cov_v8sf, int);
__cov_m256i __builtin_ia32_vperm2f128_si256(__cov_v8si, __cov_v8si, int);
__cov_m128  __builtin_ia32_vbroadcastss(const float *);
__cov_m256d __builtin_ia32_vbroadcastsd256(const double *);
__cov_m256  __builtin_ia32_vbroadcastss256(const float *);
__cov_m256d __builtin_ia32_vbroadcastf128_pd256(const __cov_v2df *);
__cov_m256  __builtin_ia32_vbroadcastf128_ps256(const __cov_m128 *);
__cov_m256d __builtin_ia32_vinsertf128_pd256(__cov_v4df, __cov_v2df, int);
__cov_m256  __builtin_ia32_vinsertf128_ps256(__cov_v8sf, __cov_v4sf, int);
__cov_m256i __builtin_ia32_vinsertf128_si256(__cov_v8si, __cov_v4si, int);
__cov_m256  __builtin_ia32_rcpps256(__cov_v8sf);
__cov_m256  __builtin_ia32_rsqrtps256(__cov_v8sf);
__cov_m256d __builtin_ia32_sqrtpd256(__cov_v4df);
__cov_m256  __builtin_ia32_sqrtps256(__cov_v8sf);
__cov_m256d __builtin_ia32_roundpd256(__cov_v4df, int);
__cov_m256  __builtin_ia32_roundps256(__cov_v8sf, int);
__cov_m256d __builtin_ia32_unpckhpd256(__cov_v4df, __cov_v4df);
__cov_m256d __builtin_ia32_unpcklpd256(__cov_v4df, __cov_v4df);
__cov_m256  __builtin_ia32_unpckhps256(__cov_v8sf, __cov_v8sf);
__cov_m256  __builtin_ia32_unpcklps256(__cov_v8sf, __cov_v8sf);
int __builtin_ia32_vtestzpd(__cov_v2df, __cov_v2df);
int __builtin_ia32_vtestcpd(__cov_v2df, __cov_v2df);
int __builtin_ia32_vtestnzcpd(__cov_v2df, __cov_v2df);
int __builtin_ia32_vtestzps(__cov_v4sf, __cov_v4sf);
int __builtin_ia32_vtestcps(__cov_v4sf, __cov_v4sf);
int __builtin_ia32_vtestnzcps(__cov_v4sf, __cov_v4sf);
int __builtin_ia32_vtestzpd256(__cov_v4df, __cov_v4df);
int __builtin_ia32_vtestcpd256(__cov_v4df, __cov_v4df);
int __builtin_ia32_vtestnzcpd256(__cov_v4df, __cov_v4df);
int __builtin_ia32_vtestzps256(__cov_v8sf, __cov_v8sf);
int __builtin_ia32_vtestcps256(__cov_v8sf, __cov_v8sf);
int __builtin_ia32_vtestnzcps256(__cov_v8sf, __cov_v8sf);
int __builtin_ia32_ptestz256(__cov_v4di, __cov_v4di);
int __builtin_ia32_ptestc256(__cov_v4di, __cov_v4di);
int __builtin_ia32_ptestnzc256(__cov_v4di, __cov_v4di);
int __builtin_ia32_movmskpd256(__cov_v4df);
int __builtin_ia32_movmskps256(__cov_v8sf);
__cov_m128d __builtin_ia32_pd_pd256(__cov_v4df);
__cov_m128  __builtin_ia32_ps_ps256(__cov_v8sf);
__cov_m128i __builtin_ia32_si_si256(__cov_v8si);
__cov_m256d __builtin_ia32_pd256_pd(__cov_v2df);
__cov_m256  __builtin_ia32_ps256_ps(__cov_v4sf);
__cov_m256i __builtin_ia32_si256_si(__cov_v4si);
__cov_m256d __builtin_ia32_loadupd256(const double *);
void __builtin_ia32_storeupd256(double *, __cov_v4df);
__cov_m256  __builtin_ia32_loadups256(const float *);
void __builtin_ia32_storeups256(float *, __cov_v8sf);
__cov_m256i __builtin_ia32_loaddqu256(const char *);
void __builtin_ia32_storedqu256(char *, __cov_v32qi);
__cov_m256  __builtin_ia32_movshdup256(__cov_v8sf);
__cov_m256  __builtin_ia32_movsldup256(__cov_v8sf);
__cov_m256d __builtin_ia32_movddup256(__cov_v4df);
__cov_m256i __builtin_ia32_lddqu256(const char *);
void __builtin_ia32_movntdq256(__cov_v4di *, __cov_v4di);
void __builtin_ia32_movntpd256(double *, __cov_v4df);
void __builtin_ia32_movntps256(float *, __cov_v8sf);
void __builtin_ia32_vzeroall(void);
void __builtin_ia32_vzeroupper(void);

// Handle incorrect signatures for some older gcc versions
// See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=47318
// Some older gcc versions were patched, so allow for GNU_GCC_47318_PATCHED
//   specified manually
#ifndef GNU_GCC_47318_PATCHED

/* Auto-detect GNU_GCC_47318_PATCHED for Red Hat gcc 4.4.5 */
#if (defined(__GNUC_RH_RELEASE__) && __GNUC__ == 4 && __GNUC_MINOR__ == 4 && \
     __GNUC_PATCHLEVEL__ == 5 && __GNUC_RH_RELEASE__ >= 6)
#define GNU_GCC_47318_PATCHED 1
#endif

/* Assume GNU_GCC_47318_PATCHED for gcc 4.5.2 since some popular distributions
   applied this patch but didn't provide a way to detect it */
#if (__GNUC__ == 4 && __GNUC_MINOR__ == 5 && __GNUC_PATCHLEVEL__ == 2)
#define GNU_GCC_47318_PATCHED 1
#endif

#endif

/* Respect GNU_GCC_47318_PATCHED if it's defined, otherwise use the gcc version
   to detect if this is fixed. The wrong signatures being detected will
   result in compiler errors in avxintrin.h. */
#if ((defined(GNU_GCC_47318_PATCHED) && GNU_GCC_47318_PATCHED && \
     !(__GNUC__ == 4 && __GNUC_MINOR__ == 5 & __GNUC_PATCHLEVEL__ == 2)) \
     || (!defined(GNU_GCC_47318_PATCHED) \
        && \
          (__GNUC__ > 4) \
       || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6) \
       || (__GNUC__ == 4 && __GNUC_MINOR__ == 5 && __GNUC_PATCHLEVEL__ >= 3) \
       || (__GNUC__ == 4 && __GNUC_MINOR__ == 4 && __GNUC_PATCHLEVEL__ >= 6) \
      ) \
    )
/* Signatures after this bug was fixed */
/* If the alternative signatures were incorrectly chosen (in the #else), use
   -DGNU_GCC_47318_PATCHED=1 to use these instead. This should only be
   necessary for some variants of gcc 4.4.5 and gcc 4.5.2. */
__cov_m128d __builtin_ia32_maskloadpd(const __cov_v2df *, __cov_v2di);
void __builtin_ia32_maskstorepd(__cov_v2df *, __cov_v2di, __cov_v2df);
__cov_m256d __builtin_ia32_maskloadpd256(const __cov_v4df *, __cov_v4di);
void __builtin_ia32_maskstorepd256(__cov_v4df *, __cov_v4di, __cov_v4df);
__cov_m128  __builtin_ia32_maskloadps(const __cov_v4sf *, __cov_v4si);
void __builtin_ia32_maskstoreps(__cov_v4sf *, __cov_v4si, __cov_v4sf);
__cov_m256  __builtin_ia32_maskloadps256(const __cov_v8sf *, __cov_v8si);
void __builtin_ia32_maskstoreps256(__cov_v8sf *, __cov_v8si, __cov_v8sf);

#else

/* Signatures before this gcc bug was fixed */
/* If the alternative signatures were incorrectly chosen (above), use
   -DGNU_GCC_47318_PATCHED=0 to use these instead. */
__cov_m128d __builtin_ia32_maskloadpd(const __cov_v2df *, __cov_v2df);
void __builtin_ia32_maskstorepd(__cov_v2df *, __cov_v2df, __cov_v2df);
__cov_m256d __builtin_ia32_maskloadpd256(const __cov_v4df *, __cov_v4df);
void __builtin_ia32_maskstorepd256(__cov_v4df *, __cov_v4df, __cov_v4df);
__cov_m128  __builtin_ia32_maskloadps(const __cov_v4sf *, __cov_v4sf);
void __builtin_ia32_maskstoreps(__cov_v4sf *, __cov_v4sf, __cov_v4sf);
__cov_m256  __builtin_ia32_maskloadps256(const __cov_v8sf *, __cov_v8sf);
void __builtin_ia32_maskstoreps256(__cov_v8sf *, __cov_v8sf, __cov_v8sf);

#endif

#if defined(__FMA__) || defined(__FMA4__) || __COVERITY_GCC49_INTRINSICS
/* These were interpretted from fmaintrin.h, fma4intrin.h */
__cov_m128d __builtin_ia32_vfmaddpd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_m256d __builtin_ia32_vfmaddpd256 (__cov_v4df, __cov_v4df, __cov_v4df);
__cov_m128  __builtin_ia32_vfmaddps (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_m256  __builtin_ia32_vfmaddps256 (__cov_v8sf, __cov_v8sf, __cov_v8sf);
__cov_m128d __builtin_ia32_vfmaddsd3 (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_m128  __builtin_ia32_vfmaddss3 (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_m128d __builtin_ia32_vfmaddsubpd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_m256d __builtin_ia32_vfmaddsubpd256 (__cov_v4df, __cov_v4df, __cov_v4df);
__cov_m128  __builtin_ia32_vfmaddsubps (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_m256  __builtin_ia32_vfmaddsubps256 (__cov_v8sf, __cov_v8sf, __cov_v8sf);

void __builtin_ia32_vzeroall(void);
void __builtin_ia32_vzeroupper(void);
#endif /* __FMA__ || __FMA4__ || __COVERITY_GCC49_INTRINSICS */

#if defined(__FMA4__) || __COVERITY_GCC49_INTRINSICS
/* These were interpretted from fma4intrin.h */
__cov_m128 __builtin_ia32_vfmaddss (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_m128d __builtin_ia32_vfmaddsd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_m128 __builtin_ia32_vfmsubps (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_m128d __builtin_ia32_vfmsubpd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_m128 __builtin_ia32_vfmsubss (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_m128d __builtin_ia32_vfmsubsd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_m128 __builtin_ia32_vfnmaddps (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_m128d __builtin_ia32_vfnmaddpd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_m128 __builtin_ia32_vfnmaddss (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_m128d __builtin_ia32_vfnmaddsd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_m128 __builtin_ia32_vfnmsubps (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_m128d __builtin_ia32_vfnmsubpd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_m128 __builtin_ia32_vfnmsubss (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_m128d __builtin_ia32_vfnmsubsd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_m128 __builtin_ia32_vfmsubaddps (__cov_v4sf, __cov_v4sf, __cov_v4sf);
__cov_m128d __builtin_ia32_vfmsubaddpd (__cov_v2df, __cov_v2df, __cov_v2df);
__cov_m256 __builtin_ia32_vfmsubps256 (__cov_v8sf, __cov_v8sf, __cov_v8sf);
__cov_m256d __builtin_ia32_vfmsubpd256 (__cov_v4df, __cov_v4df, __cov_v4df);
__cov_m256 __builtin_ia32_vfnmaddps256 (__cov_v8sf, __cov_v8sf, __cov_v8sf);
__cov_m256d __builtin_ia32_vfnmaddpd256 (__cov_v4df, __cov_v4df, __cov_v4df);
__cov_m256 __builtin_ia32_vfnmsubps256 (__cov_v8sf, __cov_v8sf, __cov_v8sf);
__cov_m256d __builtin_ia32_vfnmsubpd256 (__cov_v4df, __cov_v4df, __cov_v4df);
__cov_m256 __builtin_ia32_vfmsubaddps256 (__cov_v8sf, __cov_v8sf, __cov_v8sf);
__cov_m256d __builtin_ia32_vfmsubaddpd256 (__cov_v4df, __cov_v4df, __cov_v4df);
#endif /* __FMA4__ || __COVERITY_GCC49_INTRINSICS */

#endif /* __AVX__ || __COVERITY_GCC49_INTRINSICS */

#if defined(__XSAVE__) || __COVERITY_GCC49_INTRINSICS
void __builtin_ia32_xsave(void *, long long);
void __builtin_ia32_xrstor(void *, long long);

#if defined(__x86_64__) || __COVERITY_GCC49_INTRINSICS
void __builtin_ia32_xsave64 (void *, long long);
void __builtin_ia32_xrstor64(void *, long long);
#endif /* __x86_64__ || __COVERITY_GCC49_INTRINSICS */

#endif /* __XSAVE__ || __COVERITY_GCC49_INTRINSICS */

#if defined(__XSAVEOPT__) || __COVERITY_GCC49_INTRINSICS
void __builtin_ia32_xsaveopt(void *, long long);
void __builtin_ia32_xsaveopt64(void *, long long);
#endif /* __XSAVEOPT__ || __COVERITY_GCC49_INTRINSICS */

typedef char __cov_v64qi __attribute__((__vector_size__(64)));
typedef double __cov_m512d __attribute__((__vector_size__(64), __may_alias__));
typedef double __cov_v8df __attribute__((__vector_size__(64)));
typedef float __cov_m512 __attribute__((__vector_size__(64), __may_alias__));
typedef float __cov_v16sf __attribute__((__vector_size__(64)));
typedef int __cov_v16si __attribute__((__vector_size__(64)));
typedef long long __cov_m512i __attribute__((__vector_size__(64), __may_alias__));
typedef long long __cov_v8di __attribute__((__vector_size__(64)));
typedef short __cov_v32hi __attribute__((__vector_size__(64)));
typedef unsigned char __cov_mmask8;
typedef unsigned short __cov_mmask16;
typedef unsigned long __cov_mmask32;
typedef unsigned long long __cov_mmask64;

/*
 * AES (Advanced Encryption Standard) Intrinsics
 * Introduced in GCC 4.4.
 */
#if defined(__AES__) || __COVERITY_GCC49_INTRINSICS
__cov_v2di __builtin_ia32_aesenc128(__cov_v2di,__cov_v2di);
__cov_v2di __builtin_ia32_aesenclast128(__cov_v2di,__cov_v2di);
__cov_v2di __builtin_ia32_aesdec128(__cov_v2di,__cov_v2di);
__cov_v2di __builtin_ia32_aesdeclast128(__cov_v2di,__cov_v2di);
__cov_v2di __builtin_ia32_aeskeygenassist128(__cov_v2di,const int);
__cov_v2di __builtin_ia32_aesimc128(__cov_v2di);
#endif /* __AES__ || __COVERITY_GCC49_INTRINSICS */

/*
 * PCLMUL (Perform carry-less multiplication) Intrinsics
 */
#if defined(__PCLMUL__) || __COVERITY_GCC49_INTRINSICS
__cov_m128i __builtin_ia32_pclmulqdq128(__cov_v2di,__cov_v2di,int);
#endif /* __PCLMUL__ || __COVERITY_GCC49_INTRINSICS */

/*
 * Intrinsics for accessing the eflags register.
 * Introduced in GCC 4.9.
 */
#if __COVERITY_GCC49_INTRINSICS
unsigned int __builtin_ia32_readeflags_u32(void);
unsigned long long __builtin_ia32_readeflags_u64(void);
void __builtin_ia32_writeeflags_u32(unsigned int);

unsigned long long __builtin_ia32_readeflags_u64(void);
void __builtin_ia32_writeeflags_u64(unsigned long long);
#endif /* __COVERITY_GCC49_INTRINSICS */

/*
 * AVX-2 (Advanced Vector eXtensions) intrinsics.
 * Introduced in GCC 4.7.
 */
#if defined(__AVX2__) || __COVERITY_GCC49_INTRINSICS
__cov_v32qi __builtin_ia32_mpsadbw256(__cov_v32qi,__cov_v32qi,int);
__cov_v32qi __builtin_ia32_pabsb256(__cov_v32qi);
__cov_v16hi __builtin_ia32_pabsw256(__cov_v16hi);
__cov_v8si __builtin_ia32_pabsd256(__cov_v8si);
__cov_v16hi __builtin_ia32_packssdw256(__cov_v8si,__cov_v8si);
__cov_v32qi __builtin_ia32_packsswb256(__cov_v16hi,__cov_v16hi);
__cov_v16hi __builtin_ia32_packusdw256(__cov_v8si,__cov_v8si);
__cov_v32qi __builtin_ia32_packuswb256(__cov_v16hi,__cov_v16hi);
__cov_v32qi __builtin_ia32_paddb256(__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_paddw256(__cov_v16hi,__cov_v16hi);
__cov_v8si __builtin_ia32_paddd256(__cov_v8si,__cov_v8si);
__cov_v4di __builtin_ia32_paddq256(__cov_v4di,__cov_v4di);
__cov_v32qi __builtin_ia32_paddsb256(__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_paddsw256(__cov_v16hi,__cov_v16hi);
__cov_v32qi __builtin_ia32_paddusb256(__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_paddusw256(__cov_v16hi,__cov_v16hi);
__cov_v4di __builtin_ia32_palignr256(__cov_v4di,__cov_v4di,int);
__cov_v4di __builtin_ia32_andsi256(__cov_v4di,__cov_v4di);
__cov_v4di __builtin_ia32_andnotsi256(__cov_v4di,__cov_v4di);
__cov_v32qi __builtin_ia32_pavgb256(__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_pavgw256(__cov_v16hi,__cov_v16hi);
__cov_v32qi __builtin_ia32_pblendvb256(__cov_v32qi,__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_pblendw256(__cov_v16hi,__cov_v16hi,int);
__cov_v32qi __builtin_ia32_pcmpeqb256(__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_pcmpeqw256(__cov_v16hi,__cov_v16hi);
__cov_v8si __builtin_ia32_pcmpeqd256(__cov_v8si,__cov_v8si);
__cov_v4di __builtin_ia32_pcmpeqq256(__cov_v4di,__cov_v4di);
__cov_v32qi __builtin_ia32_pcmpgtb256(__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_pcmpgtw256(__cov_v16hi,__cov_v16hi);
__cov_v8si __builtin_ia32_pcmpgtd256(__cov_v8si,__cov_v8si);
__cov_v4di __builtin_ia32_pcmpgtq256(__cov_v4di,__cov_v4di);
__cov_v16hi __builtin_ia32_phaddw256(__cov_v16hi,__cov_v16hi);
__cov_v8si __builtin_ia32_phaddd256(__cov_v8si,__cov_v8si);
__cov_v16hi __builtin_ia32_phaddsw256(__cov_v16hi,__cov_v16hi);
__cov_v16hi __builtin_ia32_phsubw256(__cov_v16hi,__cov_v16hi);
__cov_v8si __builtin_ia32_phsubd256(__cov_v8si,__cov_v8si);
__cov_v16hi __builtin_ia32_phsubsw256(__cov_v16hi,__cov_v16hi);
__cov_v32qi __builtin_ia32_pmaddubsw256(__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_pmaddwd256(__cov_v16hi,__cov_v16hi);
__cov_v32qi __builtin_ia32_pmaxsb256(__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_pmaxsw256(__cov_v16hi,__cov_v16hi);
__cov_v8si __builtin_ia32_pmaxsd256(__cov_v8si,__cov_v8si);
__cov_v32qi __builtin_ia32_pmaxub256(__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_pmaxuw256(__cov_v16hi,__cov_v16hi);
__cov_v8si __builtin_ia32_pmaxud256(__cov_v8si,__cov_v8si);
__cov_v32qi __builtin_ia32_pminsb256(__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_pminsw256(__cov_v16hi,__cov_v16hi);
__cov_v8si __builtin_ia32_pminsd256(__cov_v8si,__cov_v8si);
__cov_v32qi __builtin_ia32_pminub256(__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_pminuw256(__cov_v16hi,__cov_v16hi);
__cov_v8si __builtin_ia32_pminud256(__cov_v8si,__cov_v8si);
int __builtin_ia32_pmovmskb256(__cov_v32qi);
__cov_v16hi __builtin_ia32_pmovsxbw256(__cov_v16qi);
__cov_v8si __builtin_ia32_pmovsxbd256(__cov_v16qi);
__cov_v4di __builtin_ia32_pmovsxbq256(__cov_v16qi);
__cov_v8si __builtin_ia32_pmovsxwd256(__cov_v8hi);
__cov_v4di __builtin_ia32_pmovsxwq256(__cov_v8hi);
__cov_v4di __builtin_ia32_pmovsxdq256(__cov_v4si);
__cov_v16hi __builtin_ia32_pmovzxbw256(__cov_v16qi);
__cov_v8si __builtin_ia32_pmovzxbd256(__cov_v16qi);
__cov_v4di __builtin_ia32_pmovzxbq256(__cov_v16qi);
__cov_v8si __builtin_ia32_pmovzxwd256(__cov_v8hi);
__cov_v4di __builtin_ia32_pmovzxwq256(__cov_v8hi);
__cov_v4di __builtin_ia32_pmovzxdq256(__cov_v4si);
__cov_v4di __builtin_ia32_pmuldq256(__cov_v8si,__cov_v8si);
__cov_v16hi __builtin_ia32_pmulhrsw256(__cov_v16hi, __cov_v16hi);
__cov_v16hi __builtin_ia32_pmulhuw256(__cov_v16hi,__cov_v16hi);
__cov_v16hi __builtin_ia32_pmulhw256(__cov_v16hi,__cov_v16hi);
__cov_v16hi __builtin_ia32_pmullw256(__cov_v16hi,__cov_v16hi);
__cov_v8si __builtin_ia32_pmulld256(__cov_v8si,__cov_v8si);
__cov_v4di __builtin_ia32_pmuludq256(__cov_v8si,__cov_v8si);
__cov_v4di __builtin_ia32_por256(__cov_v4di,__cov_v4di);
__cov_v16hi __builtin_ia32_psadbw256(__cov_v32qi,__cov_v32qi);
__cov_v32qi __builtin_ia32_pshufb256(__cov_v32qi,__cov_v32qi);
__cov_v8si __builtin_ia32_pshufd256(__cov_v8si,int);
__cov_v16hi __builtin_ia32_pshufhw256(__cov_v16hi,int);
__cov_v16hi __builtin_ia32_pshuflw256(__cov_v16hi,int);
__cov_v32qi __builtin_ia32_psignb256(__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_psignw256(__cov_v16hi,__cov_v16hi);
__cov_v8si __builtin_ia32_psignd256(__cov_v8si,__cov_v8si);
__cov_v4di __builtin_ia32_pslldqi256(__cov_v4di,int);
__cov_v16hi __builtin_ia32_psllwi256(__cov_v16hi,int);
__cov_v16hi __builtin_ia32_psllw256(__cov_v16hi,__cov_v8hi);
__cov_v8si __builtin_ia32_pslldi256(__cov_v8si,int);
__cov_v8si __builtin_ia32_pslld256(__cov_v8si,__cov_v4si);
__cov_v4di __builtin_ia32_psllqi256(__cov_v4di,int);
__cov_v4di __builtin_ia32_psllq256(__cov_v4di,__cov_v2di);
__cov_v16hi __builtin_ia32_psrawi256(__cov_v16hi,int);
__cov_v16hi __builtin_ia32_psraw256(__cov_v16hi,__cov_v8hi);
__cov_v8si __builtin_ia32_psradi256(__cov_v8si,int);
__cov_v8si __builtin_ia32_psrad256(__cov_v8si,__cov_v4si);
__cov_v4di __builtin_ia32_psrldqi256(__cov_v4di, int);
__cov_v16hi __builtin_ia32_psrlwi256(__cov_v16hi,int);
__cov_v16hi __builtin_ia32_psrlw256(__cov_v16hi,__cov_v8hi);
__cov_v8si __builtin_ia32_psrldi256(__cov_v8si,int);
__cov_v8si __builtin_ia32_psrld256(__cov_v8si,__cov_v4si);
__cov_v4di __builtin_ia32_psrlqi256(__cov_v4di,int);
__cov_v4di __builtin_ia32_psrlq256(__cov_v4di,__cov_v2di);
__cov_v32qi __builtin_ia32_psubb256(__cov_v32qi,__cov_v32qi);
__cov_m256i __builtin_ia32_psubw256(__cov_v16hi,__cov_v16hi);
__cov_v8si __builtin_ia32_psubd256(__cov_v8si,__cov_v8si);
__cov_v4di __builtin_ia32_psubq256(__cov_v4di,__cov_v4di);
__cov_v32qi __builtin_ia32_psubsb256(__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_psubsw256(__cov_v16hi,__cov_v16hi);
__cov_v32qi __builtin_ia32_psubusb256(__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_psubusw256(__cov_v16hi,__cov_v16hi);
__cov_v32qi __builtin_ia32_punpckhbw256(__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_punpckhwd256(__cov_v16hi,__cov_v16hi);
__cov_v8si __builtin_ia32_punpckhdq256(__cov_v8si,__cov_v8si);
__cov_v4di __builtin_ia32_punpckhqdq256(__cov_v4di,__cov_v4di);
__cov_v32qi __builtin_ia32_punpcklbw256(__cov_v32qi,__cov_v32qi);
__cov_v16hi __builtin_ia32_punpcklwd256(__cov_v16hi,__cov_v16hi);
__cov_v8si __builtin_ia32_punpckldq256(__cov_v8si,__cov_v8si);
__cov_v4di __builtin_ia32_punpcklqdq256(__cov_v4di,__cov_v4di);
__cov_v4di __builtin_ia32_pxor256(__cov_v4di,__cov_v4di);
__cov_v4di __builtin_ia32_movntdqa256(__cov_v4di *);
__cov_v4sf __builtin_ia32_vbroadcastss_ps(__cov_v4sf);
__cov_v8sf __builtin_ia32_vbroadcastss_ps256(__cov_v4sf);
__cov_v4df __builtin_ia32_vbroadcastsd_pd256(__cov_v2df);
__cov_v4di __builtin_ia32_vbroadcastsi256(__cov_v2di);
__cov_v4si __builtin_ia32_pblendd128(__cov_v4si,__cov_v4si, int);
__cov_v8si __builtin_ia32_pblendd256(__cov_v8si,__cov_v8si, int);
__cov_v32qi __builtin_ia32_pbroadcastb256(__cov_v16qi);
__cov_v16hi __builtin_ia32_pbroadcastw256(__cov_v8hi);
__cov_v8si __builtin_ia32_pbroadcastd256(__cov_v4si);
__cov_v4di __builtin_ia32_pbroadcastq256(__cov_v2di);
__cov_v16qi __builtin_ia32_pbroadcastb128(__cov_v16qi);
__cov_v8hi __builtin_ia32_pbroadcastw128(__cov_v8hi);
__cov_v4si __builtin_ia32_pbroadcastd128(__cov_v4si);
__cov_v2di __builtin_ia32_pbroadcastq128(__cov_v2di);
__cov_v8si __builtin_ia32_permvarsi256(__cov_v8si,__cov_v8si);
__cov_v4df __builtin_ia32_permdf256(__cov_v4df,int);
__cov_v8sf __builtin_ia32_permvarsf256(__cov_v8sf,__cov_v8si);
__cov_v4di __builtin_ia32_permdi256(__cov_v4di,int);
__cov_v4di __builtin_ia32_permti256(__cov_v4di,__cov_v4di,int);
__cov_m128i __builtin_ia32_extract128i256(__cov_v4di,int);
__cov_v4di __builtin_ia32_insert128i256(__cov_v4di,__cov_v2di,int);
__cov_v8si __builtin_ia32_maskloadd256(const __cov_v8si *,__cov_v8si);
__cov_v4di __builtin_ia32_maskloadq256(const __cov_v4di *,__cov_v4di);
__cov_v4si __builtin_ia32_maskloadd(const __cov_v4si *,__cov_v4si);
__cov_v2di __builtin_ia32_maskloadq(const __cov_v2di *,__cov_v2di);
void __builtin_ia32_maskstored256(__cov_v8si *,__cov_v8si,__cov_v8si);
void __builtin_ia32_maskstoreq256(__cov_v4di *,__cov_v4di,__cov_v4di);
void __builtin_ia32_maskstored(__cov_v4si *,__cov_v4si,__cov_v4si);
void __builtin_ia32_maskstoreq(__cov_v2di *,__cov_v2di,__cov_v2di);
__cov_v8si __builtin_ia32_psllv8si(__cov_v8si,__cov_v8si);
__cov_v4si __builtin_ia32_psllv4si(__cov_v4si,__cov_v4si);
__cov_v4di __builtin_ia32_psllv4di(__cov_v4di,__cov_v4di);
__cov_v2di __builtin_ia32_psllv2di(__cov_v2di,__cov_v2di);
__cov_v8si __builtin_ia32_psrav8si(__cov_v8si,__cov_v8si);
__cov_v4si __builtin_ia32_psrav4si(__cov_v4si,__cov_v4si);
__cov_v8si __builtin_ia32_psrlv8si(__cov_v8si,__cov_v8si);
__cov_v4si __builtin_ia32_psrlv4si(__cov_v4si,__cov_v4si);
__cov_v4di __builtin_ia32_psrlv4di(__cov_v4di,__cov_v4di);
__cov_v2di __builtin_ia32_psrlv2di(__cov_v2di,__cov_v2di);
__cov_v2df __builtin_ia32_gathersiv2df(__cov_v2df, const double *,__cov_v4si,__cov_v2df,int);
__cov_v4df __builtin_ia32_gathersiv4df(__cov_v4df, const double *,__cov_v4si,__cov_v4df,int);
__cov_v2df __builtin_ia32_gatherdiv2df(__cov_v2df, const double *,__cov_v2di,__cov_v2df,int);
__cov_v4df __builtin_ia32_gatherdiv4df(__cov_v4df, const double *,__cov_v4di,__cov_v4df,int);
__cov_v4sf __builtin_ia32_gathersiv4sf(__cov_v4sf, const float *,__cov_v4si,__cov_v4sf,int);
__cov_v8sf __builtin_ia32_gathersiv8sf(__cov_v8sf, const float *,__cov_v8si,__cov_v8sf,int);
__cov_v4sf __builtin_ia32_gatherdiv4sf(__cov_v4sf, const float *,__cov_v2di,__cov_v4sf,int);
__cov_v4sf __builtin_ia32_gatherdiv4sf256(__cov_v4sf, const float *,__cov_v4di,__cov_v4sf,int);
__cov_v2di __builtin_ia32_gathersiv2di(__cov_v2di, const long long *,__cov_v4si,__cov_v2di,int);
__cov_v4di __builtin_ia32_gathersiv4di(__cov_v4di, const long long *,__cov_v4si,__cov_v4di,int);
__cov_v2di __builtin_ia32_gatherdiv2di(__cov_v2di, const long long *,__cov_v2di,__cov_v2di,int);
__cov_v4di __builtin_ia32_gatherdiv4di(__cov_v4di, const long long *,__cov_v4di,__cov_v4di,int);
__cov_v4si __builtin_ia32_gathersiv4si(__cov_v4si, const int *,__cov_v4si,__cov_v4si,int);
__cov_v8si __builtin_ia32_gathersiv8si(__cov_v8si, const int *,__cov_v8si,__cov_v8si,int);
__cov_v4si __builtin_ia32_gatherdiv4si(__cov_v4si, const int *,__cov_v2di,__cov_v4si,int);
__cov_v4si __builtin_ia32_gatherdiv4si256(__cov_v4si, const int *,__cov_v4di,__cov_v4si,int);
#endif /* __AVX2__ || __COVERITY_GCC49_INTRINSICS */

/*
 * AVX-512 (Advanced Vector eXtensions) foundation intrinsics.
 * Introduced in GCC 4.9.
 */
#if defined(__AVX512F__) || __COVERITY_GCC49_INTRINSICS
__cov_m512d __builtin_ia32_addpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_addps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512i __builtin_ia32_blendmd_512_mask(__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512d __builtin_ia32_blendmpd_512_mask(__cov_v8df,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_blendmps_512_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_m512i __builtin_ia32_blendmq_512_mask(__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512 __builtin_ia32_broadcastf32x4_512(__cov_v4sf,__cov_v16sf,__cov_mmask16);
__cov_m512d __builtin_ia32_broadcastf64x4_512(__cov_v4df,__cov_v8df,__cov_mmask8);
__cov_m512i __builtin_ia32_broadcasti32x4_512(__cov_v4si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_broadcasti64x4_512(__cov_v4di,__cov_v8di,__cov_mmask8);
__cov_m512d __builtin_ia32_broadcastsd512(__cov_v2df,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_broadcastss512(__cov_v4sf,__cov_v16sf,__cov_mmask16);
__cov_mmask16 __builtin_ia32_cmpd512_mask(__cov_v16si,__cov_v16si,int,__cov_mmask16);
__cov_mmask8 __builtin_ia32_cmpq512_mask(__cov_v8di,__cov_v8di,int,__cov_mmask8);
__cov_m512d __builtin_ia32_compressdf512_mask(__cov_v8df,__cov_v8df,__cov_mmask8);
__cov_m512i __builtin_ia32_compressdi512_mask(__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512 __builtin_ia32_compresssf512_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_m512i __builtin_ia32_compresssi512_mask(__cov_v16si,__cov_v16si,__cov_mmask16);
void __builtin_ia32_compressstoredf512_mask(__cov_v8df *,__cov_v8df,__cov_mmask8);
void __builtin_ia32_compressstoredi512_mask(__cov_v8di *,__cov_v8di,__cov_mmask8);
void __builtin_ia32_compressstoresf512_mask(__cov_v16sf *,__cov_v16sf,__cov_mmask16);
void __builtin_ia32_compressstoresi512_mask(__cov_v16si *,__cov_v16si,__cov_mmask16);
__cov_m512d __builtin_ia32_cvtdq2pd512_mask(__cov_v8si,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_cvtdq2ps512_mask(__cov_v16si,__cov_v16sf,__cov_mmask16,int);
__cov_m256i __builtin_ia32_cvtpd2dq512_mask(__cov_v8df,__cov_v8si,__cov_mmask8,int);
__cov_m256 __builtin_ia32_cvtpd2ps512_mask(__cov_v8df,__cov_v8sf,__cov_mmask8,int);
__cov_m256i __builtin_ia32_cvtpd2udq512_mask(__cov_v8df,__cov_v8si,__cov_mmask8,int);
__cov_m512i __builtin_ia32_cvtps2dq512_mask(__cov_v16sf,__cov_v16si,__cov_mmask16,int);
__cov_m512d __builtin_ia32_cvtps2pd512_mask(__cov_v8sf,__cov_v8df,__cov_mmask8,int);
__cov_m512i __builtin_ia32_cvtps2udq512_mask(__cov_v16sf,__cov_v16si,__cov_mmask16,int);
__cov_m256i __builtin_ia32_cvttpd2dq512_mask(__cov_v8df,__cov_v8si,__cov_mmask8,int);
__cov_m256i __builtin_ia32_cvttpd2udq512_mask(__cov_v8df,__cov_v8si,__cov_mmask8,int);
__cov_m512i __builtin_ia32_cvttps2dq512_mask(__cov_v16sf,__cov_v16si,__cov_mmask16,int);
__cov_m512i __builtin_ia32_cvttps2udq512_mask(__cov_v16sf,__cov_v16si,__cov_mmask16,int);
__cov_m512d __builtin_ia32_cvtudq2pd512_mask(__cov_v8si,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_cvtudq2ps512_mask(__cov_v16si,__cov_v16sf,__cov_mmask16,int);
__cov_v4df __builtin_ia32_extractf64x4_mask(__cov_v8df,int,__cov_v4df,__cov_mmask8);
__cov_v4sf __builtin_ia32_extractf32x4_mask(__cov_v16sf,int,__cov_v4sf,__cov_mmask8);
__cov_v4di __builtin_ia32_extracti64x4_mask(__cov_v8di,int,__cov_v4di,__cov_mmask8);
__cov_v4si __builtin_ia32_extracti32x4_mask(__cov_v16si,int,__cov_v4si,__cov_mmask8);
__cov_v16si __builtin_ia32_inserti32x4_mask(__cov_v16si,__cov_v4si,int,__cov_v16si,__cov_mmask16);
__cov_v16sf __builtin_ia32_insertf32x4_mask(__cov_v16sf,__cov_v4sf,int,__cov_v16sf,__cov_mmask16);
__cov_v8di __builtin_ia32_inserti64x4_mask(__cov_v8di,__cov_v4di,int,__cov_v8di,__cov_mmask8);
__cov_v8df __builtin_ia32_insertf64x4_mask(__cov_v8df,__cov_v4df,int,__cov_v8df,__cov_mmask8);
__cov_v16si __builtin_ia32_inserti32x8_mask(__cov_v16si,__cov_v8si,int,__cov_v16si,__cov_mmask16);
__cov_m128d __builtin_ia32_cvtusi2sd32(__cov_v2df,unsigned);
__cov_m128d __builtin_ia32_cvtusi2sd64(__cov_v2df,unsigned long long,int);
__cov_v2df __builtin_ia32_cvtsi2sd64(__cov_v2df,long long,int);
__cov_m128 __builtin_ia32_cvtusi2ss32(__cov_v4sf,unsigned,int);
__cov_v4sf __builtin_ia32_cvtsi2ss32(__cov_v4sf,int,int);
__cov_m128 __builtin_ia32_cvtusi2ss64(__cov_v4sf,unsigned long long,int);
__cov_v4sf __builtin_ia32_cvtsi2ss64(__cov_v4sf,long long,int);
__cov_m512d __builtin_ia32_divpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_divps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m128d __builtin_ia32_mulsd_round(__cov_v2df,__cov_v2df,int);
__cov_m128 __builtin_ia32_mulss_round(__cov_v4sf,__cov_v4sf,int);
__cov_m128d __builtin_ia32_divsd_round(__cov_v2df,__cov_v2df,int);
__cov_m128 __builtin_ia32_divss_round(__cov_v4sf,__cov_v4sf,int);
__cov_m512d __builtin_ia32_expanddf512_mask(__cov_v8df,__cov_v8df,__cov_mmask8);
__cov_m512d __builtin_ia32_expanddf512_maskz(__cov_v8df,__cov_v8df,__cov_mmask8);
__cov_m512i __builtin_ia32_expanddi512_mask(__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_expanddi512_maskz(__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512d __builtin_ia32_expandloaddf512_mask(const __cov_v8df *,__cov_v8df,__cov_mmask8);
__cov_m512d __builtin_ia32_expandloaddf512_maskz(const __cov_v8df *,__cov_v8df,__cov_mmask8);
__cov_m512i __builtin_ia32_expandloaddi512_mask(const __cov_v8di *,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_expandloaddi512_maskz(const __cov_v8di *,__cov_v8di,__cov_mmask8);
__cov_m512 __builtin_ia32_expandloadsf512_mask(const __cov_v16sf *,__cov_v16sf,__cov_mmask16);
__cov_m512 __builtin_ia32_expandloadsf512_maskz(const __cov_v16sf *,__cov_v16sf,__cov_mmask16);
__cov_m512i __builtin_ia32_expandloadsi512_mask(const __cov_v16si *,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_expandloadsi512_maskz(const __cov_v16si *,__cov_v16si,__cov_mmask16);
__cov_m512 __builtin_ia32_expandsf512_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_m512 __builtin_ia32_expandsf512_maskz(__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_m512i __builtin_ia32_expandsi512_mask(__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_expandsi512_maskz(__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_mmask16 __builtin_ia32_kandhi(__cov_mmask16,__cov_mmask16);
__cov_mmask16 __builtin_ia32_kandnhi(__cov_mmask16,__cov_mmask16);
__cov_mmask16 __builtin_ia32_kmov16(__cov_mmask16);
__cov_mmask16 __builtin_ia32_knothi(__cov_mmask16);
__cov_mmask16 __builtin_ia32_korhi(__cov_mmask16,__cov_mmask16);
__cov_mmask16 __builtin_ia32_kortestchi(__cov_mmask16,__cov_mmask16);
__cov_mmask16 __builtin_ia32_kortestzhi(__cov_mmask16,__cov_mmask16);
__cov_mmask16 __builtin_ia32_kunpckhi(__cov_mmask16,__cov_mmask16);
__cov_mmask16 __builtin_ia32_kxnorhi(__cov_mmask16,__cov_mmask16);
__cov_mmask16 __builtin_ia32_kxorhi(__cov_mmask16,__cov_mmask16);
__cov_m512d __builtin_ia32_loadapd512_mask(const __cov_v8df *,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_loadaps512_mask(const __cov_v16sf *,__cov_v16sf,__cov_mmask16);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_m512i __builtin_ia32_loaddqudi512_mask(const long long *,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_loaddqusi512_mask(const int *,__cov_v16si,__cov_mmask16);
__cov_m512d __builtin_ia32_loadupd512_mask(const double *,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_loadups512_mask(const float *,__cov_v16sf,__cov_mmask16);
#else
__cov_m512i __builtin_ia32_loaddqudi512_mask(const __cov_v8di *,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_loaddqusi512_mask(const __cov_v16si *,__cov_v16si,__cov_mmask16);
__cov_m512d __builtin_ia32_loadupd512_mask(const __cov_v8df *,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_loadups512_mask(const __cov_v16sf *,__cov_v16sf,__cov_mmask16);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_m512d __builtin_ia32_maxpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_maxps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512d __builtin_ia32_minpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_minps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512d __builtin_ia32_movapd512_mask(__cov_v8df,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_movaps512_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_m512d __builtin_ia32_movddup512_mask(__cov_v8df,__cov_v8df,__cov_mmask8);
__cov_m512i __builtin_ia32_movdqa32_512_mask(__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_movdqa32load512_mask(const __cov_v16si *,__cov_v16si,__cov_mmask16);
void __builtin_ia32_movdqa32store512_mask(__cov_v16si *,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_movdqa64_512_mask(__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_movdqa64load512_mask(const __cov_v8di *,__cov_v8di,__cov_mmask8);
void __builtin_ia32_movdqa64store512_mask(__cov_v8di *,__cov_v8di,__cov_mmask8);
void __builtin_ia32_movntdq512(__cov_v8di *,__cov_v8di);
__cov_m512i __builtin_ia32_movntdqa512(__cov_v8di *);
void __builtin_ia32_movntpd512(double *,__cov_v8df);
void __builtin_ia32_movntps512(float *,__cov_v16sf);
__cov_m512 __builtin_ia32_movshdup512_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_m512 __builtin_ia32_movsldup512_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_m512d __builtin_ia32_mulpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_mulps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512i __builtin_ia32_pabsd512_mask(__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_pabsq512_mask(__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_paddd512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_paddq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pandd512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_pandnd512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_pandnq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pandq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pbroadcastd512(__cov_v4si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_pbroadcastd512_gpr_mask(int,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_pbroadcastq512(__cov_v2di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pbroadcastq512_gpr_mask(long long,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pbroadcastq512_mem_mask(long long,__cov_v8di,__cov_mmask8);
__cov_mmask16  __builtin_ia32_pcmpeqd512_mask(__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_mmask8 __builtin_ia32_pcmpeqq512_mask(__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_mmask16 __builtin_ia32_pcmpgtd512_mask(__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_mmask8 __builtin_ia32_pcmpgtq512_mask(__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512d __builtin_ia32_permvardf512_mask(__cov_v8df,__cov_v8di,__cov_v8df,__cov_mmask8);
__cov_m512i __builtin_ia32_permvardi512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512 __builtin_ia32_permvarsf512_mask(__cov_v16sf,__cov_v16si,__cov_v16sf,__cov_mmask16);
__cov_m512i __builtin_ia32_permvarsi512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_pmaxsd512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_pmaxsq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pmaxud512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_pmaxuq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pminsd512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_pminsq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pminud512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_pminuq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m128i __builtin_ia32_pmovdb512_mask(__cov_v16si,__cov_v16qi,__cov_mmask16);
void __builtin_ia32_pmovdb512mem_mask(__cov_v16qi *,__cov_v16si,__cov_mmask16);
__cov_m256i __builtin_ia32_pmovdw512_mask(__cov_v16si,__cov_v16hi,__cov_mmask16);
void __builtin_ia32_pmovdw512mem_mask(__cov_v16hi *,__cov_v16si,__cov_mmask16);
__cov_m128i __builtin_ia32_pmovqb512_mask(__cov_v8di,__cov_v16qi,__cov_mmask8);
void __builtin_ia32_pmovqb512mem_mask(__cov_v16qi *,__cov_v8di,__cov_mmask8);
__cov_m256i __builtin_ia32_pmovqd512_mask(__cov_v8di,__cov_v8si,__cov_mmask8);
void __builtin_ia32_pmovqd512mem_mask(__cov_v8si *,__cov_v8di,__cov_mmask8);
__cov_m128i __builtin_ia32_pmovqw512_mask(__cov_v8di,__cov_v8hi,__cov_mmask8);
void __builtin_ia32_pmovqw512mem_mask(__cov_v8hi *,__cov_v8di,__cov_mmask8);
__cov_m128i __builtin_ia32_pmovsdb512_mask(__cov_v16si,__cov_v16qi,__cov_mmask16);
void __builtin_ia32_pmovsdb512mem_mask(__cov_v16qi *,__cov_v16si,__cov_mmask16);
__cov_m256i __builtin_ia32_pmovsdw512_mask(__cov_v16si,__cov_v16hi,__cov_mmask16);
void __builtin_ia32_pmovsdw512mem_mask(__cov_v16hi *,__cov_v16si,__cov_mmask16);
__cov_m128i __builtin_ia32_pmovsqb512_mask(__cov_v8di,__cov_v16qi,__cov_mmask8);
void __builtin_ia32_pmovsqb512mem_mask(__cov_v16qi *,__cov_v8di,__cov_mmask8);
__cov_m256i __builtin_ia32_pmovsqd512_mask(__cov_v8di,__cov_v8si,__cov_mmask8);
void __builtin_ia32_pmovsqd512mem_mask(__cov_v8si *,__cov_v8di,__cov_mmask8);
__cov_m128i __builtin_ia32_pmovsqw512_mask(__cov_v8di,__cov_v8hi,__cov_mmask8);
void __builtin_ia32_pmovsqw512mem_mask(__cov_v8hi *,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pmovsxbd512_mask(__cov_v16qi,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_pmovsxbq512_mask(__cov_v16qi,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pmovsxdq512_mask(__cov_v8si,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pmovsxwd512_mask(__cov_v16hi,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_pmovsxwq512_mask(__cov_v8hi,__cov_v8di,__cov_mmask8);
__cov_m128i __builtin_ia32_pmovusdb512_mask(__cov_v16si,__cov_v16qi,__cov_mmask16);
void __builtin_ia32_pmovusdb512mem_mask(__cov_v16qi *,__cov_v16si,__cov_mmask16);
__cov_m256i __builtin_ia32_pmovusdw512_mask(__cov_v16si,__cov_v16hi,__cov_mmask16);
void __builtin_ia32_pmovusdw512mem_mask(__cov_v16hi *,__cov_v16si,__cov_mmask16);
__cov_m128i __builtin_ia32_pmovusqb512_mask(__cov_v8di,__cov_v16qi,__cov_mmask8);
void __builtin_ia32_pmovusqb512mem_mask(__cov_v16qi *,__cov_v8di,__cov_mmask8);
__cov_m256i __builtin_ia32_pmovusqd512_mask(__cov_v8di,__cov_v8si,__cov_mmask8);
void __builtin_ia32_pmovusqd512mem_mask(__cov_v8si *,__cov_v8di,__cov_mmask8);
__cov_m128i __builtin_ia32_pmovusqw512_mask(__cov_v8di,__cov_v8hi,__cov_mmask8);
void __builtin_ia32_pmovusqw512mem_mask(__cov_v8hi *,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pmovzxbd512_mask(__cov_v16qi,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_pmovzxbq512_mask(__cov_v16qi,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pmovzxdq512_mask(__cov_v8si,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pmovzxwd512_mask(__cov_v16hi,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_pmovzxwq512_mask(__cov_v8hi,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pmuldq512_mask(__cov_v16si,__cov_v16si,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pmulld512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_pmuludq512_mask(__cov_v16si,__cov_v16si,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_psllqi512_mask(__cov_v8di, unsigned int, __cov_v8di, __cov_mmask8);
__cov_m512i __builtin_ia32_pord512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_porq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_prolvd512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_prolvq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_prorvd512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_prorvq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pslld512_mask(__cov_v16si,__cov_v4si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_psrldi512_mask(__cov_v16si,unsigned int,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_psllq512_mask(__cov_v8di,__cov_v2di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_psrlqi512_mask(__cov_v8di, int, __cov_v8di, __cov_mmask8);
__cov_m512i __builtin_ia32_psllv16si_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_psllv8di_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_psrad512_mask(__cov_v16si,__cov_v4si,__cov_v16si,__cov_mmask16);
__cov_m128d __builtin_ia32_addsd_round(__cov_v2df,__cov_v2df,int);
__cov_m128  __builtin_ia32_addss_round(__cov_v4sf,__cov_v4sf,int);
__cov_m128d __builtin_ia32_subsd_round(__cov_v2df,__cov_v2df,int);
__cov_m128  __builtin_ia32_subss_round(__cov_v4sf,__cov_v4sf,int);
__cov_v16si __builtin_ia32_pshufd512_mask(__cov_v16si,int,__cov_v16si,__cov_mmask16);
__cov_v8di  __builtin_ia32_shuf_i64x2_mask(__cov_v8di,__cov_v8di,int,__cov_v8di,__cov_mmask8);
__cov_v16si __builtin_ia32_shuf_i32x4_mask(__cov_v16si,__cov_v16si,int,__cov_v16si,__cov_mmask16);
__cov_v8df  __builtin_ia32_shuf_f64x2_mask(__cov_v8df,__cov_v8df,int,__cov_v8df,__cov_mmask8);
__cov_v16sf __builtin_ia32_shuf_f32x4_mask(__cov_v16sf,__cov_v16sf,int,__cov_v16sf,__cov_mmask16);
__cov_v16hi __builtin_ia32_pshufhw256_mask(__cov_v16hi,int,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_pshuflw256_mask(__cov_v16hi,int,__cov_v16hi,__cov_mmask16);
__cov_m512i __builtin_ia32_pternlogq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,int,__cov_mmask8);
__cov_m512i __builtin_ia32_pternlogq512_maskz(__cov_v8di,__cov_v8di,__cov_v8di,int,__cov_mmask8);
__cov_m512i __builtin_ia32_pternlogd512_mask(__cov_v16si,__cov_v16si,__cov_v16si,int,__cov_mmask16);
__cov_m512i __builtin_ia32_pternlogd512_maskz(__cov_v16si,__cov_v16si,__cov_v16si,int,__cov_mmask16);
__cov_m512i __builtin_ia32_psraq512_mask(__cov_v8di,__cov_v2di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pslldi512_mask(__cov_v16si,unsigned int,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_psrav16si_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_psrav8di_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_psrld512_mask(__cov_v16si,__cov_v4si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_psradi512_mask(__cov_v16si,unsigned int,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_psrlq512_mask(__cov_v8di,__cov_v2di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_psraqi512_mask(__cov_v8di,int,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_psrlv16si_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_psrlv8di_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_psubd512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_psubq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_mmask16 __builtin_ia32_ptestmd512(__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_mmask8  __builtin_ia32_ptestmq512(__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_mmask16 __builtin_ia32_ptestnmd512(__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_mmask8 __builtin_ia32_ptestnmq512(__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_punpckhdq512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_punpckhqdq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_punpckldq512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_punpcklqdq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_pxord512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_pxorq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512d __builtin_ia32_rcp14pd512_mask(__cov_v8df,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_rcp14ps512_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_m128d __builtin_ia32_rcp14sd(__cov_v2df,__cov_v2df);
__cov_m128 __builtin_ia32_rcp14ss(__cov_v4sf,__cov_v4sf);
__cov_m512d __builtin_ia32_rndscalepd_mask(__cov_v8df,int,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_rndscaleps_mask(__cov_v16sf,int,__cov_v16sf,__cov_mmask16,int);
__cov_m512d __builtin_ia32_rsqrt14pd512_mask(__cov_v8df,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_rsqrt14ps512_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_m128d __builtin_ia32_rsqrt14sd(__cov_v2df,__cov_v2df);
__cov_m128 __builtin_ia32_rsqrt14ss(__cov_v4sf,__cov_v4sf);
__cov_m512d __builtin_ia32_scalefpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_scalefps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m128d __builtin_ia32_scalefsd_round(__cov_v2df,__cov_v2df,int);
__cov_m128 __builtin_ia32_scalefss_round(__cov_v4sf,__cov_v4sf,int);
__cov_m512d __builtin_ia32_sqrtpd512_mask(__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_sqrtps512_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m128d __builtin_ia32_sqrtsd_round(__cov_v2df,__cov_v2df,int);
__cov_m128d __builtin_ia32_sqrtss_round(__cov_v4sf,__cov_v4sf,int);
void __builtin_ia32_storeapd512_mask(__cov_v8df *,__cov_v8df,__cov_mmask8);
void __builtin_ia32_storeaps512_mask(__cov_v16sf *,__cov_v16sf,__cov_mmask16);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
void __builtin_ia32_storedqudi512_mask(long long *,__cov_v8di,__cov_mmask8);
void __builtin_ia32_storedqusi512_mask(int *,__cov_v16si,__cov_mmask16);
void  __builtin_ia32_storeupd512_mask(double *,__cov_v8df,__cov_mmask8);
void __builtin_ia32_storeups512_mask(float *,__cov_v16sf,__cov_mmask16);
#else
void __builtin_ia32_storedqudi512_mask(__cov_v8di *,__cov_v8di,__cov_mmask8);
void __builtin_ia32_storedqusi512_mask(__cov_v16si *,__cov_v16si,__cov_mmask16);
void  __builtin_ia32_storeupd512_mask(__cov_v8df *,__cov_v8df,__cov_mmask8);
void __builtin_ia32_storeups512_mask(__cov_v16sf *,__cov_v16sf,__cov_mmask16);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_m512d __builtin_ia32_subpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_subps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_mmask16 __builtin_ia32_ucmpd512_mask(__cov_v16si,__cov_v16si,int,__cov_mmask16);
__cov_mmask8 __builtin_ia32_ucmpq512_mask(__cov_v8di,__cov_v8di,int,__cov_mmask8);
__cov_m512d __builtin_ia32_unpckhpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_unpckhps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_m512d __builtin_ia32_unpcklpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_unpcklps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_m512 __builtin_ia32_vcvtph2ps512_mask(__cov_v16hi,__cov_v16sf,__cov_mmask16,int);
unsigned __builtin_ia32_vcvtsd2usi32(__cov_v2df,int);
unsigned long long __builtin_ia32_vcvtsd2usi64(__cov_v2df,int);
unsigned __builtin_ia32_vcvtss2usi32(__cov_v4sf,int);
unsigned long long __builtin_ia32_vcvtss2usi64(__cov_v4sf,int);
int __builtin_ia32_vcvttsd2si32(__cov_v2df,int);
long long __builtin_ia32_vcvttsd2si64(__cov_v2df,int);
unsigned __builtin_ia32_vcvttsd2usi32(__cov_v2df,int);
unsigned long long __builtin_ia32_vcvttsd2usi64(__cov_v2df,int);
int __builtin_ia32_vcvttss2si32(__cov_v4sf,int);
long long __builtin_ia32_vcvttss2si64(__cov_v4sf,int);
unsigned __builtin_ia32_vcvttss2usi32(__cov_v4sf,int);
unsigned long long __builtin_ia32_vcvttss2usi64(__cov_v4sf,int);
__cov_m512d __builtin_ia32_vfmaddpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512d __builtin_ia32_vfmaddpd512_mask3(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512d __builtin_ia32_vfmaddpd512_maskz(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_vfmaddps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512 __builtin_ia32_vfmaddps512_mask3(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512 __builtin_ia32_vfmaddps512_maskz(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512d __builtin_ia32_vfmaddsubpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512d __builtin_ia32_vfmaddsubpd512_mask3(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512d __builtin_ia32_vfmaddsubpd512_maskz(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_vfmaddsubps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512 __builtin_ia32_vfmaddsubps512_mask3(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512 __builtin_ia32_vfmaddsubps512_maskz(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512d __builtin_ia32_vfmsubaddpd512_mask3(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_vfmsubaddps512_mask3(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512d __builtin_ia32_vfmsubpd512_mask3(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_vfmsubps512_mask3(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512d __builtin_ia32_vfnmaddpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_vfnmaddps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512d __builtin_ia32_vfnmsubpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512d __builtin_ia32_vfnmsubpd512_mask3(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_vfnmsubps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512 __builtin_ia32_vfnmsubps512_mask3(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512i __builtin_ia32_vpermi2vard512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512d __builtin_ia32_vpermi2varpd512_mask(__cov_v8df,__cov_v8di,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_vpermi2varps512_mask(__cov_v16sf,__cov_v16si,__cov_v16sf,__cov_mmask16);
__cov_m512i __builtin_ia32_vpermi2varq512_mask(__cov_v8di, __cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512d __builtin_ia32_vpermilvarpd512_mask(__cov_v8df,__cov_v8di,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_vpermilvarps512_mask(__cov_v16sf,__cov_v16si,__cov_v16sf,__cov_mmask16);
__cov_m512i __builtin_ia32_vpermt2vard512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_vpermt2vard512_maskz(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512d __builtin_ia32_vpermt2varpd512_mask(__cov_v8di,__cov_v8df,__cov_v8df,__cov_mmask8);
__cov_m512d __builtin_ia32_vpermt2varpd512_maskz(__cov_v8di,__cov_v8df,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_vpermt2varps512_mask(__cov_v16si,__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_m512 __builtin_ia32_vpermt2varps512_maskz(__cov_v16si,__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_m512d __builtin_ia32_vpermilpd512_mask(__cov_v8df,int,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_vpermilps512_mask(__cov_v16sf,int,__cov_v16sf,__cov_mmask16);
__cov_m512i __builtin_ia32_permdi512_mask(__cov_v8di,int,__cov_v8di,__cov_mmask8);
__cov_m512d __builtin_ia32_permdf512_mask(__cov_v8df,int,__cov_v8df,__cov_mmask8);
__cov_m512i __builtin_ia32_permvardi512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_permvarsi512_mask(__cov_v16si,__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512d __builtin_ia32_permvardf512_mask(__cov_v8df,__cov_v8di,__cov_v8df,__cov_mmask8);
__cov_m512 __builtin_ia32_permvarsf512_mask(__cov_v16sf,__cov_v16si,__cov_v16sf,__cov_mmask16);
__cov_m512i __builtin_ia32_vpermt2varq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_vpermt2varq512_maskz(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512 __builtin_ia32_shufps512_mask(__cov_v16sf,__cov_v16sf,int,__cov_v16sf,__cov_mmask16);
__cov_m512d __builtin_ia32_shufpd512_mask(__cov_v8df,__cov_v8df,int,__cov_v8df,__cov_mmask8);
__cov_m512d __builtin_ia32_fixupimmpd512_mask(__cov_v8df,__cov_v8df,__cov_v8di,int,__cov_mmask8,int);
__cov_m512 __builtin_ia32_fixupimmps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16si,int,__cov_mmask16,int);
__cov_m128d __builtin_ia32_fixupimmsd_mask(__cov_v2df,__cov_v2df,__cov_v2di,int,__cov_mmask8,int);
__cov_m128 __builtin_ia32_fixupimmss_mask(__cov_v4sf,__cov_v4sf,__cov_v4si,int,__cov_mmask8,int);
__cov_m512d __builtin_ia32_fixupimmpd512_maskz(__cov_v8df,__cov_v8df,__cov_v8di,int,__cov_mmask8,int);
__cov_m512 __builtin_ia32_fixupimmps512_maskz(__cov_v16sf,__cov_v16sf,__cov_v16si,int,__cov_mmask16,int);
__cov_m128 __builtin_ia32_fixupimmss_maskz(__cov_v4sf,__cov_v4sf,__cov_v4si,int,__cov_mmask8,int);
__cov_m512i __builtin_ia32_prold512_mask(__cov_v16si,int,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_prord512_mask(__cov_v16si,int,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_prolq512_mask(__cov_v8di,int,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_prorq512_mask(__cov_v8di,int,__cov_v8di,__cov_mmask8);
__cov_m128d __builtin_ia32_fixupimmsd_maskz(__cov_v2df,__cov_v2df,__cov_v2di,int,__cov_mmask8,int);
__cov_m256i __builtin_ia32_vcvtps2ph512_mask(__cov_v16sf,int,__cov_v16hi,int);
__cov_m128 __builtin_ia32_cvtsd2ss_round(__cov_v4sf,__cov_v2df,int);
__cov_m128d __builtin_ia32_cvtss2sd_round(__cov_v2df,__cov_v4sf,int);
__cov_m128  __builtin_ia32_getexpss128_round(__cov_v4sf,__cov_v4sf,int);
__cov_m128d __builtin_ia32_getexpsd128_round(__cov_v2df,__cov_v2df,int);
__cov_m512 __builtin_ia32_getexpps512_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512d __builtin_ia32_getexppd512_mask(__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512d __builtin_ia32_getmantpd512_mask(__cov_v8df,int,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_getmantps512_mask(__cov_v16sf,int,__cov_v16sf,__cov_mmask16,int);
__cov_m128d __builtin_ia32_getmantsd_round(__cov_v2df,__cov_v2df,int,int);
__cov_m128 __builtin_ia32_getmantss_round(__cov_v4sf,__cov_v4sf,int,int);
__cov_m128 __builtin_ia32_rndscaless_round(__cov_v4sf,__cov_v4sf,int,int);
__cov_m128d __builtin_ia32_rndscalesd_round(__cov_v2df,__cov_v2df,int,int);
__cov_m512i __builtin_ia32_alignd512_mask(__cov_v16si,__cov_v16si,int,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_alignq512_mask(__cov_v8di,__cov_v8di,int,__cov_v8di,__cov_mmask8);
__cov_mmask16 __builtin_ia32_pcmpeqd512_mask(__cov_v16si,__cov_v16si,__cov_mmask16);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_m512 __builtin_ia32_gathersiv16sf(__cov_v16sf,const void*,__cov_v16si,__cov_mmask16,int);
__cov_m512d __builtin_ia32_gathersiv8df(__cov_v8df,const void*,__cov_v8si,__cov_mmask8,int);
__cov_m256 __builtin_ia32_gatherdiv16sf(__cov_v8sf,const void*,__cov_v8di,__cov_mmask8,int);
__cov_m512d __builtin_ia32_gatherdiv8df(__cov_v8df,const void*,__cov_v8di,__cov_mmask8,int);
__cov_m512i __builtin_ia32_gathersiv16si(__cov_v16si,const void*,__cov_v16si,__cov_mmask16,int);
__cov_m512i __builtin_ia32_gathersiv8di(__cov_v8di,const void*,__cov_v8si,__cov_mmask8,int);
__cov_m256i __builtin_ia32_gatherdiv16si(__cov_v8si,const void*,__cov_v8di,__cov_mmask8,int);
__cov_m512i __builtin_ia32_gatherdiv8di(__cov_v8di,const void*,__cov_v8di,__cov_mmask8,int);
#else
__cov_m512 __builtin_ia32_gathersiv16sf(__cov_v16sf,const float *,__cov_v16si,__cov_mmask16,int);
__cov_m512d __builtin_ia32_gathersiv8df(__cov_v8df,const double *,__cov_v8si,__cov_mmask8,int);
__cov_m256 __builtin_ia32_gatherdiv16sf(__cov_v8sf,const float *,__cov_v8di,__cov_mmask8,int);
__cov_m512d __builtin_ia32_gatherdiv8df(__cov_v8df,const double *,__cov_v8di,__cov_mmask8,int);
__cov_m512i __builtin_ia32_gathersiv16si(__cov_v16si,const int *,__cov_v16si,__cov_mmask16,int);
__cov_m512i __builtin_ia32_gathersiv8di(__cov_v8di,const long long *,__cov_v8si,__cov_mmask8,int);
__cov_m256i __builtin_ia32_gatherdiv16si(__cov_v8si,const int *,__cov_v8di,__cov_mmask8,int);
__cov_m512i __builtin_ia32_gatherdiv8di(__cov_v8di,const long long*,__cov_v8di,__cov_mmask8,int);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_m128d __builtin_ia32_maxsd_round(__cov_v2df,__cov_v2df,int);
__cov_m128d __builtin_ia32_maxss_round(__cov_v4sf,__cov_v4sf,int);
__cov_m128d __builtin_ia32_minsd_round(__cov_v2df,__cov_v2df,int);
__cov_m128d __builtin_ia32_minss_round(__cov_v4sf,__cov_v4sf,int);
__cov_m128d __builtin_ia32_vfmaddsd3_round(__cov_v2df,__cov_v2df,__cov_v2df, int);
__cov_m128 __builtin_ia32_vfmaddss3_round(__cov_v4sf,__cov_v4sf,__cov_v4sf,int);
__cov_m512d __builtin_ia32_exp2pd_mask(__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_exp2ps_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m512d __builtin_ia32_rcp28pd_mask(__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_rcp28ps_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m128d __builtin_ia32_rcp28sd_round(__cov_v2df,__cov_v2df,int);
__cov_m128d __builtin_ia32_rcp28ss_round(__cov_v4sf,__cov_v4sf,int);
__cov_m512d __builtin_ia32_rsqrt28pd_mask(__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_m512 __builtin_ia32_rsqrt28ps_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_m128d __builtin_ia32_rsqrt28sd_round(__cov_v2df,__cov_v2df,int);
__cov_m128 __builtin_ia32_rsqrt28ss_round(__cov_v4sf,__cov_v4sf,int);
__cov_m128i __builtin_ia32_sha1rnds4(__cov_v4si,__cov_v4si,int);
__cov_v8hi __builtin_ia32_vcvtps2ph(__cov_v4sf,int);
__cov_m128i __builtin_ia32_vcvtps2ph256(__cov_v8sf, int);
__cov_m128i __builtin_ia32_vprotbi(__cov_v16qi, int);
__cov_m128i __builtin_ia32_vprotwi(__cov_v8hi,int);
__cov_m128i __builtin_ia32_vprotdi(__cov_v4si,int);
__cov_m128i __builtin_ia32_vprotqi(__cov_v2di,int);
__cov_m128d __builtin_ia32_vpermil2pd(__cov_v2df,__cov_v2df,__cov_v2di,int);
__cov_m256d __builtin_ia32_vpermil2pd256(__cov_v4df,__cov_v4df,__cov_v4di,int);
__cov_m128 __builtin_ia32_vpermil2ps(__cov_v4sf,__cov_v4sf,__cov_v4si,int);
__cov_m256 __builtin_ia32_vpermil2ps256(__cov_v8sf,__cov_v8sf,__cov_v8si,int);
long long __builtin_ia32_vcvtss2si64(__cov_v4sf,int);
long long __builtin_ia32_vcvttss2si64(__cov_v4sf,int);
int __builtin_ia32_vcvtss2si32(__cov_v4sf,int);
long long __builtin_ia32_vcvtsd2si64(__cov_v2df, int);
int __builtin_ia32_vcvtsd2si32(__cov_v2df,int);
__cov_mmask8 __builtin_ia32_cmppd512_mask(__cov_v8df,__cov_v8df,int,__cov_mmask8,int);
__cov_mmask16 __builtin_ia32_cmpps512_mask(__cov_v16sf,__cov_v16sf,int,__cov_mmask16,int);
__cov_mmask8 __builtin_ia32_cmpsd_mask(__cov_v2df,__cov_v2df,int,__cov_mmask8,int);
__cov_mmask8 __builtin_ia32_cmpss_mask(__cov_v4sf,__cov_v4sf,int,__cov_mmask8,int);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
void __builtin_ia32_scattersiv16sf(void*, __cov_mmask16,__cov_v16si,__cov_v16sf,int);
void __builtin_ia32_scattersiv8df(void *,__cov_mmask8,__cov_v8si,__cov_v8df,int);
void __builtin_ia32_scatterdiv16sf(void*,__cov_mmask8,__cov_v8di,__cov_v8sf,int);
void __builtin_ia32_scatterdiv8df(void*,__cov_mmask8,__cov_v8di,__cov_v8df,int);
void __builtin_ia32_scattersiv16si(void*,__cov_mmask16,__cov_v16si,__cov_v16si,int);
void __builtin_ia32_scattersiv8di(void*,__cov_mmask8,__cov_v8si,__cov_v8di,int);
void __builtin_ia32_scatterdiv16si(void*,__cov_mmask8,__cov_v8di,__cov_v8si,int);
void __builtin_ia32_scatterdiv8di(void*,__cov_mmask8,__cov_v8di,__cov_v8di,int);
#else
void __builtin_ia32_scattersiv16sf(float *, __cov_mmask16,__cov_v16si,__cov_v16sf,int);
void __builtin_ia32_scattersiv8df(double *,__cov_mmask8,__cov_v8si,__cov_v8df,int);
void __builtin_ia32_scatterdiv16sf(float *,__cov_mmask8,__cov_v8di,__cov_v8sf,int);
void __builtin_ia32_scatterdiv8df(double *,__cov_mmask8,__cov_v8di,__cov_v8df,int);
void __builtin_ia32_scattersiv16si(int *,__cov_mmask16,__cov_v16si,__cov_v16si,int);
void __builtin_ia32_scattersiv8di(long long*,__cov_mmask8,__cov_v8si,__cov_v8di,int);
void __builtin_ia32_scatterdiv16si(int *,__cov_mmask8,__cov_v8di,__cov_v8si,int);
void __builtin_ia32_scatterdiv8di(long long*,__cov_mmask8,__cov_v8di,__cov_v8di,int);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
void __builtin_ia32_scattersiv8df(double *,__cov_mmask8,__cov_v8si,__cov_v8df,int);
void __builtin_ia32_scatterdiv16sf(float *,__cov_mmask8,__cov_v8di,__cov_v8sf,int);
void __builtin_ia32_scatterdiv8df(double *,__cov_mmask8,__cov_v8di,__cov_v8df,int);
void __builtin_ia32_scattersiv16si(int *,__cov_mmask16,__cov_v16si,__cov_v16si,int);
void __builtin_ia32_scattersiv8di(long long*,__cov_mmask8,__cov_v8si,__cov_v8di,int);
void __builtin_ia32_scatterdiv16si(int *,__cov_mmask8,__cov_v8di,__cov_v8si,int);
void __builtin_ia32_scatterdiv8di(long long*,__cov_mmask8,__cov_v8di,__cov_v8di,int);
int __builtin_ia32_vcomiss(__cov_v4sf,__cov_v4sf,int,int);
int __builtin_ia32_vcomisd(__cov_v2df,__cov_v2df,int,int);
#endif /* __AVX512F__ || __COVERITY_GCC49_INTRINSICS */

/*
 * AVX-512 (Advanced Vector eXtensions) foundation intrinsics.
 * Introduced in GCC 4.9.
 */
#if defined(__AVX512PF__) || __COVERITY_GCC49_INTRINSICS
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
void __builtin_ia32_gatherpfdpd(__cov_mmask8,__cov_v8si,const void*,int,int);
void __builtin_ia32_gatherpfdps(__cov_mmask16,__cov_v16si,const void*,int,int);
void __builtin_ia32_gatherpfqpd(__cov_mmask8, __cov_v8di,const void *,int,int);
void __builtin_ia32_gatherpfqps(__cov_mmask8,__cov_v8di,const void*,int,int);
void __builtin_ia32_scatterpfdpd(__cov_mmask8, __cov_v8si,const void *,int,int);
void __builtin_ia32_scatterpfdps(__cov_mmask16,__cov_v16si,const void *,int,int);
void __builtin_ia32_scatterpfqpd(__cov_mmask8,__cov_v8di,const void*,int,int);
void __builtin_ia32_scatterpfqps(__cov_mmask8,__cov_v8di,const void*,int,int);
#else
void __builtin_ia32_gatherpfdpd(__cov_mmask8,__cov_v8si,const long long *,int,int);
void __builtin_ia32_gatherpfdps(__cov_mmask16,__cov_v16si,const int *,int,int);
void __builtin_ia32_gatherpfqpd(__cov_mmask8, __cov_v8di,const long long *,int,int);
void __builtin_ia32_gatherpfqps(__cov_mmask8,__cov_v8di,const int *,int,int);
void __builtin_ia32_scatterpfdpd(__cov_mmask8, __cov_v8si,const long long *,int,int);
void __builtin_ia32_scatterpfdps(__cov_mmask16,__cov_v16si,const int *,int,int);
void __builtin_ia32_scatterpfqpd(__cov_mmask8,__cov_v8di,const long long *,int,int);
void __builtin_ia32_scatterpfqps(__cov_mmask8,__cov_v8di,const int *,int,int);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
#endif


/*
 * AVX-512 (Advanced Vector eXtensions) conflict detection intrinsics.
 * Introduced in GCC 4.9.
 */
#if defined(__AVX512CD__) || __COVERITY_GCC49_INTRINSICS
__cov_m512i __builtin_ia32_vpconflictsi_512_mask(__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_vpconflictdi_512_mask(__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_vplzcntq_512_mask(__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_vplzcntd_512_mask(__cov_v16si,__cov_v16si,__cov_mmask16);
__cov_m512i __builtin_ia32_broadcastmb512(__cov_mmask8);
__cov_m512i __builtin_ia32_broadcastmw512(__cov_mmask16);
#endif /* __AVX512CD__ || __COVERITY_GCC49_INTRINSICS */

/*
 * SHA (Secure Hash Algorithm) intrinsics.
 * Introduced in GCC 4.9.
 */
#if defined(__SHA__) || __COVERITY_GCC49_INTRINSICS
__cov_m128i __builtin_ia32_sha1msg1(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_sha1msg2(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_sha1nexte(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_sha256msg1(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_sha256msg2(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_sha256rnds2(__cov_v4si,__cov_v4si,__cov_v4si);
#endif /* __SHA__ || __COVERITY_GCC49_INTRINSICS */

/*
 * F16C intrinsics (a.k.a. CVT16).
 * Introduced in GCC 4.6., officially supported in GCC 4.7.
 */
#if defined(__F16C__) || __COVERITY_GCC49_INTRINSICS
__cov_m128 __builtin_ia32_vcvtph2ps(__cov_v8hi);
__cov_m256 __builtin_ia32_vcvtph2ps256(__cov_v8hi);
#endif /* __F16C__ || __COVERITY_GCC49_INTRINSICS */

/*
 * Advanced Bit Manipulation intrinsics.
 * Introduced in GCC 4.7.
 */
#if defined(__LZCNT__) || __COVERITY_GCC49_INTRINSICS
unsigned short __builtin_clzs(unsigned short);
#endif /* __LZCNT__ || __COVERITY_GCC49_INTRINSICS */

/*
 * BMI (bit manipulation) intrinsics.
 * Introduced in GCC 4.6.
 */
#if defined(__BMI__) || __COVERITY_GCC49_INTRINSICS
unsigned short __builtin_ctzs(unsigned short);
unsigned int __builtin_ia32_bextr_u32(unsigned int,unsigned int);
unsigned long long __builtin_ia32_bextr_u64(unsigned long long,unsigned long long);
#endif /* __BMI__ || __COVERITY_GCC49_INTRINSICS */

/*
 * BMI2 (bit manipulation) intrinsics.
 * Introduced in GCC 4.7.
 */
#if defined(__BMI2__) || __COVERITY_GCC49_INTRINSICS
unsigned int __builtin_ia32_bzhi_si(unsigned int,unsigned int);
unsigned int __builtin_ia32_pdep_si(unsigned int,unsigned int);
unsigned int __builtin_ia32_pext_si(unsigned int,unsigned int);
unsigned long long __builtin_ia32_bzhi_di(unsigned long long,unsigned long long);
unsigned long long __builtin_ia32_pdep_di(unsigned long long,unsigned long long);
unsigned long long __builtin_ia32_pext_di(unsigned long long,unsigned long long);
#endif /* __BMI2__ || __COVERITY_GCC49_INTRINSICS */

/*
 * RTM (Restricted Transactional Memory) intrinsics.
 * Introduced in GCC 4.8.
 */
#if defined(__RTM__) || __COVERITY_GCC49_INTRINSICS
unsigned int __builtin_ia32_xbegin(void);
void __builtin_ia32_xend(void);
void __builtin_ia32_xabort(const unsigned int);
int __builtin_ia32_xtest(void);
#endif /* __RTM__ || __COVERITY_GCC49_INTRINSICS */

/*
 * RdRand intrinsics, a.k.a. Bull Mountain.
 * Introduced in GCC 4.6, officially supported in GCC 4.7.
 */
#if defined(__RDRND__) || __COVERITY_GCC49_INTRINSICS
int __builtin_ia32_rdrand16_step(unsigned short *);
int __builtin_ia32_rdrand32_step(unsigned int *);
int __builtin_ia32_rdrand64_step(unsigned long long *);
#endif /* __RDRND__ || __COVERITY_GCC49_INTRINSICS */

/*
 * FSGS base accesor intrinsics.
 * Introduced in GCC 4.6, officially supported in GCC 4.7.
 */
#if defined(__FSGSBASE__) || __COVERITY_GCC49_INTRINSICS
unsigned int __builtin_ia32_rdfsbase32(void);
unsigned long long __builtin_ia32_rdfsbase64(void);
unsigned int __builtin_ia32_rdgsbase32(void);
unsigned long long __builtin_ia32_rdgsbase64(void);
void __builtin_ia32_wrfsbase32(unsigned int);
void __builtin_ia32_wrfsbase64(unsigned long long);
void __builtin_ia32_wrgsbase32(unsigned int);
void __builtin_ia32_wrgsbase64(unsigned long long);
#endif /* __FSGSBASE__ || __COVERITY_GCC49_INTRINSICS */

/*
 * XOP (eXtended Operations) Intrinsics.
 * Introduced in GCC 4.5.
 */
#if defined(__XOP__) || __COVERITY_GCC49_INTRINSICS
__cov_m128d __builtin_ia32_vfrczpd(__cov_v2df);
__cov_m256d __builtin_ia32_vfrczpd256(__cov_v4df);
__cov_m128 __builtin_ia32_vfrczps(__cov_v4sf);
__cov_m256 __builtin_ia32_vfrczps256(__cov_v8sf);
__cov_v2df __builtin_ia32_vfrczsd(__cov_v2df);
__cov_v4sf __builtin_ia32_vfrczss(__cov_v4sf);
__cov_m128i __builtin_ia32_vpcmov(__cov_m128i,__cov_m128i,__cov_m128i);
__cov_m128i __builtin_ia32_vpcomeqb(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpcomeqd(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpcomeqq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpcomequb(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpcomequd(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpcomequq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpcomequw(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpcomeqw(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpcomfalseb(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpcomfalsed(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpcomfalseq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpcomfalseub(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpcomfalseud(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpcomfalseuq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpcomfalseuw(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpcomfalsew(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpcomgeb(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpcomged(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpcomgeq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpcomgeub(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpcomgeud(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpcomgeuq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpcomgeuw(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpcomgew(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpcomgtb(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpcomgtd(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpcomgtq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpcomgtub(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpcomgtud(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpcomgtuq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpcomgtuw(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpcomgtw(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpcomleb(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpcomled(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpcomleq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpcomleub(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpcomleud(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpcomleuq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpcomleuw(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpcomlew(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpcomltb(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpcomltd(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpcomltq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpcomltub(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpcomltud(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpcomltuq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpcomltuw(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpcomltw(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpcomneqb(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpcomneqd(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpcomneqq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpcomnequb(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpcomnequd(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpcomnequq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpcomnequw(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpcomneqw(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpcomtrueb(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpcomtrued(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpcomtrueq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpcomtrueub(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpcomtrueud(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpcomtrueuq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpcomtrueuw(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpcomtruew(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vphaddbd(__cov_v16qi);
__cov_m128i __builtin_ia32_vphaddbq(__cov_v16qi);
__cov_m128i __builtin_ia32_vphaddbw(__cov_v16qi);
__cov_m128i __builtin_ia32_vphadddq(__cov_v4si);
__cov_m128i __builtin_ia32_vphaddubd(__cov_v16qi);
__cov_m128i __builtin_ia32_vphaddubq(__cov_v16qi);
__cov_m128i __builtin_ia32_vphaddubw(__cov_v16qi);
__cov_m128i __builtin_ia32_vphaddudq(__cov_v4si);
__cov_m128i __builtin_ia32_vphadduwd(__cov_v8hi);
__cov_m128i __builtin_ia32_vphadduwq(__cov_v8hi);
__cov_m128i __builtin_ia32_vphaddwd(__cov_v8hi);
__cov_m128i __builtin_ia32_vphaddwq(__cov_v8hi);
__cov_m128i __builtin_ia32_vphsubbw(__cov_v16qi);
__cov_m128i __builtin_ia32_vphsubdq(__cov_v4si);
__cov_m128i __builtin_ia32_vphsubwd(__cov_v8hi);
__cov_m128i __builtin_ia32_vpmacsdd(__cov_v4si,__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpmacsdqh(__cov_v4si,__cov_v4si,__cov_v2di);
__cov_m128i __builtin_ia32_vpmacsdql(__cov_v4si,__cov_v4si,__cov_v2di);
__cov_m128i __builtin_ia32_vpmacssdd(__cov_v4si,__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpmacssdqh(__cov_v4si,__cov_v4si,__cov_v2di);
__cov_m128i __builtin_ia32_vpmacssdql(__cov_v4si,__cov_v4si,__cov_v2di);
__cov_m128i __builtin_ia32_vpmacsswd(__cov_v8hi,__cov_v8hi,__cov_v4si);
__cov_m128i __builtin_ia32_vpmacssww(__cov_v8hi,__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpmacswd(__cov_v8hi,__cov_v8hi,__cov_v4si);
__cov_m128i __builtin_ia32_vpmacsww(__cov_v8hi,__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpmadcsswd(__cov_v8hi,__cov_v8hi,__cov_v4si);
__cov_m128i __builtin_ia32_vpmadcswd(__cov_v8hi,__cov_v8hi,__cov_v4si);
__cov_m128i __builtin_ia32_vpperm(__cov_v16qi,__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vprotb(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vprotd(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vprotq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vprotw(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpshab(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpshad(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpshaq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpshaw(__cov_v8hi,__cov_v8hi);
__cov_m128i __builtin_ia32_vpshlb(__cov_v16qi,__cov_v16qi);
__cov_m128i __builtin_ia32_vpshld(__cov_v4si,__cov_v4si);
__cov_m128i __builtin_ia32_vpshlq(__cov_v2di,__cov_v2di);
__cov_m128i __builtin_ia32_vpshlw(__cov_v8hi,__cov_v8hi);

/*
 * GCC bug 56788
 * Prior to 4.9, these intrinsics erroneously took
 * two arguments instead of just one.
 */
#if __COVERITY_GCC_VERSION_AT_LEAST(4, 9) || \
    ((__GNUC__ == 4 && __GNUC_MINOR__ == 8 && __GNUC_PATCHLEVEL__ >= 2) \
    && (defined(__linux__) || (defined(__unix__) && !defined(__CYGWIN__))))
__cov_v2df __builtin_ia32_vfrczsd(__cov_v2df);
__cov_v4sf __builtin_ia32_vfrczss(__cov_v4sf);
#else
__cov_v2df __builtin_ia32_vfrczsd(__cov_v2df, __cov_v2df);
__cov_v4sf __builtin_ia32_vfrczss(__cov_v4sf, __cov_v4sf);
#endif /* 4.9+ */

#endif /* __XOP__ || __COVERITY_GCC49_INTRINSICS */

/*
 * LWP (light weight profiling) intrinsics.
 * Introduced in GCC 4.5.
 */
#if defined(__LWP__) || __COVERITY_GCC49_INTRINSICS
void __builtin_ia32_llwpcb(void*);
void *__builtin_ia32_slwpcb(void);
void __builtin_ia32_lwpval32(unsigned int,unsigned int,unsigned int);
void __builtin_ia32_lwpval64(unsigned long long,unsigned int,unsigned int);
unsigned char __builtin_ia32_lwpins32(unsigned int,unsigned int,unsigned int);
unsigned char __builtin_ia32_lwpins64(unsigned long long,unsigned int,unsigned int);
#endif /* __LWP__ || __COVERITY_GCC49_INTRINSICS */

/*
 * TBM (trailing bit manipulation) intrinsics.
 */
#if defined(__TBM__) || __COVERITY_GCC49_INTRINSICS
unsigned int __builtin_ia32_bextri_u32(unsigned int,unsigned int);
unsigned long long __builtin_ia32_bextri_u64(unsigned long long,unsigned long long);
#endif

/*
 * RDSEED intrinsic.
 * Introduced in GCC 4.8.
 */
#if defined(__RDSEED__) || __COVERITY_GCC49_INTRINSICS
int __builtin_ia32_rdseed_hi_step(unsigned short *);
int __builtin_ia32_rdseed_si_step(unsigned int *);
int __builtin_ia32_rdseed_di_step(unsigned long long *);
#endif /* __RDSEED__ || __COVERITY_GCC49_INTRINSICS */

#endif /* 4.0+ */

#if __COVERITY_GCC_VERSION_AT_LEAST(5, 0)
typedef struct { } __cov_bounds_type;
__cov_bounds_type __builtin_ia32_bndint(__cov_bounds_type,__cov_bounds_type);
__cov_bounds_type __builtin_ia32_bndldx(void const *,void const *);
__cov_bounds_type __builtin_ia32_bndmk(void const *,unsigned long);
__cov_bounds_type __builtin_ia32_bndret(void const *);
__cov_mmask16 __builtin_ia32_cmpb128_mask(__cov_v16qi,__cov_v16qi,int,__cov_mmask16);
__cov_mmask16 __builtin_ia32_cmpps512_mask(__cov_v16sf,__cov_v16sf,int,__cov_mmask16,int);
__cov_mmask16 __builtin_ia32_cmpw256_mask(__cov_v16hi,__cov_v16hi,int,__cov_mmask16);
__cov_mmask16 __builtin_ia32_cvtb2mask128(__cov_v16qi);
__cov_mmask16 __builtin_ia32_cvtd2mask512(__cov_v16si);
__cov_mmask16 __builtin_ia32_cvtw2mask256(__cov_v16hi);
__cov_mmask16 __builtin_ia32_fpclassps512_mask(__cov_v16sf,int,__cov_mmask16);
__cov_mmask16 __builtin_ia32_pcmpeqb128_mask(__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_mmask16 __builtin_ia32_pcmpeqw256_mask(__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_mmask16 __builtin_ia32_pcmpgtb128_mask(__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_mmask16 __builtin_ia32_pcmpgtw256_mask(__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_mmask16 __builtin_ia32_ptestmb128(__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_mmask16 __builtin_ia32_ptestmw256(__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_mmask16 __builtin_ia32_ptestnmb128(__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_mmask16 __builtin_ia32_ptestnmw256(__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_mmask16 __builtin_ia32_ucmpb128_mask(__cov_v16qi,__cov_v16qi,int,__cov_mmask16);
__cov_mmask16 __builtin_ia32_ucmpw256_mask(__cov_v16hi,__cov_v16hi,int,__cov_mmask16);
__cov_mmask8 __builtin_ia32_cmpd128_mask(__cov_v4si,__cov_v4si,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_cmpd256_mask(__cov_v8si,__cov_v8si,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_cmppd128_mask(__cov_v2df,__cov_v2df,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_cmppd256_mask(__cov_v4df,__cov_v4df,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_cmppd512_mask(__cov_v8df,__cov_v8df,int,__cov_mmask8,int);
__cov_mmask8 __builtin_ia32_cmpps128_mask(__cov_v4sf,__cov_v4sf,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_cmpps256_mask(__cov_v8sf,__cov_v8sf,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_cmpq128_mask(__cov_v2di,__cov_v2di,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_cmpq256_mask(__cov_v4di,__cov_v4di,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_cmpsd_mask(__cov_v2df,__cov_v2df,int,__cov_mmask8,int);
__cov_mmask8 __builtin_ia32_cmpss_mask(__cov_v4sf,__cov_v4sf,int,__cov_mmask8,int);
__cov_mmask8 __builtin_ia32_cmpw128_mask(__cov_v8hi,__cov_v8hi,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_cvtd2mask128(__cov_v4si);
__cov_mmask8 __builtin_ia32_cvtd2mask256(__cov_v8si);
__cov_mmask8 __builtin_ia32_cvtq2mask128(__cov_v2di);
__cov_mmask8 __builtin_ia32_cvtq2mask256(__cov_v4di);
__cov_mmask8 __builtin_ia32_cvtq2mask512(__cov_v8di);
__cov_mmask8 __builtin_ia32_cvtw2mask128(__cov_v8hi);
__cov_mmask8 __builtin_ia32_fpclasspd128_mask(__cov_v2df,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_fpclasspd256_mask(__cov_v4df,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_fpclasspd512_mask(__cov_v8df,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_fpclassps128_mask(__cov_v4sf,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_fpclassps256_mask(__cov_v8sf,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_fpclasssd(__cov_v2df,int);
__cov_mmask8 __builtin_ia32_fpclassss(__cov_v4sf,int);
__cov_mmask8 __builtin_ia32_pcmpeqd128_mask(__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_mmask8 __builtin_ia32_pcmpeqd256_mask(__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_mmask8 __builtin_ia32_pcmpeqq128_mask(__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_mmask8 __builtin_ia32_pcmpeqq256_mask(__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_mmask8 __builtin_ia32_pcmpeqw128_mask(__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_mmask8 __builtin_ia32_pcmpgtd128_mask(__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_mmask8 __builtin_ia32_pcmpgtd256_mask(__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_mmask8 __builtin_ia32_pcmpgtq128_mask(__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_mmask8 __builtin_ia32_pcmpgtq256_mask(__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_mmask8 __builtin_ia32_pcmpgtw128_mask(__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_mmask8 __builtin_ia32_ptestmd128(__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_mmask8 __builtin_ia32_ptestmd256(__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_mmask8 __builtin_ia32_ptestmq128(__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_mmask8 __builtin_ia32_ptestmq256(__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_mmask8 __builtin_ia32_ptestmw128(__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_mmask8 __builtin_ia32_ptestnmd128(__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_mmask8 __builtin_ia32_ptestnmd256(__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_mmask8 __builtin_ia32_ptestnmq128(__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_mmask8 __builtin_ia32_ptestnmq256(__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_mmask8 __builtin_ia32_ptestnmw128(__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_mmask8 __builtin_ia32_ucmpd128_mask(__cov_v4si,__cov_v4si,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_ucmpd256_mask(__cov_v8si,__cov_v8si,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_ucmpq128_mask(__cov_v2di,__cov_v2di,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_ucmpq256_mask(__cov_v4di,__cov_v4di,int,__cov_mmask8);
__cov_mmask8 __builtin_ia32_ucmpw128_mask(__cov_v8hi,__cov_v8hi,int,__cov_mmask8);
__cov_v16hi __builtin_ia32_blendmw_256_mask(__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_cvtmask2w256(__cov_mmask16);
__cov_v16hi __builtin_ia32_dbpsadbw256_mask(__cov_v32qi,__cov_v32qi,int,__cov_v16hi,__cov_mmask16);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v16hi __builtin_ia32_loaddquhi256_mask(const short*,__cov_v16hi,__cov_mmask16);
#else
__cov_v16hi __builtin_ia32_loaddquhi256_mask(__cov_v16hi const *,__cov_v16hi,__cov_mmask16);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v16hi __builtin_ia32_movdquhi256_mask(__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_pabsw256_mask(__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_packssdw256_mask(__cov_v8si,__cov_v8si,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_packusdw256_mask(__cov_v8si,__cov_v8si,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_paddsw256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_paddusw256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_paddw256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_pavgw256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_pbroadcastw256_gpr_mask(__cov_mmask16,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_pbroadcastw256_mask(__cov_v8hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_permvarhi256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_pmaddubsw256_mask(__cov_v32qi,__cov_v32qi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_pmaxsw256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_pmaxuw256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_pminsw256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_pminuw256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_pmovsxbw256_mask(__cov_v16qi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_pmovzxbw256_mask(__cov_v16qi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_pmulhrsw256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_pmulhuw256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_pmulhw256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_pmullw256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_psllv16hi_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_psllw256_mask(__cov_v16hi,__cov_v8hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_psllwi256_mask(__cov_v16hi,int,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_psrav16hi_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_psraw256_mask(__cov_v16hi,__cov_v8hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_psrawi256_mask(__cov_v16hi,int,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_psrlv16hi_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_psrlw256_mask(__cov_v16hi,__cov_v8hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_psrlwi256_mask(__cov_v16hi,int,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_psubsw256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_psubusw256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_psubw256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_punpckhwd256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_punpcklwd256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_vcvtps2ph512_mask(__cov_v16sf,int,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_vpcmov_v16hi256(__cov_v16hi,__cov_v16hi,__cov_v16hi);
__cov_v16hi __builtin_ia32_vpermi2varhi256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_vpermt2varhi256_mask(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16hi __builtin_ia32_vpermt2varhi256_maskz(__cov_v16hi,__cov_v16hi,__cov_v16hi,__cov_mmask16);
__cov_v16qi __builtin_ia32_blendmb_128_mask(__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_cvtmask2b128(__cov_mmask16);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v16qi __builtin_ia32_loaddquqi128_mask(const char*,__cov_v16qi,__cov_mmask16);
#else
__cov_v16qi __builtin_ia32_loaddquqi128_mask(__cov_v16qi const *,__cov_v16qi,__cov_mmask16);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v16qi __builtin_ia32_movdquqi128_mask(__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_pabsb128_mask(__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_packsswb128_mask(__cov_v8hi,__cov_v8hi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_packuswb128_mask(__cov_v8hi,__cov_v8hi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_paddb128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_paddsb128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_paddusb128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_pavgb128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_pbroadcastb128_gpr_mask(__cov_mmask8,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_pbroadcastb128_mask(__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_pmaxsb128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_pmaxub128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_pminsb128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_pminub128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_pmovdb128_mask(__cov_v4si,__cov_v16qi,__cov_mmask8);
__cov_v16qi __builtin_ia32_pmovdb256_mask(__cov_v8si,__cov_v16qi,__cov_mmask8);
__cov_v16qi __builtin_ia32_pmovqb128_mask(__cov_v2di,__cov_v16qi,__cov_mmask8);
__cov_v16qi __builtin_ia32_pmovqb256_mask(__cov_v4di,__cov_v16qi,__cov_mmask8);
__cov_v16qi __builtin_ia32_pmovsdb128_mask(__cov_v4si,__cov_v16qi,__cov_mmask8);
__cov_v16qi __builtin_ia32_pmovsdb256_mask(__cov_v8si,__cov_v16qi,__cov_mmask8);
__cov_v16qi __builtin_ia32_pmovsqb128_mask(__cov_v2di,__cov_v16qi,__cov_mmask8);
__cov_v16qi __builtin_ia32_pmovsqb256_mask(__cov_v4di,__cov_v16qi,__cov_mmask8);
__cov_v16qi __builtin_ia32_pmovswb128_mask(__cov_v8hi,__cov_v16qi,__cov_mmask8);
__cov_v16qi __builtin_ia32_pmovswb256_mask(__cov_v16hi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_pmovusdb128_mask(__cov_v4si,__cov_v16qi,__cov_mmask8);
__cov_v16qi __builtin_ia32_pmovusdb256_mask(__cov_v8si,__cov_v16qi,__cov_mmask8);
__cov_v16qi __builtin_ia32_pmovusqb128_mask(__cov_v2di,__cov_v16qi,__cov_mmask8);
__cov_v16qi __builtin_ia32_pmovusqb256_mask(__cov_v4di,__cov_v16qi,__cov_mmask8);
__cov_v16qi __builtin_ia32_pmovuswb128_mask(__cov_v8hi,__cov_v16qi,__cov_mmask8);
__cov_v16qi __builtin_ia32_pmovuswb256_mask(__cov_v16hi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_pmovwb128_mask(__cov_v8hi,__cov_v16qi,__cov_mmask8);
__cov_v16qi __builtin_ia32_pmovwb256_mask(__cov_v16hi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_pshufb128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_psubb128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_psubsb128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_psubusb128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_punpckhbw128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_punpcklbw128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_v16qi __builtin_ia32_vpcmov_v16qi(__cov_v16qi,__cov_v16qi,__cov_v16qi);
__cov_v16qi __builtin_ia32_vpcomneb(__cov_v16qi,__cov_v16qi);
__cov_v16qi __builtin_ia32_vpcomneub(__cov_v16qi,__cov_v16qi);
__cov_v16qi __builtin_ia32_vprotbi(__cov_v16qi,int);
__cov_v16sf __builtin_ia32_andnps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_v16sf __builtin_ia32_andps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_v16sf __builtin_ia32_broadcastf32x2_512_mask(__cov_v4sf,__cov_v16sf,__cov_mmask16);
__cov_v16sf __builtin_ia32_broadcastf32x8_512_mask(__cov_v8sf,__cov_v16sf,__cov_mmask16);
__cov_v16sf __builtin_ia32_copysignps512(__cov_v16sf,__cov_v16sf);
__cov_v16sf __builtin_ia32_copysignps512(__cov_v16sf,__cov_v16sf);
__cov_v16sf __builtin_ia32_exp2ps(__cov_v16sf);
__cov_v16sf __builtin_ia32_exp2ps_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_v16sf __builtin_ia32_exp2ps_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_v16sf __builtin_ia32_fixupimmps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16si,int,__cov_mmask16,int);
__cov_v16sf __builtin_ia32_fixupimmps512_maskz(__cov_v16sf,__cov_v16sf,__cov_v16si,int,__cov_mmask16,int);
__cov_v16sf __builtin_ia32_fixupimmps512_maskz(__cov_v16sf,__cov_v16sf,__cov_v16si,int,__cov_mmask16,int);
__cov_v16sf __builtin_ia32_getexpps512_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_v16sf __builtin_ia32_getmantps512_mask(__cov_v16sf,int,__cov_v16sf,__cov_mmask16,int);
__cov_v16sf __builtin_ia32_insertf32x8_mask(__cov_v16sf,__cov_v8sf,int,__cov_v16sf,__cov_mmask16);
__cov_v16sf __builtin_ia32_orps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_v16sf __builtin_ia32_ps512_256ps(__cov_v8sf);
__cov_v16sf __builtin_ia32_ps512_ps(__cov_v4sf);
__cov_v16sf __builtin_ia32_rangeps512_mask(__cov_v16sf,__cov_v16sf,int,__cov_v16sf,__cov_mmask16,int);
__cov_v16sf __builtin_ia32_rcp28ps_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_v16sf __builtin_ia32_reduceps512_mask(__cov_v16sf,int,__cov_v16sf,__cov_mmask16);
__cov_v16sf __builtin_ia32_rsqrt28ps_mask(__cov_v16sf,__cov_v16sf,__cov_mmask16,int);
__cov_v16sf __builtin_ia32_shufps512_mask(__cov_v16sf,__cov_v16sf,int,__cov_v16sf,__cov_mmask16);
__cov_v16sf __builtin_ia32_vpermilps512_mask(__cov_v16sf,int,__cov_v16sf,__cov_mmask16);
__cov_v16sf __builtin_ia32_xorps512_mask(__cov_v16sf,__cov_v16sf,__cov_v16sf,__cov_mmask16);
__cov_v16si __builtin_ia32_alignd512_mask(__cov_v16si,__cov_v16si,int,__cov_v16si,__cov_mmask16);
__cov_v16si __builtin_ia32_broadcasti32x2_512_mask(__cov_v4si,__cov_v16si,__cov_mmask16);
__cov_v16si __builtin_ia32_broadcasti32x8_512_mask(__cov_v8si,__cov_v16si,__cov_mmask16);
__cov_v16si __builtin_ia32_ceilpd_vec_pack_sfix512(__cov_v8df,__cov_v8df);
__cov_v16si __builtin_ia32_ceilpd_vec_pack_sfix512(__cov_v8df,__cov_v8df);
__cov_v16si __builtin_ia32_ceilpd_vec_pack_sfix512(__cov_v8df,__cov_v8df);
__cov_v16si __builtin_ia32_cvtmask2d512(__cov_mmask16);
__cov_v16si __builtin_ia32_floorpd_vec_pack_sfix512(__cov_v8df,__cov_v8df);
__cov_v16si __builtin_ia32_floorpd_vec_pack_sfix512(__cov_v8df,__cov_v8df);
__cov_v16si __builtin_ia32_floorpd_vec_pack_sfix512(__cov_v8df,__cov_v8df);
__cov_v16si __builtin_ia32_pmaddwd512_mask(__cov_v32hi,__cov_v32hi,__cov_v16si,__cov_mmask16);
__cov_v16si __builtin_ia32_prold512_mask(__cov_v16si,int,__cov_v16si,__cov_mmask16);
__cov_v16si __builtin_ia32_prord512_mask(__cov_v16si,int,__cov_v16si,__cov_mmask16);
__cov_v16si __builtin_ia32_pslldi512_mask(__cov_v16si,int,__cov_v16si,__cov_mmask16);
__cov_v16si __builtin_ia32_psradi512_mask(__cov_v16si,int,__cov_v16si,__cov_mmask16);
__cov_v16si __builtin_ia32_psrldi512_mask(__cov_v16si,int,__cov_v16si,__cov_mmask16);
__cov_v16si __builtin_ia32_pternlogd512_mask(__cov_v16si,__cov_v16si,__cov_v16si,int,__cov_mmask16);
__cov_v16si __builtin_ia32_pternlogd512_maskz(__cov_v16si,__cov_v16si,__cov_v16si,int,__cov_mmask16);
__cov_v16si __builtin_ia32_pternlogd512_maskz(__cov_v16si,__cov_v16si,__cov_v16si,int,__cov_mmask16);
__cov_v16si __builtin_ia32_roundpd_az_vec_pack_sfix512(__cov_v8df,__cov_v8df);
__cov_v16si __builtin_ia32_roundpd_az_vec_pack_sfix512(__cov_v8df,__cov_v8df);
__cov_v16si __builtin_ia32_roundpd_az_vec_pack_sfix512(__cov_v8df,__cov_v8df);
__cov_v16si __builtin_ia32_si512_256si(__cov_v8si);
__cov_v16si __builtin_ia32_si512_si(__cov_v4si);
__cov_v2df __builtin_ia32_addpd128_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_addsd_round(__cov_v2df,__cov_v2df,int);
__cov_v2df __builtin_ia32_andnpd128_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_andpd128_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_blendmpd_128_mask(__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_ceilpd(__cov_v2df);
__cov_v2df __builtin_ia32_compressdf128_mask(__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_copysignpd(__cov_v2df,__cov_v2df);
__cov_v2df __builtin_ia32_cvtdq2pd128_mask(__cov_v4si,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_cvtps2pd128_mask(__cov_v4sf,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_cvtqq2pd128_mask(__cov_v2di,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_cvtss2sd_round(__cov_v2df,__cov_v4sf,int);
__cov_v2df __builtin_ia32_cvtudq2pd128_mask(__cov_v4si,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_cvtuqq2pd128_mask(__cov_v2di,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_divpd_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_divsd_round(__cov_v2df,__cov_v2df,int);
__cov_v2df __builtin_ia32_expanddf128_mask(__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_expanddf128_maskz(__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_expandloaddf128_mask(__cov_v2df const *,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_expandloaddf128_maskz(__cov_v2df const *,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_extractf64x2_256_mask(__cov_v4df,int,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_extractf64x2_512_mask(__cov_v8df,int,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_fixupimmpd128_mask(__cov_v2df,__cov_v2df,__cov_v2di,int,__cov_mmask8);
__cov_v2df __builtin_ia32_fixupimmpd128_maskz(__cov_v2df,__cov_v2df,__cov_v2di,int,__cov_mmask8);
__cov_v2df __builtin_ia32_fixupimmsd_mask(__cov_v2df,__cov_v2df,__cov_v2di,int,__cov_mmask8,int);
__cov_v2df __builtin_ia32_fixupimmsd_maskz(__cov_v2df,__cov_v2df,__cov_v2di,int,__cov_mmask8,int);
__cov_v2df __builtin_ia32_fixupimmsd_maskz(__cov_v2df,__cov_v2df,__cov_v2di,int,__cov_mmask8,int);
__cov_v2df __builtin_ia32_floorpd(__cov_v2df);
__cov_v2df __builtin_ia32_getexppd128_mask(__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_getexpsd128_round(__cov_v2df,__cov_v2df,int);
__cov_v2df __builtin_ia32_getmantpd128_mask(__cov_v2df,int,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_getmantsd_round(__cov_v2df,__cov_v2df,int,int);
__cov_v2df __builtin_ia32_loadapd128_mask(__cov_v2df const *,__cov_v2df,__cov_mmask8);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v2df __builtin_ia32_loadupd128_mask(const double *,__cov_v2df,__cov_mmask8);
#else
__cov_v2df __builtin_ia32_loadupd128_mask(__cov_v2df const *,__cov_v2df,__cov_mmask8);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v2df __builtin_ia32_maxpd_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_maxsd_round(__cov_v2df,__cov_v2df,int);
__cov_v2df __builtin_ia32_minpd_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_minsd_round(__cov_v2df,__cov_v2df,int);
__cov_v2df __builtin_ia32_movapd128_mask(__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_movddup128_mask(__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_mulpd_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_mulsd_round(__cov_v2df,__cov_v2df,int);
__cov_v2df __builtin_ia32_orpd128_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_rangepd128_mask(__cov_v2df,__cov_v2df,int,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_rangesd128_round(__cov_v2df,__cov_v2df,int,int);
__cov_v2df __builtin_ia32_rcp14pd128_mask(__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_rcp28sd_round(__cov_v2df,__cov_v2df,int);
__cov_v2df __builtin_ia32_reducepd128_mask(__cov_v2df,int,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_reducesd(__cov_v2df,__cov_v2df,int);
__cov_v2df __builtin_ia32_rintpd(__cov_v2df);
__cov_v2df __builtin_ia32_rndscalepd_128_mask(__cov_v2df,int,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_rndscalesd_round(__cov_v2df,__cov_v2df,int,int);
__cov_v2df __builtin_ia32_roundpd_az(__cov_v2df);
__cov_v2df __builtin_ia32_rsqrt14pd128_mask(__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_rsqrt28sd_round(__cov_v2df,__cov_v2df,int);
__cov_v2df __builtin_ia32_scalefpd128_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_shufpd128_mask(__cov_v2df,__cov_v2df,int,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_sqrtpd128_mask(__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_sqrtsd_round(__cov_v2df,__cov_v2df,int);
__cov_v2df __builtin_ia32_subpd128_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_subsd_round(__cov_v2df,__cov_v2df,int);
__cov_v2df __builtin_ia32_truncpd(__cov_v2df);
__cov_v2df __builtin_ia32_unpckhpd128_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_unpcklpd128_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_vfmaddpd128_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_vfmaddpd128_mask3(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_vfmaddpd128_maskz(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_vfmaddsd3_round(__cov_v2df,__cov_v2df,__cov_v2df,int);
__cov_v2df __builtin_ia32_vfmaddsubpd128_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_vfmaddsubpd128_mask3(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_vfmaddsubpd128_maskz(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_vfmsubaddpd128_mask3(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_vfmsubpd128_mask3(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_vfnmaddpd128_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_vfnmsubpd128_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_vfnmsubpd128_mask3(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_vpcmov_v2df(__cov_v2df,__cov_v2df,__cov_v2df);
__cov_v2df __builtin_ia32_vpermi2varpd128_mask(__cov_v2df,__cov_v2di,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_vpermil2pd(__cov_v2df,__cov_v2df,__cov_v2di,int);
__cov_v2df __builtin_ia32_vpermilpd_mask(__cov_v2df,int,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_vpermilvarpd_mask(__cov_v2df,__cov_v2di,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_vpermt2varpd128_mask(__cov_v2di,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_vpermt2varpd128_maskz(__cov_v2di,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2df __builtin_ia32_xorpd128_mask(__cov_v2df,__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_v2di __builtin_ia32_alignq128_mask(__cov_v2di,__cov_v2di,int,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_blendmq_128_mask(__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_broadcastmb128(__cov_mmask8);
__cov_v2di __builtin_ia32_compressdi128_mask(__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_cvtmask2q128(__cov_mmask8);
__cov_v2di __builtin_ia32_cvtpd2qq128_mask(__cov_v2df,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_cvtpd2uqq128_mask(__cov_v2df,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_cvtps2qq128_mask(__cov_v4sf,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_cvtps2uqq128_mask(__cov_v4sf,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_cvttpd2qq128_mask(__cov_v2df,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_cvttpd2uqq128_mask(__cov_v2df,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_cvttps2qq128_mask(__cov_v4sf,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_cvttps2uqq128_mask(__cov_v4sf,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_expanddi128_mask(__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_expanddi128_maskz(__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_expandloaddi128_mask(__cov_v2di const *,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_expandloaddi128_maskz(__cov_v2di const *,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_extracti64x2_256_mask(__cov_v4di,int,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_extracti64x2_512_mask(__cov_v8di,int,__cov_v2di,__cov_mmask8);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v2di __builtin_ia32_loaddqudi128_mask(const long long *,__cov_v2di,__cov_mmask8);
#else
__cov_v2di __builtin_ia32_loaddqudi128_mask(__cov_v2di const *,__cov_v2di,__cov_mmask8);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v2di __builtin_ia32_movdqa64_128_mask(__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_movdqa64load128_mask(__cov_v2di const *,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pabsq128_mask(__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_paddq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_palignr128_mask(__cov_v2di,__cov_v2di,int,__cov_v2di,__cov_mmask16);
__cov_v2di __builtin_ia32_pandnq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pandq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pbroadcastq128_gpr_mask(unsigned long long,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pbroadcastq128_mask(__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pbroadcastq128_mem_mask(unsigned long long,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pmaxsq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pmaxuq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pminsq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pminuq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pmovsxbq128_mask(__cov_v16qi,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pmovsxdq128_mask(__cov_v4si,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pmovsxwq128_mask(__cov_v8hi,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pmovzxbq128_mask(__cov_v16qi,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pmovzxdq128_mask(__cov_v4si,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pmovzxwq128_mask(__cov_v8hi,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pmuldq128_mask(__cov_v4si,__cov_v4si,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pmullq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pmuludq128_mask(__cov_v4si,__cov_v4si,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_porq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_prolq128_mask(__cov_v2di,int,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_prolvq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_prorq128_mask(__cov_v2di,int,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_prorvq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_psllq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_psllqi128_mask(__cov_v2di,int,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_psllv2di_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_psraq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_psraqi128_mask(__cov_v2di,int,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_psravq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_psrlq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_psrlqi128_mask(__cov_v2di,int,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_psrlv2di_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_psubq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pternlogq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,int,__cov_mmask8);
__cov_v2di __builtin_ia32_pternlogq128_maskz(__cov_v2di,__cov_v2di,__cov_v2di,int,__cov_mmask8);
__cov_v2di __builtin_ia32_punpckhqdq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_punpcklqdq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_pxorq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_vpcmov_v2di(__cov_v2di,__cov_v2di,__cov_v2di);
__cov_v2di __builtin_ia32_vpcomneuq(__cov_v2di,__cov_v2di);
__cov_v2di __builtin_ia32_vpconflictdi_128_mask(__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_vpermi2varq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_vpermt2varq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_vpermt2varq128_maskz(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_vplzcntq_128_mask(__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_v2di __builtin_ia32_vprotqi(__cov_v2di,int);
__cov_v2si __builtin_ia32_pswapdsi(__cov_v2si);
__cov_v32hi __builtin_ia32_blendmw_512_mask(__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_cvtmask2w512(unsigned int);
__cov_v32hi __builtin_ia32_dbpsadbw512_mask(__cov_v64qi,__cov_v64qi,int,__cov_v32hi,unsigned int);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v32hi __builtin_ia32_loaddquhi512_mask(const short*,__cov_v32hi,unsigned int);
#else
__cov_v32hi __builtin_ia32_loaddquhi512_mask(__cov_v32hi const *,__cov_v32hi,unsigned int);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v32hi __builtin_ia32_movdquhi512_mask(__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pabsw512_mask(__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_packssdw512_mask(__cov_v16si,__cov_v16si,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_packusdw512_mask(__cov_v16si,__cov_v16si,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_paddsw512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_paddusw512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_paddw512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pavgw512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pbroadcastw512_gpr_mask(__cov_mmask16,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pbroadcastw512_mask(__cov_v8hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_permvarhi512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pmaddubsw512_mask(__cov_v64qi,__cov_v64qi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pmaxsw512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pmaxuw512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pminsw512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pminuw512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pmovsxbw512_mask(__cov_v32qi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pmovzxbw512_mask(__cov_v32qi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pmulhrsw512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pmulhuw512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pmulhw512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pmullw512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pshufhw512_mask(__cov_v32hi,int,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_pshuflw512_mask(__cov_v32hi,int,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_psllv32hi_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_psllw512_mask(__cov_v32hi,__cov_v8hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_psllwi512_mask(__cov_v32hi,int,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_psrav32hi_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_psraw512_mask(__cov_v32hi,__cov_v8hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_psrawi512_mask(__cov_v32hi,int,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_psrlv32hi_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_psrlw512_mask(__cov_v32hi,__cov_v8hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_psrlwi512_mask(__cov_v32hi,int,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_psubsw512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_psubusw512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_psubw512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_punpckhwd512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_punpcklwd512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_vpermi2varhi512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_vpermt2varhi512_mask(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32hi __builtin_ia32_vpermt2varhi512_maskz(__cov_v32hi,__cov_v32hi,__cov_v32hi,unsigned int);
__cov_v32qi __builtin_ia32_blendmb_256_mask(__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_cvtmask2b256(unsigned int);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v32qi __builtin_ia32_loaddquqi256_mask(const char*,__cov_v32qi,unsigned int);
#else
__cov_v32qi __builtin_ia32_loaddquqi256_mask(__cov_v32qi const *,__cov_v32qi,unsigned int);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v32qi __builtin_ia32_movdquqi256_mask(__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_pabsb256_mask(__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_packsswb256_mask(__cov_v16hi,__cov_v16hi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_packuswb256_mask(__cov_v16hi,__cov_v16hi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_paddb256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_paddsb256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_paddusb256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_pavgb256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_pbroadcastb256_gpr_mask(__cov_mmask8,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_pbroadcastb256_mask(__cov_v16qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_pmaxsb256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_pmaxub256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_pminsb256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_pminub256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_pmovswb512_mask(__cov_v32hi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_pmovuswb512_mask(__cov_v32hi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_pmovwb512_mask(__cov_v32hi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_pshufb256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_psubb256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_psubsb256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_psubusb256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_punpckhbw256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_punpcklbw256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,unsigned int);
__cov_v32qi __builtin_ia32_vpcmov_v32qi256(__cov_v32qi,__cov_v32qi,__cov_v32qi);
__cov_v4df __builtin_ia32_addpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_andnpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_andpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_blendmpd_256_mask(__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_broadcastf64x2_256_mask(__cov_v2df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_broadcastsd256_mask(__cov_v2df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_ceilpd256(__cov_v4df);
__cov_v4df __builtin_ia32_ceilpd256(__cov_v4df);
__cov_v4df __builtin_ia32_compressdf256_mask(__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_copysignpd256(__cov_v4df,__cov_v4df);
__cov_v4df __builtin_ia32_copysignpd256(__cov_v4df,__cov_v4df);
__cov_v4df __builtin_ia32_cvtdq2pd256_mask(__cov_v4si,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_cvtps2pd256_mask(__cov_v4sf,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_cvtqq2pd256_mask(__cov_v4di,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_cvtudq2pd256_mask(__cov_v4si,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_cvtuqq2pd256_mask(__cov_v4di,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_divpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_expanddf256_mask(__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_expanddf256_maskz(__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_expandloaddf256_mask(__cov_v4df const *,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_expandloaddf256_maskz(__cov_v4df const *,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_fixupimmpd256_mask(__cov_v4df,__cov_v4df,__cov_v4di,int,__cov_mmask8);
__cov_v4df __builtin_ia32_fixupimmpd256_maskz(__cov_v4df,__cov_v4df,__cov_v4di,int,__cov_mmask8);
__cov_v4df __builtin_ia32_floorpd256(__cov_v4df);
__cov_v4df __builtin_ia32_floorpd256(__cov_v4df);
__cov_v4df __builtin_ia32_getexppd256_mask(__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_getmantpd256_mask(__cov_v4df,int,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_insertf64x2_256_mask(__cov_v4df,__cov_v2df,int,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_loadapd256_mask(__cov_v4df const *,__cov_v4df,__cov_mmask8);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v4df __builtin_ia32_loadupd256_mask(const double *,__cov_v4df,__cov_mmask8);
#else
__cov_v4df __builtin_ia32_loadupd256_mask(__cov_v4df const *,__cov_v4df,__cov_mmask8);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v4df __builtin_ia32_maxpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_minpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_movapd256_mask(__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_movddup256_mask(__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_mulpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_orpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_permdf256_mask(__cov_v4df,int,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_permvardf256_mask(__cov_v4df,__cov_v4di,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_rangepd256_mask(__cov_v4df,__cov_v4df,int,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_rcp14pd256_mask(__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_reducepd256_mask(__cov_v4df,int,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_rintpd256(__cov_v4df);
__cov_v4df __builtin_ia32_rintpd256(__cov_v4df);
__cov_v4df __builtin_ia32_rndscalepd_256_mask(__cov_v4df,int,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_roundpd_az256(__cov_v4df);
__cov_v4df __builtin_ia32_roundpd_az256(__cov_v4df);
__cov_v4df __builtin_ia32_rsqrt14pd256_mask(__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_scalefpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_shuf_f64x2_256_mask(__cov_v4df,__cov_v4df,int,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_shufpd256_mask(__cov_v4df,__cov_v4df,int,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_sqrtpd256_mask(__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_subpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_truncpd256(__cov_v4df);
__cov_v4df __builtin_ia32_truncpd256(__cov_v4df);
__cov_v4df __builtin_ia32_unpckhpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_unpcklpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_vfmaddpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_vfmaddpd256_mask3(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_vfmaddpd256_maskz(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_vfmaddsubpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_vfmaddsubpd256_mask3(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_vfmaddsubpd256_maskz(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_vfmsubaddpd256_mask3(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_vfmsubpd256_mask3(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_vfnmaddpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_vfnmsubpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_vfnmsubpd256_mask3(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_vpcmov_v4df256(__cov_v4df,__cov_v4df,__cov_v4df);
__cov_v4df __builtin_ia32_vpermi2varpd256_mask(__cov_v4df,__cov_v4di,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_vpermil2pd256(__cov_v4df,__cov_v4df,__cov_v4di,int);
__cov_v4df __builtin_ia32_vpermil2pd256(__cov_v4df,__cov_v4df,__cov_v4di,int);
__cov_v4df __builtin_ia32_vpermilpd256_mask(__cov_v4df,int,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_vpermilvarpd256_mask(__cov_v4df,__cov_v4di,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_vpermt2varpd256_mask(__cov_v4di,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_vpermt2varpd256_maskz(__cov_v4di,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4df __builtin_ia32_xorpd256_mask(__cov_v4df,__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_v4di __builtin_ia32_alignq256_mask(__cov_v4di,__cov_v4di,int,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_blendmq_256_mask(__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_broadcasti64x2_256_mask(__cov_v2di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_broadcastmb256(__cov_mmask8);
__cov_v4di __builtin_ia32_compressdi256_mask(__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_cvtmask2q256(__cov_mmask8);
__cov_v4di __builtin_ia32_cvtpd2qq256_mask(__cov_v4df,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_cvtpd2uqq256_mask(__cov_v4df,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_cvtps2qq256_mask(__cov_v4sf,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_cvtps2uqq256_mask(__cov_v4sf,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_cvttpd2qq256_mask(__cov_v4df,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_cvttpd2uqq256_mask(__cov_v4df,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_cvttps2qq256_mask(__cov_v4sf,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_cvttps2uqq256_mask(__cov_v4sf,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_expanddi256_mask(__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_expanddi256_maskz(__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_expandloaddi256_mask(__cov_v4di const *,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_expandloaddi256_maskz(__cov_v4di const *,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_inserti64x2_256_mask(__cov_v4di,__cov_v2di,int,__cov_v4di,__cov_mmask8);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v4di __builtin_ia32_loaddqudi256_mask(const long long *,__cov_v4di,__cov_mmask8);
#else
__cov_v4di __builtin_ia32_loaddqudi256_mask(__cov_v4di const *,__cov_v4di,__cov_mmask8);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v4di __builtin_ia32_movdqa64_256_mask(__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_movdqa64load256_mask(__cov_v4di const *,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pabsq256_mask(__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_paddq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_palignr256_mask(__cov_v4di,__cov_v4di,int,__cov_v4di,unsigned int);
__cov_v4di __builtin_ia32_pandnq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pandq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pbroadcastq256_gpr_mask(unsigned long long,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pbroadcastq256_mask(__cov_v2di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pbroadcastq256_mem_mask(unsigned long long,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_permdi256_mask(__cov_v4di,int,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_permvardi256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pmaxsq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pmaxuq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pminsq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pminuq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pmovsxbq256_mask(__cov_v16qi,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pmovsxdq256_mask(__cov_v4si,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pmovsxwq256_mask(__cov_v8hi,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pmovzxbq256_mask(__cov_v16qi,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pmovzxdq256_mask(__cov_v4si,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pmovzxwq256_mask(__cov_v8hi,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pmuldq256_mask(__cov_v8si,__cov_v8si,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pmullq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pmuludq256_mask(__cov_v8si,__cov_v8si,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_porq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_prolq256_mask(__cov_v4di,int,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_prolvq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_prorq256_mask(__cov_v4di,int,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_prorvq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_psllq256_mask(__cov_v4di,__cov_v2di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_psllqi256_mask(__cov_v4di,int,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_psllv4di_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_psraq256_mask(__cov_v4di,__cov_v2di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_psraqi256_mask(__cov_v4di,int,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_psravq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_psrlq256_mask(__cov_v4di,__cov_v2di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_psrlqi256_mask(__cov_v4di,int,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_psrlv4di_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_psubq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pternlogq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,int,__cov_mmask8);
__cov_v4di __builtin_ia32_pternlogq256_maskz(__cov_v4di,__cov_v4di,__cov_v4di,int,__cov_mmask8);
__cov_v4di __builtin_ia32_punpckhqdq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_punpcklqdq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_pxorq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_shuf_i64x2_256_mask(__cov_v4di,__cov_v4di,int,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_vpcmov256(__cov_v4di,__cov_v4di,__cov_v4di);
__cov_v4di __builtin_ia32_vpcmov_v4di256(__cov_v4di,__cov_v4di,__cov_v4di);
__cov_v4di __builtin_ia32_vpconflictdi_256_mask(__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_vpermi2varq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_vpermt2varq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_vpermt2varq256_maskz(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4di __builtin_ia32_vplzcntq_256_mask(__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_v4sf __builtin_ia32_addps128_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_addss_round(__cov_v4sf,__cov_v4sf,int);
__cov_v4sf __builtin_ia32_andnps128_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_andps128_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_blendmps_128_mask(__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_broadcastss128_mask(__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_ceilps(__cov_v4sf);
__cov_v4sf __builtin_ia32_compresssf128_mask(__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_copysignps(__cov_v4sf,__cov_v4sf);
__cov_v4sf __builtin_ia32_cvtdq2ps128_mask(__cov_v4si,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_cvtpd2ps256_mask(__cov_v4df,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_cvtpd2ps_mask(__cov_v2df,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_cvtqq2ps128_mask(__cov_v2di,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_cvtqq2ps256_mask(__cov_v4di,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_cvtsd2ss_round(__cov_v4sf,__cov_v2df,int);
__cov_v4sf __builtin_ia32_cvtudq2ps128_mask(__cov_v4si,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_cvtuqq2ps128_mask(__cov_v2di,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_cvtuqq2ps256_mask(__cov_v4di,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_divps_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_divss_round(__cov_v4sf,__cov_v4sf,int);
__cov_v4sf __builtin_ia32_expandloadsf128_mask(__cov_v4sf const *,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_expandloadsf128_maskz(__cov_v4sf const *,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_expandsf128_mask(__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_expandsf128_maskz(__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_extractf32x4_256_mask(__cov_v8sf,int,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_fixupimmps128_mask(__cov_v4sf,__cov_v4sf,__cov_v4si,int,__cov_mmask8);
__cov_v4sf __builtin_ia32_fixupimmps128_maskz(__cov_v4sf,__cov_v4sf,__cov_v4si,int,__cov_mmask8);
__cov_v4sf __builtin_ia32_fixupimmss_mask(__cov_v4sf,__cov_v4sf,__cov_v4si,int,__cov_mmask8,int);
__cov_v4sf __builtin_ia32_fixupimmss_maskz(__cov_v4sf,__cov_v4sf,__cov_v4si,int,__cov_mmask8,int);
__cov_v4sf __builtin_ia32_fixupimmss_maskz(__cov_v4sf,__cov_v4sf,__cov_v4si,int,__cov_mmask8,int);
__cov_v4sf __builtin_ia32_floorps(__cov_v4sf);
__cov_v4sf __builtin_ia32_getexpps128_mask(__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_getexpss128_round(__cov_v4sf,__cov_v4sf,int);
__cov_v4sf __builtin_ia32_getmantps128_mask(__cov_v4sf,int,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_getmantss_round(__cov_v4sf,__cov_v4sf,int,int);
__cov_v4sf __builtin_ia32_loadaps128_mask(__cov_v4sf const *,__cov_v4sf,__cov_mmask8);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v4sf __builtin_ia32_loadups128_mask(const float *,__cov_v4sf,__cov_mmask8);
#else
__cov_v4sf __builtin_ia32_loadups128_mask(__cov_v4sf const *,__cov_v4sf,__cov_mmask8);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v4sf __builtin_ia32_maxps_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_maxss_round(__cov_v4sf,__cov_v4sf,int);
__cov_v4sf __builtin_ia32_minps_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_minss_round(__cov_v4sf,__cov_v4sf,int);
__cov_v4sf __builtin_ia32_movaps128_mask(__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_movshdup128_mask(__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_movsldup128_mask(__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_mulps_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_mulss_round(__cov_v4sf,__cov_v4sf,int);
__cov_v4sf __builtin_ia32_orps128_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_rangeps128_mask(__cov_v4sf,__cov_v4sf,int,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_rangess128_round(__cov_v4sf,__cov_v4sf,int,int);
__cov_v4sf __builtin_ia32_rcp14ps128_mask(__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_rcp28ss_round(__cov_v4sf,__cov_v4sf,int);
__cov_v4sf __builtin_ia32_reduceps128_mask(__cov_v4sf,int,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_reducess(__cov_v4sf,__cov_v4sf,int);
__cov_v4sf __builtin_ia32_rintps(__cov_v4sf);
__cov_v4sf __builtin_ia32_rndscaleps_128_mask(__cov_v4sf,int,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_rndscaless_round(__cov_v4sf,__cov_v4sf,int,int);
__cov_v4sf __builtin_ia32_roundps_az(__cov_v4sf);
__cov_v4sf __builtin_ia32_rsqrt14ps128_mask(__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_rsqrt28ss_round(__cov_v4sf,__cov_v4sf,int);
__cov_v4sf __builtin_ia32_rsqrtps_nr(__cov_v4sf);
__cov_v4sf __builtin_ia32_scalefps128_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_shufps128_mask(__cov_v4sf,__cov_v4sf,int,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_sqrtps128_mask(__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_sqrtps_nr(__cov_v4sf);
__cov_v4sf __builtin_ia32_sqrtss_round(__cov_v4sf,__cov_v4sf,int);
__cov_v4sf __builtin_ia32_subps128_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_subss_round(__cov_v4sf,__cov_v4sf,int);
__cov_v4sf __builtin_ia32_truncps(__cov_v4sf);
__cov_v4sf __builtin_ia32_unpckhps128_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_unpcklps128_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vcvtph2ps_mask(__cov_v8hi,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vfmaddps128_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vfmaddps128_mask3(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vfmaddps128_maskz(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vfmaddss3_round(__cov_v4sf,__cov_v4sf,__cov_v4sf,int);
__cov_v4sf __builtin_ia32_vfmaddsubps128_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vfmaddsubps128_mask3(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vfmaddsubps128_maskz(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vfmsubaddps128_mask3(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vfmsubps128_mask3(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vfnmaddps128_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vfnmsubps128_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vfnmsubps128_mask3(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vpcmov_v4sf(__cov_v4sf,__cov_v4sf,__cov_v4sf);
__cov_v4sf __builtin_ia32_vpermi2varps128_mask(__cov_v4sf,__cov_v4si,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vpermil2ps(__cov_v4sf,__cov_v4sf,__cov_v4si,int);
__cov_v4sf __builtin_ia32_vpermilps_mask(__cov_v4sf,int,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vpermilvarps_mask(__cov_v4sf,__cov_v4si,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vpermt2varps128_mask(__cov_v4si,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_vpermt2varps128_maskz(__cov_v4si,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4sf __builtin_ia32_xorps128_mask(__cov_v4sf,__cov_v4sf,__cov_v4sf,__cov_mmask8);
__cov_v4si __builtin_ia32_alignd128_mask(__cov_v4si,__cov_v4si,int,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_blendmd_128_mask(__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_broadcasti32x2_128_mask(__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_broadcastmw128(__cov_mmask16);
__cov_v4si __builtin_ia32_ceilpd_vec_pack_sfix(__cov_v2df,__cov_v2df);
__cov_v4si __builtin_ia32_ceilpd_vec_pack_sfix(__cov_v2df,__cov_v2df);
__cov_v4si __builtin_ia32_ceilps_sfix(__cov_v4sf);
__cov_v4si __builtin_ia32_ceilps_sfix(__cov_v4sf);
__cov_v4si __builtin_ia32_compresssi128_mask(__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_cvtmask2d128(__cov_mmask8);
__cov_v4si __builtin_ia32_cvtpd2dq128_mask(__cov_v2df,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_cvtpd2dq256_mask(__cov_v4df,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_cvtpd2udq128_mask(__cov_v2df,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_cvtpd2udq256_mask(__cov_v4df,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_cvtps2dq128_mask(__cov_v4sf,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_cvtps2udq128_mask(__cov_v4sf,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_cvttpd2dq128_mask(__cov_v2df,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_cvttpd2dq256_mask(__cov_v4df,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_cvttpd2udq128_mask(__cov_v2df,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_cvttpd2udq256_mask(__cov_v4df,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_cvttps2dq128_mask(__cov_v4sf,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_cvttps2udq128_mask(__cov_v4sf,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_expandloadsi128_mask(__cov_v4si const *,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_expandloadsi128_maskz(__cov_v4si const *,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_expandsi128_mask(__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_expandsi128_maskz(__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_extracti32x4_256_mask(__cov_v8si,int,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_floorpd_vec_pack_sfix(__cov_v2df,__cov_v2df);
__cov_v4si __builtin_ia32_floorpd_vec_pack_sfix(__cov_v2df,__cov_v2df);
__cov_v4si __builtin_ia32_floorps_sfix(__cov_v4sf);
__cov_v4si __builtin_ia32_floorps_sfix(__cov_v4sf);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v4si __builtin_ia32_loaddqusi128_mask(const int*,__cov_v4si,__cov_mmask8);
#else
__cov_v4si __builtin_ia32_loaddqusi128_mask(__cov_v4si const *,__cov_v4si,__cov_mmask8);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v4si __builtin_ia32_movdqa32_128_mask(__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_movdqa32load128_mask(__cov_v4si const *,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pabsd128_mask(__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_paddd128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pandd128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pandnd128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pbroadcastd128_gpr_mask(unsigned int,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pbroadcastd128_mask(__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pmaddwd128_mask(__cov_v8hi,__cov_v8hi,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pmaxsd128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pmaxud128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pminsd128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pminud128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pmovqd128_mask(__cov_v2di,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pmovqd256_mask(__cov_v4di,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pmovsqd128_mask(__cov_v2di,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pmovsqd256_mask(__cov_v4di,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pmovsxbd128_mask(__cov_v16qi,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pmovsxwd128_mask(__cov_v8hi,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pmovusqd128_mask(__cov_v2di,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pmovusqd256_mask(__cov_v4di,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pmovzxbd128_mask(__cov_v16qi,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pmovzxwd128_mask(__cov_v8hi,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pmulld128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pord128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_prold128_mask(__cov_v4si,int,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_prolvd128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_prord128_mask(__cov_v4si,int,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_prorvd128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pshufd128_mask(__cov_v4si,int,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pslld128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pslldi128_mask(__cov_v4si,int,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_psllv4si_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_psrad128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_psradi128_mask(__cov_v4si,int,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_psrav4si_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_psrld128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_psrldi128_mask(__cov_v4si,int,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_psrlv4si_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_psubd128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pternlogd128_mask(__cov_v4si,__cov_v4si,__cov_v4si,int,__cov_mmask8);
__cov_v4si __builtin_ia32_pternlogd128_maskz(__cov_v4si,__cov_v4si,__cov_v4si,int,__cov_mmask8);
__cov_v4si __builtin_ia32_punpckhdq128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_punpckldq128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_pxord128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_roundpd_az_vec_pack_sfix(__cov_v2df,__cov_v2df);
__cov_v4si __builtin_ia32_roundpd_az_vec_pack_sfix(__cov_v2df,__cov_v2df);
__cov_v4si __builtin_ia32_roundps_az_sfix(__cov_v4sf);
__cov_v4si __builtin_ia32_roundps_az_sfix(__cov_v4sf);
__cov_v4si __builtin_ia32_vec_pack_sfix(__cov_v2df,__cov_v2df);
__cov_v4si __builtin_ia32_vpcmov_v4si(__cov_v4si,__cov_v4si,__cov_v4si);
__cov_v4si __builtin_ia32_vpcomned(__cov_v4si,__cov_v4si);
__cov_v4si __builtin_ia32_vpcomneud(__cov_v4si,__cov_v4si);
__cov_v4si __builtin_ia32_vpconflictsi_128_mask(__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_vpermi2vard128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_vpermt2vard128_mask(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_vpermt2vard128_maskz(__cov_v4si,__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_vplzcntd_128_mask(__cov_v4si,__cov_v4si,__cov_mmask8);
__cov_v4si __builtin_ia32_vprotdi(__cov_v4si,int);
__cov_v64qi __builtin_ia32_blendmb_512_mask(__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_cvtmask2b512(unsigned long long);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v64qi __builtin_ia32_loaddquqi512_mask(const char*,__cov_v64qi,unsigned long long);
#else
__cov_v64qi __builtin_ia32_loaddquqi512_mask(__cov_v64qi const *,__cov_v64qi,unsigned long long);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v64qi __builtin_ia32_movdquqi512_mask(__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_pabsb512_mask(__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_packsswb512_mask(__cov_v32hi,__cov_v32hi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_packuswb512_mask(__cov_v32hi,__cov_v32hi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_paddb512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_paddsb512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_paddusb512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_pavgb512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_pbroadcastb512_gpr_mask(__cov_mmask8,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_pbroadcastb512_mask(__cov_v16qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_pmaxsb512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_pmaxub512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_pminsb512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_pminub512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_pshufb512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_psubb512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_psubsb512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_psubusb512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_punpckhbw512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v64qi __builtin_ia32_punpcklbw512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,unsigned long long);
__cov_v8df __builtin_ia32_andnpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8);
__cov_v8df __builtin_ia32_andpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8);
__cov_v8df __builtin_ia32_broadcastf64x2_512_mask(__cov_v2df,__cov_v8df,__cov_mmask8);
__cov_v8df __builtin_ia32_copysignpd512(__cov_v8df,__cov_v8df);
__cov_v8df __builtin_ia32_copysignpd512(__cov_v8df,__cov_v8df);
__cov_v8df __builtin_ia32_cvtqq2pd512_mask(__cov_v8di,__cov_v8df,__cov_mmask8,int);
__cov_v8df __builtin_ia32_cvtuqq2pd512_mask(__cov_v8di,__cov_v8df,__cov_mmask8,int);
__cov_v8df __builtin_ia32_exp2pd_mask(__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_v8df __builtin_ia32_fixupimmpd512_mask(__cov_v8df,__cov_v8df,__cov_v8di,int,__cov_mmask8,int);
__cov_v8df __builtin_ia32_fixupimmpd512_maskz(__cov_v8df,__cov_v8df,__cov_v8di,int,__cov_mmask8,int);
__cov_v8df __builtin_ia32_fixupimmpd512_maskz(__cov_v8df,__cov_v8df,__cov_v8di,int,__cov_mmask8,int);
__cov_v8df __builtin_ia32_getexppd512_mask(__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_v8df __builtin_ia32_getmantpd512_mask(__cov_v8df,int,__cov_v8df,__cov_mmask8,int);
__cov_v8df __builtin_ia32_insertf64x2_512_mask(__cov_v8df,__cov_v2df,int,__cov_v8df,__cov_mmask8);
__cov_v8df __builtin_ia32_orpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8);
__cov_v8df __builtin_ia32_pd512_256pd(__cov_v4df);
__cov_v8df __builtin_ia32_pd512_pd(__cov_v2df);
__cov_v8df __builtin_ia32_permdf512_mask(__cov_v8df,int,__cov_v8df,__cov_mmask8);
__cov_v8df __builtin_ia32_rangepd512_mask(__cov_v8df,__cov_v8df,int,__cov_v8df,__cov_mmask8,int);
__cov_v8df __builtin_ia32_rcp28pd_mask(__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_v8df __builtin_ia32_reducepd512_mask(__cov_v8df,int,__cov_v8df,__cov_mmask8);
__cov_v8df __builtin_ia32_rsqrt28pd_mask(__cov_v8df,__cov_v8df,__cov_mmask8,int);
__cov_v8df __builtin_ia32_shufpd512_mask(__cov_v8df,__cov_v8df,int,__cov_v8df,__cov_mmask8);
__cov_v8df __builtin_ia32_vpermilpd512_mask(__cov_v8df,int,__cov_v8df,__cov_mmask8);
__cov_v8df __builtin_ia32_xorpd512_mask(__cov_v8df,__cov_v8df,__cov_v8df,__cov_mmask8);
__cov_v8di __builtin_ia32_alignq512_mask(__cov_v8di,__cov_v8di,int,__cov_v8di,__cov_mmask8);
__cov_v8di __builtin_ia32_broadcasti64x2_512_mask(__cov_v2di,__cov_v8di,__cov_mmask8);
__cov_v8di __builtin_ia32_cvtmask2q512(__cov_mmask8);
__cov_v8di __builtin_ia32_cvtpd2qq512_mask(__cov_v8df,__cov_v8di,__cov_mmask8,int);
__cov_v8di __builtin_ia32_cvtpd2uqq512_mask(__cov_v8df,__cov_v8di,__cov_mmask8,int);
__cov_v8di __builtin_ia32_cvtps2qq512_mask(__cov_v8sf,__cov_v8di,__cov_mmask8,int);
__cov_v8di __builtin_ia32_cvtps2uqq512_mask(__cov_v8sf,__cov_v8di,__cov_mmask8,int);
__cov_v8di __builtin_ia32_cvttpd2qq512_mask(__cov_v8df,__cov_v8di,__cov_mmask8,int);
__cov_v8di __builtin_ia32_cvttpd2uqq512_mask(__cov_v8df,__cov_v8di,__cov_mmask8,int);
__cov_v8di __builtin_ia32_cvttps2qq512_mask(__cov_v8sf,__cov_v8di,__cov_mmask8,int);
__cov_v8di __builtin_ia32_cvttps2uqq512_mask(__cov_v8sf,__cov_v8di,__cov_mmask8,int);
__cov_v8di __builtin_ia32_inserti64x2_512_mask(__cov_v8di,__cov_v2di,int,__cov_v8di,__cov_mmask8);
__cov_v8di __builtin_ia32_palignr512(__cov_v8di,__cov_v8di,int);
__cov_v8di __builtin_ia32_palignr512_mask(__cov_v8di,__cov_v8di,int,__cov_v8di,unsigned long long);
__cov_v8di __builtin_ia32_permdi512_mask(__cov_v8di,int,__cov_v8di,__cov_mmask8);
__cov_v8di __builtin_ia32_pmullq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_v8di __builtin_ia32_prolq512_mask(__cov_v8di,int,__cov_v8di,__cov_mmask8);
__cov_v8di __builtin_ia32_prorq512_mask(__cov_v8di,int,__cov_v8di,__cov_mmask8);
__cov_v8di __builtin_ia32_psadbw512(__cov_v64qi,__cov_v64qi);
__cov_v8di __builtin_ia32_pslldq512(__cov_v8di,int);
__cov_v8di __builtin_ia32_psllqi512_mask(__cov_v8di,int,__cov_v8di,__cov_mmask8);
__cov_v8di __builtin_ia32_psraqi512_mask(__cov_v8di,int,__cov_v8di,__cov_mmask8);
__cov_v8di __builtin_ia32_psrldq512(__cov_v8di,int);
__cov_v8di __builtin_ia32_psrlqi512_mask(__cov_v8di,int,__cov_v8di,__cov_mmask8);
__cov_v8di __builtin_ia32_pternlogq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,int,__cov_mmask8);
__cov_v8di __builtin_ia32_pternlogq512_maskz(__cov_v8di,__cov_v8di,__cov_v8di,int,__cov_mmask8);
__cov_v8di __builtin_ia32_pternlogq512_maskz(__cov_v8di,__cov_v8di,__cov_v8di,int,__cov_mmask8);
__cov_v8hi __builtin_ia32_blendmw_128_mask(__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_cvtmask2w128(__cov_mmask8);
__cov_v8hi __builtin_ia32_dbpsadbw128_mask(__cov_v16qi,__cov_v16qi,int,__cov_v8hi,__cov_mmask8);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v8hi __builtin_ia32_loaddquhi128_mask(const short*,__cov_v8hi,__cov_mmask8);
#else
__cov_v8hi __builtin_ia32_loaddquhi128_mask(__cov_v8hi const *,__cov_v8hi,__cov_mmask8);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v8hi __builtin_ia32_movdquhi128_mask(__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pabsw128_mask(__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_packssdw128_mask(__cov_v4si,__cov_v4si,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_packusdw128_mask(__cov_v4si,__cov_v4si,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_paddsw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_paddusw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_paddw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pavgw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pbroadcastw128_gpr_mask(__cov_mmask16,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pbroadcastw128_mask(__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_permvarhi128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmaddubsw128_mask(__cov_v16qi,__cov_v16qi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmaxsw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmaxuw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pminsw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pminuw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmovdw128_mask(__cov_v4si,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmovdw256_mask(__cov_v8si,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmovqw128_mask(__cov_v2di,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmovqw256_mask(__cov_v4di,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmovsdw128_mask(__cov_v4si,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmovsdw256_mask(__cov_v8si,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmovsqw128_mask(__cov_v2di,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmovsqw256_mask(__cov_v4di,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmovsxbw128_mask(__cov_v16qi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmovusdw128_mask(__cov_v4si,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmovusdw256_mask(__cov_v8si,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmovusqw128_mask(__cov_v2di,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmovusqw256_mask(__cov_v4di,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmovzxbw128_mask(__cov_v16qi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmulhrsw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmulhuw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmulhw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pmullw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pshufhw128_mask(__cov_v8hi,int,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_pshuflw128_mask(__cov_v8hi,int,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_psllv8hi_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_psllw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_psllwi128_mask(__cov_v8hi,int,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_psrav8hi_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_psraw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_psrawi128_mask(__cov_v8hi,int,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_psrlv8hi_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_psrlw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_psrlwi128_mask(__cov_v8hi,int,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_psubsw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_psubusw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_psubw128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_punpckhwd128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_punpcklwd128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_vcvtps2ph256_mask(__cov_v8sf,int,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_vcvtps2ph_mask(__cov_v4sf,int,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_vpcmov_v8hi(__cov_v8hi,__cov_v8hi,__cov_v8hi);
__cov_v8hi __builtin_ia32_vpcomneuw(__cov_v8hi,__cov_v8hi);
__cov_v8hi __builtin_ia32_vpcomnew(__cov_v8hi,__cov_v8hi);
__cov_v8hi __builtin_ia32_vpermi2varhi128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_vpermt2varhi128_mask(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_vpermt2varhi128_maskz(__cov_v8hi,__cov_v8hi,__cov_v8hi,__cov_mmask8);
__cov_v8hi __builtin_ia32_vprotwi(__cov_v8hi,int);
__cov_v8sf __builtin_ia32_addps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_andnps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_andps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_blendmps_256_mask(__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_broadcastf32x2_256_mask(__cov_v4sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_broadcastf32x4_256_mask(__cov_v4sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_broadcastss256_mask(__cov_v4sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_ceilps256(__cov_v8sf);
__cov_v8sf __builtin_ia32_ceilps256(__cov_v8sf);
__cov_v8sf __builtin_ia32_compresssf256_mask(__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_copysignps256(__cov_v8sf,__cov_v8sf);
__cov_v8sf __builtin_ia32_copysignps256(__cov_v8sf,__cov_v8sf);
__cov_v8sf __builtin_ia32_cvtdq2ps256_mask(__cov_v8si,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_cvtqq2ps512_mask(__cov_v8di,__cov_v8sf,__cov_mmask8,int);
__cov_v8sf __builtin_ia32_cvtudq2ps256_mask(__cov_v8si,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_cvtuqq2ps512_mask(__cov_v8di,__cov_v8sf,__cov_mmask8,int);
__cov_v8sf __builtin_ia32_divps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_expandloadsf256_mask(__cov_v8sf const *,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_expandloadsf256_maskz(__cov_v8sf const *,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_expandsf256_mask(__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_expandsf256_maskz(__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_extractf32x8_mask(__cov_v16sf,int,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_fixupimmps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8si,int,__cov_mmask8);
__cov_v8sf __builtin_ia32_fixupimmps256_maskz(__cov_v8sf,__cov_v8sf,__cov_v8si,int,__cov_mmask8);
__cov_v8sf __builtin_ia32_floorps256(__cov_v8sf);
__cov_v8sf __builtin_ia32_floorps256(__cov_v8sf);
__cov_v8sf __builtin_ia32_getexpps256_mask(__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_getmantps256_mask(__cov_v8sf,int,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_insertf32x4_256_mask(__cov_v8sf,__cov_v4sf,int,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_loadaps256_mask(__cov_v8sf const *,__cov_v8sf,__cov_mmask8);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v8sf __builtin_ia32_loadups256_mask(const float *,__cov_v8sf,__cov_mmask8);
#else
__cov_v8sf __builtin_ia32_loadups256_mask(__cov_v8sf const *,__cov_v8sf,__cov_mmask8);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v8sf __builtin_ia32_maxps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_minps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_movaps256_mask(__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_movshdup256_mask(__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_movsldup256_mask(__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_mulps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_orps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_permvarsf256_mask(__cov_v8sf,__cov_v8si,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_rangeps256_mask(__cov_v8sf,__cov_v8sf,int,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_rcp14ps256_mask(__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_reduceps256_mask(__cov_v8sf,int,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_rintps256(__cov_v8sf);
__cov_v8sf __builtin_ia32_rintps256(__cov_v8sf);
__cov_v8sf __builtin_ia32_rndscaleps_256_mask(__cov_v8sf,int,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_roundps_az256(__cov_v8sf);
__cov_v8sf __builtin_ia32_roundps_az256(__cov_v8sf);
__cov_v8sf __builtin_ia32_rsqrt14ps256_mask(__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_rsqrtps_nr256(__cov_v8sf);
__cov_v8sf __builtin_ia32_rsqrtps_nr256(__cov_v8sf);
__cov_v8sf __builtin_ia32_scalefps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_shuf_f32x4_256_mask(__cov_v8sf,__cov_v8sf,int,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_shufps256_mask(__cov_v8sf,__cov_v8sf,int,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_sqrtps256_mask(__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_sqrtps_nr256(__cov_v8sf);
__cov_v8sf __builtin_ia32_sqrtps_nr256(__cov_v8sf);
__cov_v8sf __builtin_ia32_subps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_truncps256(__cov_v8sf);
__cov_v8sf __builtin_ia32_truncps256(__cov_v8sf);
__cov_v8sf __builtin_ia32_unpckhps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_unpcklps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vcvtph2ps256_mask(__cov_v8hi,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vfmaddps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vfmaddps256_mask3(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vfmaddps256_maskz(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vfmaddsubps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vfmaddsubps256_mask3(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vfmaddsubps256_maskz(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vfmsubaddps256_mask3(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vfmsubps256_mask3(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vfnmaddps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vfnmsubps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vfnmsubps256_mask3(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vpcmov_v8sf256(__cov_v8sf,__cov_v8sf,__cov_v8sf);
__cov_v8sf __builtin_ia32_vpermi2varps256_mask(__cov_v8sf,__cov_v8si,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vpermil2ps256(__cov_v8sf,__cov_v8sf,__cov_v8si,int);
__cov_v8sf __builtin_ia32_vpermil2ps256(__cov_v8sf,__cov_v8sf,__cov_v8si,int);
__cov_v8sf __builtin_ia32_vpermilps256_mask(__cov_v8sf,int,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vpermilvarps256_mask(__cov_v8sf,__cov_v8si,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vpermt2varps256_mask(__cov_v8si,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_vpermt2varps256_maskz(__cov_v8si,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8sf __builtin_ia32_xorps256_mask(__cov_v8sf,__cov_v8sf,__cov_v8sf,__cov_mmask8);
__cov_v8si __builtin_ia32_alignd256_mask(__cov_v8si,__cov_v8si,int,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_blendmd_256_mask(__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_broadcasti32x2_256_mask(__cov_v4si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_broadcasti32x4_256_mask(__cov_v4si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_broadcastmw256(__cov_mmask16);
__cov_v8si __builtin_ia32_ceilpd_vec_pack_sfix256(__cov_v4df,__cov_v4df);
__cov_v8si __builtin_ia32_ceilpd_vec_pack_sfix256(__cov_v4df,__cov_v4df);
__cov_v8si __builtin_ia32_ceilpd_vec_pack_sfix256(__cov_v4df,__cov_v4df);
__cov_v8si __builtin_ia32_ceilps_sfix256(__cov_v8sf);
__cov_v8si __builtin_ia32_ceilps_sfix256(__cov_v8sf);
__cov_v8si __builtin_ia32_ceilps_sfix256(__cov_v8sf);
__cov_v8si __builtin_ia32_compresssi256_mask(__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_cvtmask2d256(__cov_mmask8);
__cov_v8si __builtin_ia32_cvtps2dq256_mask(__cov_v8sf,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_cvtps2udq256_mask(__cov_v8sf,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_cvttps2dq256_mask(__cov_v8sf,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_cvttps2udq256_mask(__cov_v8sf,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_expandloadsi256_mask(__cov_v8si const *,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_expandloadsi256_maskz(__cov_v8si const *,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_expandsi256_mask(__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_expandsi256_maskz(__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_extracti32x8_mask(__cov_v16si,int,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_floorpd_vec_pack_sfix256(__cov_v4df,__cov_v4df);
__cov_v8si __builtin_ia32_floorpd_vec_pack_sfix256(__cov_v4df,__cov_v4df);
__cov_v8si __builtin_ia32_floorpd_vec_pack_sfix256(__cov_v4df,__cov_v4df);
__cov_v8si __builtin_ia32_floorps_sfix256(__cov_v8sf);
__cov_v8si __builtin_ia32_floorps_sfix256(__cov_v8sf);
__cov_v8si __builtin_ia32_floorps_sfix256(__cov_v8sf);
__cov_v8si __builtin_ia32_inserti32x4_256_mask(__cov_v8si,__cov_v4si,int,__cov_v8si,__cov_mmask8);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v8si __builtin_ia32_loaddqusi256_mask(const int*,__cov_v8si,__cov_mmask8);
#else
__cov_v8si __builtin_ia32_loaddqusi256_mask(__cov_v8si const *,__cov_v8si,__cov_mmask8);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_v8si __builtin_ia32_movdqa32_256_mask(__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_movdqa32load256_mask(__cov_v8si const *,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pabsd256_mask(__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_paddd256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pandd256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pandnd256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pbroadcastd256_gpr_mask(unsigned int,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pbroadcastd256_mask(__cov_v4si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_permvarsi256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pmaddwd256_mask(__cov_v16hi,__cov_v16hi,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pmaxsd256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pmaxud256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pminsd256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pminud256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pmovsxbd256_mask(__cov_v16qi,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pmovsxwd256_mask(__cov_v8hi,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pmovzxbd256_mask(__cov_v16qi,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pmovzxwd256_mask(__cov_v8hi,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pmulld256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pord256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_prold256_mask(__cov_v8si,int,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_prolvd256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_prord256_mask(__cov_v8si,int,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_prorvd256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pshufd256_mask(__cov_v8si,int,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pslld256_mask(__cov_v8si,__cov_v4si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pslldi256_mask(__cov_v8si,int,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_psllv8si_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_psrad256_mask(__cov_v8si,__cov_v4si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_psradi256_mask(__cov_v8si,int,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_psrav8si_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_psrld256_mask(__cov_v8si,__cov_v4si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_psrldi256_mask(__cov_v8si,int,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_psrlv8si_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_psubd256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pternlogd256_mask(__cov_v8si,__cov_v8si,__cov_v8si,int,__cov_mmask8);
__cov_v8si __builtin_ia32_pternlogd256_maskz(__cov_v8si,__cov_v8si,__cov_v8si,int,__cov_mmask8);
__cov_v8si __builtin_ia32_punpckhdq256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_punpckldq256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_pxord256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_roundpd_az_vec_pack_sfix256(__cov_v4df,__cov_v4df);
__cov_v8si __builtin_ia32_roundpd_az_vec_pack_sfix256(__cov_v4df,__cov_v4df);
__cov_v8si __builtin_ia32_roundpd_az_vec_pack_sfix256(__cov_v4df,__cov_v4df);
__cov_v8si __builtin_ia32_roundps_az_sfix256(__cov_v8sf);
__cov_v8si __builtin_ia32_roundps_az_sfix256(__cov_v8sf);
__cov_v8si __builtin_ia32_roundps_az_sfix256(__cov_v8sf);
__cov_v8si __builtin_ia32_shuf_i32x4_256_mask(__cov_v8si,__cov_v8si,int,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_vec_pack_sfix256(__cov_v4df,__cov_v4df);
__cov_v8si __builtin_ia32_vec_pack_sfix256(__cov_v4df,__cov_v4df);
__cov_v8si __builtin_ia32_vpcmov_v8si256(__cov_v8si,__cov_v8si,__cov_v8si);
__cov_v8si __builtin_ia32_vpconflictsi_256_mask(__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_vpermi2vard256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_vpermt2vard256_mask(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_vpermt2vard256_maskz(__cov_v8si,__cov_v8si,__cov_v8si,__cov_mmask8);
__cov_v8si __builtin_ia32_vplzcntd_256_mask(__cov_v8si,__cov_v8si,__cov_mmask8);
float __builtin_ia32_rsqrtf(float);
int __builtin_ia32_vcvtsd2si32(__cov_v2df,int);
int __builtin_ia32_vcvtss2si32(__cov_v4sf,int);
long long __builtin_ia32_vcvtsd2si64(__cov_v2df,int);
long long __builtin_ia32_vcvtss2si64(__cov_v4sf,int);
unsigned char __builtin_ia32_sbb_u32(unsigned char, unsigned int, unsigned int, unsigned*);
unsigned char __builtin_ia32_sbb_u64(unsigned char, unsigned long long, unsigned long long, unsigned long long*);
unsigned int __builtin_ia32_cmpb256_mask(__cov_v32qi,__cov_v32qi,int,unsigned int);
unsigned int __builtin_ia32_cmpw512_mask(__cov_v32hi,__cov_v32hi,int,unsigned int);
unsigned int __builtin_ia32_cvtb2mask256(__cov_v32qi);
unsigned int __builtin_ia32_cvtw2mask512(__cov_v32hi);
unsigned int __builtin_ia32_kunpcksi(unsigned int,unsigned int);
unsigned int __builtin_ia32_pcmpeqb256_mask(__cov_v32qi,__cov_v32qi,unsigned int);
unsigned int __builtin_ia32_pcmpeqw512_mask(__cov_v32hi,__cov_v32hi,unsigned int);
unsigned int __builtin_ia32_pcmpgtb256_mask(__cov_v32qi,__cov_v32qi,unsigned int);
unsigned int __builtin_ia32_pcmpgtw512_mask(__cov_v32hi,__cov_v32hi,unsigned int);
unsigned int __builtin_ia32_ptestmb256(__cov_v32qi,__cov_v32qi,unsigned int);
unsigned int __builtin_ia32_ptestmw512(__cov_v32hi,__cov_v32hi,unsigned int);
unsigned int __builtin_ia32_ptestnmb256(__cov_v32qi,__cov_v32qi,unsigned int);
unsigned int __builtin_ia32_ptestnmw512(__cov_v32hi,__cov_v32hi,unsigned int);
unsigned int __builtin_ia32_ucmpb256_mask(__cov_v32qi,__cov_v32qi,int,unsigned int);
unsigned int __builtin_ia32_ucmpw512_mask(__cov_v32hi,__cov_v32hi,int,unsigned int);
unsigned long __builtin_ia32_sizeof(void);
unsigned long long __builtin_ia32_cmpb512_mask(__cov_v64qi,__cov_v64qi,int,unsigned long long);
unsigned long long __builtin_ia32_cvtb2mask512(__cov_v64qi);
unsigned long long __builtin_ia32_kunpckdi(unsigned long long,unsigned long long);
unsigned long long __builtin_ia32_pcmpeqb512_mask(__cov_v64qi,__cov_v64qi,unsigned long long);
unsigned long long __builtin_ia32_pcmpgtb512_mask(__cov_v64qi,__cov_v64qi,unsigned long long);
unsigned long long __builtin_ia32_ptestmb512(__cov_v64qi,__cov_v64qi,unsigned long long);
unsigned long long __builtin_ia32_ptestnmb512(__cov_v64qi,__cov_v64qi,unsigned long long);
unsigned long long __builtin_ia32_ucmpb512_mask(__cov_v64qi,__cov_v64qi,int,unsigned long long);
void __builtin_ia32_bndcl(void const *,__cov_bounds_type);
void __builtin_ia32_bndcu(void const *,__cov_bounds_type);
void __builtin_ia32_bndstx(void const *,__cov_bounds_type,void const *);
void __builtin_ia32_clflushopt(const void*);
void __builtin_ia32_compressstoredf128_mask(__cov_v2df*,__cov_v2df,__cov_mmask8);
void __builtin_ia32_compressstoredf256_mask(__cov_v4df*,__cov_v4df,__cov_mmask8);
void __builtin_ia32_compressstoredi128_mask(__cov_v2di*,__cov_v2di,__cov_mmask8);
void __builtin_ia32_compressstoredi256_mask(__cov_v4di*,__cov_v4di,__cov_mmask8);
void __builtin_ia32_compressstoresf128_mask(__cov_v4sf*,__cov_v4sf,__cov_mmask8);
void __builtin_ia32_compressstoresf256_mask(__cov_v8sf*,__cov_v8sf,__cov_mmask8);
void __builtin_ia32_compressstoresi128_mask(__cov_v4si*,__cov_v4si,__cov_mmask8);
void __builtin_ia32_compressstoresi256_mask(__cov_v8si*,__cov_v8si,__cov_mmask8);
void __builtin_ia32_movdqa32store128_mask(__cov_v4si*,__cov_v4si,__cov_mmask8);
void __builtin_ia32_movdqa32store256_mask(__cov_v8si*,__cov_v8si,__cov_mmask8);
void __builtin_ia32_movdqa64store128_mask(__cov_v2di*,__cov_v2di,__cov_mmask8);
void __builtin_ia32_movdqa64store256_mask(__cov_v4di*,__cov_v4di,__cov_mmask8);
void __builtin_ia32_pmovdb128mem_mask(__cov_v16qi*,__cov_v4si,__cov_mmask8);
void __builtin_ia32_pmovdb256mem_mask(__cov_v16qi*,__cov_v8si,__cov_mmask8);
void __builtin_ia32_pmovdw128mem_mask(__cov_v8hi*,__cov_v4si,__cov_mmask8);
void __builtin_ia32_pmovdw256mem_mask(__cov_v8hi*,__cov_v8si,__cov_mmask8);
void __builtin_ia32_pmovqb128mem_mask(__cov_v16qi*,__cov_v2di,__cov_mmask8);
void __builtin_ia32_pmovqb256mem_mask(__cov_v16qi*,__cov_v4di,__cov_mmask8);
void __builtin_ia32_pmovqd128mem_mask(__cov_v4si*,__cov_v2di,__cov_mmask8);
void __builtin_ia32_pmovqd256mem_mask(__cov_v4si*,__cov_v4di,__cov_mmask8);
void __builtin_ia32_pmovqw128mem_mask(__cov_v8hi*,__cov_v2di,__cov_mmask8);
void __builtin_ia32_pmovqw256mem_mask(__cov_v8hi*,__cov_v4di,__cov_mmask8);
void __builtin_ia32_pmovsdb128mem_mask(__cov_v16qi*,__cov_v4si,__cov_mmask8);
void __builtin_ia32_pmovsdb256mem_mask(__cov_v16qi*,__cov_v8si,__cov_mmask8);
void __builtin_ia32_pmovsdw128mem_mask(__cov_v8hi*,__cov_v4si,__cov_mmask8);
void __builtin_ia32_pmovsdw256mem_mask(__cov_v8hi*,__cov_v8si,__cov_mmask8);
void __builtin_ia32_pmovsqb128mem_mask(__cov_v16qi*,__cov_v2di,__cov_mmask8);
void __builtin_ia32_pmovsqb256mem_mask(__cov_v16qi*,__cov_v4di,__cov_mmask8);
void __builtin_ia32_pmovsqd128mem_mask(__cov_v4si*,__cov_v2di,__cov_mmask8);
void __builtin_ia32_pmovsqd256mem_mask(__cov_v4si*,__cov_v4di,__cov_mmask8);
void __builtin_ia32_pmovsqw128mem_mask(__cov_v8hi*,__cov_v2di,__cov_mmask8);
void __builtin_ia32_pmovsqw256mem_mask(__cov_v8hi*,__cov_v4di,__cov_mmask8);
void __builtin_ia32_pmovusdb128mem_mask(__cov_v16qi*,__cov_v4si,__cov_mmask8);
void __builtin_ia32_pmovusdb256mem_mask(__cov_v16qi*,__cov_v8si,__cov_mmask8);
void __builtin_ia32_pmovusdw128mem_mask(__cov_v8hi*,__cov_v4si,__cov_mmask8);
void __builtin_ia32_pmovusdw256mem_mask(__cov_v8hi*,__cov_v8si,__cov_mmask8);
void __builtin_ia32_pmovusqb128mem_mask(__cov_v16qi*,__cov_v2di,__cov_mmask8);
void __builtin_ia32_pmovusqb256mem_mask(__cov_v16qi*,__cov_v4di,__cov_mmask8);
void __builtin_ia32_pmovusqd128mem_mask(__cov_v4si*,__cov_v2di,__cov_mmask8);
void __builtin_ia32_pmovusqd256mem_mask(__cov_v4si*,__cov_v4di,__cov_mmask8);
void __builtin_ia32_pmovusqw128mem_mask(__cov_v8hi*,__cov_v2di,__cov_mmask8);
void __builtin_ia32_pmovusqw256mem_mask(__cov_v8hi*,__cov_v4di,__cov_mmask8);
void __builtin_ia32_storeapd128_mask(__cov_v2df*,__cov_v2df,__cov_mmask8);
void __builtin_ia32_storeapd256_mask(__cov_v4df*,__cov_v4df,__cov_mmask8);
void __builtin_ia32_storeaps128_mask(__cov_v4sf*,__cov_v4sf,__cov_mmask8);
void __builtin_ia32_storeaps256_mask(__cov_v8sf*,__cov_v8sf,__cov_mmask8);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
void __builtin_ia32_storedqudi128_mask(long long*,__cov_v2di,__cov_mmask8);
void __builtin_ia32_storedqudi256_mask(long long*,__cov_v4di,__cov_mmask8);
void __builtin_ia32_storedquhi128_mask(short*,__cov_v8hi,__cov_mmask8);
void __builtin_ia32_storedquhi256_mask(short*,__cov_v16hi,__cov_mmask16);
void __builtin_ia32_storedquhi512_mask(short*,__cov_v32hi,unsigned int);
void __builtin_ia32_storedquqi128_mask(char*,__cov_v16qi,__cov_mmask16);
void __builtin_ia32_storedquqi256_mask(char*,__cov_v32qi,unsigned int);
void __builtin_ia32_storedquqi512_mask(char*,__cov_v64qi,unsigned long long);
void __builtin_ia32_storedqusi128_mask(int*,__cov_v4si,__cov_mmask8);
void __builtin_ia32_storedqusi256_mask(int*,__cov_v8si,__cov_mmask8);
void __builtin_ia32_storeupd128_mask(double*,__cov_v2df,__cov_mmask8);
void __builtin_ia32_storeupd256_mask(double*,__cov_v4df,__cov_mmask8);
void __builtin_ia32_storeups128_mask(float *,__cov_v4sf,__cov_mmask8);
void __builtin_ia32_storeups256_mask(float*,__cov_v8sf,__cov_mmask8);
#else
void __builtin_ia32_storedqudi128_mask(__cov_v2di*,__cov_v2di,__cov_mmask8);
void __builtin_ia32_storedqudi256_mask(__cov_v4di*,__cov_v4di,__cov_mmask8);
void __builtin_ia32_storedquhi128_mask(__cov_v8hi*,__cov_v8hi,__cov_mmask8);
void __builtin_ia32_storedquhi256_mask(__cov_v16hi*,__cov_v16hi,__cov_mmask16);
void __builtin_ia32_storedquhi512_mask(__cov_v32hi*,__cov_v32hi,unsigned int);
void __builtin_ia32_storedquqi128_mask(__cov_v16qi*,__cov_v16qi,__cov_mmask16);
void __builtin_ia32_storedquqi256_mask(__cov_v32qi*,__cov_v32qi,unsigned int);
void __builtin_ia32_storedquqi512_mask(__cov_v64qi*,__cov_v64qi,unsigned long long);
void __builtin_ia32_storedqusi128_mask(__cov_v4si*,__cov_v4si,__cov_mmask8);
void __builtin_ia32_storedqusi256_mask(__cov_v8si*,__cov_v8si,__cov_mmask8);
void __builtin_ia32_storeupd128_mask(__cov_v2df*,__cov_v2df,__cov_mmask8);
void __builtin_ia32_storeupd256_mask(__cov_v4df*,__cov_v4df,__cov_mmask8);
void __builtin_ia32_storeups128_mask(__cov_v4sf*,__cov_v4sf,__cov_mmask8);
void __builtin_ia32_storeups256_mask(__cov_v8sf*,__cov_v8sf,__cov_mmask8);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
void __builtin_ia32_xrstors(void*,long long);
void __builtin_ia32_xrstors64(void*,long long);
void __builtin_ia32_xsavec(void*,long long);
void __builtin_ia32_xsavec64(void*,long long);
void __builtin_ia32_xsaves(void*,long long);
void __builtin_ia32_xsaves64(void*,long long);
void* __builtin_ia32_bndlower(__cov_bounds_type);
void* __builtin_ia32_bndupper(__cov_bounds_type);
void* __builtin_ia32_narrow_bounds(void const *,__cov_bounds_type,unsigned long);
__cov_m512i __builtin_ia32_vpmadd52luq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_vpmadd52huq512_mask(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_vpmadd52luq512_maskz(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m512i __builtin_ia32_vpmadd52huq512_maskz(__cov_v8di,__cov_v8di,__cov_v8di,__cov_mmask8);
__cov_m128i __builtin_ia32_vpmadd52luq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_m128i __builtin_ia32_vpmadd52huq128_mask(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_m256i __builtin_ia32_vpmadd52luq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_m256i __builtin_ia32_vpmadd52huq256_mask(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_m128i __builtin_ia32_vpmadd52luq128_maskz(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_m128i __builtin_ia32_vpmadd52huq128_maskz(__cov_v2di,__cov_v2di,__cov_v2di,__cov_mmask8);
__cov_m256i __builtin_ia32_vpmadd52luq256_maskz(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_m256i __builtin_ia32_vpmadd52huq256_maskz(__cov_v4di,__cov_v4di,__cov_v4di,__cov_mmask8);
__cov_m256d __builtin_ia32_movapd256_mask(__cov_v4df,__cov_v4df,__cov_mmask8);
__cov_m128d __builtin_ia32_movapd128_mask(__cov_v2df,__cov_v2df,__cov_mmask8);
__cov_m256d __builtin_ia32_loadapd256_mask(const __cov_v4df *,__cov_v4df,__cov_mmask8);
__cov_m128d __builtin_ia32_loadapd128_mask(const __cov_v2df *,__cov_v2df,__cov_mmask8);
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_m256 __builtin_ia32_gather3siv8sf(__cov_v8sf,const void*,__cov_v8si,__cov_mmask8,int);
__cov_m128 __builtin_ia32_gather3siv4sf(__cov_v4sf,const void*,__cov_v4si,__cov_mmask8,int);
__cov_m256d __builtin_ia32_gather3siv4df(__cov_v4df,const void*,__cov_v4si,__cov_mmask8,int);
__cov_m128d __builtin_ia32_gather3siv2df(__cov_v2df,const void*,__cov_v4si,__cov_mmask8,int);
__cov_m128 __builtin_ia32_gather3div8sf(__cov_v4sf,const void*,__cov_v4di,__cov_mmask8,int);
__cov_m128 __builtin_ia32_gather3div4sf(__cov_v4sf,const void*,__cov_v2di,__cov_mmask8,int);
__cov_m256d __builtin_ia32_gather3div4df(__cov_v4df,const void*,__cov_v4di,__cov_mmask8,int);
__cov_m128d __builtin_ia32_gather3div2df(__cov_v2df,const void*,__cov_v2di,__cov_mmask8,int);
__cov_m256i __builtin_ia32_gather3siv8si(__cov_v8si,const void*,__cov_v8si,__cov_mmask8,int);
__cov_m128i __builtin_ia32_gather3siv4si(__cov_v4si,const void*,__cov_v4si,__cov_mmask8,int);
__cov_m256i __builtin_ia32_gather3siv4di(__cov_v4di,const void*,__cov_v4si,__cov_mmask8,int);
__cov_m128i __builtin_ia32_gather3siv2di(__cov_v2di,const void*,__cov_v4si,__cov_mmask8,int);
__cov_m128i __builtin_ia32_gather3div8si(__cov_v4si,const void*,__cov_v4di,__cov_mmask8,int);
__cov_m128i __builtin_ia32_gather3div4si(__cov_v4si,const void*,__cov_v2di,__cov_mmask8,int);
__cov_m256i __builtin_ia32_gather3div4di(__cov_v4di,const void*,__cov_v4di,__cov_mmask8,int);
__cov_m128i __builtin_ia32_gather3div2di(__cov_v2di,const void*,__cov_v2di,__cov_mmask8,int);
void __builtin_ia32_scattersiv8sf(void*,__cov_mmask8,__cov_v8si,__cov_v8sf,int);
void __builtin_ia32_scattersiv4sf(void*,__cov_mmask8,__cov_v4si,__cov_v4sf,int);
void __builtin_ia32_scattersiv4df(void*,__cov_mmask8,__cov_v4si,__cov_v4df,int);
void __builtin_ia32_scattersiv2df(void*,__cov_mmask8,__cov_v4si,__cov_v2df,int);
void __builtin_ia32_scatterdiv8sf(const void*,__cov_mmask8,__cov_v4di,__cov_v4sf,int);
void __builtin_ia32_scatterdiv4sf(const void*,__cov_mmask8,__cov_v2di,__cov_v4sf,int);
void __builtin_ia32_scatterdiv4df(const void*,__cov_mmask8,__cov_v4di,__cov_v4df,int);
void __builtin_ia32_scatterdiv2df(const void*,__cov_mmask8,__cov_v2di,__cov_v2df,int);
void __builtin_ia32_scattersiv8si(const void*,__cov_mmask8,__cov_v8si,__cov_v8si,int);
void __builtin_ia32_scattersiv4si(const void*,__cov_mmask8,__cov_v4si,__cov_v4si,int);
void __builtin_ia32_scattersiv4di(const void*,__cov_mmask8,__cov_v4si,__cov_v4di,int);
void __builtin_ia32_scattersiv2di(const void*,__cov_mmask8,__cov_v4si,__cov_v2di,int);
void __builtin_ia32_scatterdiv8si(const void*,__cov_mmask8,__cov_v4di,__cov_v4si,int);
void __builtin_ia32_scatterdiv4si(const void*,__cov_mmask8,__cov_v2di,__cov_v4si,int);
void __builtin_ia32_scatterdiv4di(const void*,__cov_mmask8,__cov_v4di,__cov_v4di,int);
void __builtin_ia32_scatterdiv2di(const void*,__cov_mmask8,__cov_v2di,__cov_v2di,int);
#else
__cov_m256 __builtin_ia32_gather3siv8sf(__cov_v8sf,const float *,__cov_v8si,__cov_mmask8,int);
__cov_m128 __builtin_ia32_gather3siv4sf(__cov_v4sf,const float *,__cov_v4si,__cov_mmask8,int);
__cov_m256d __builtin_ia32_gather3siv4df(__cov_v4df,const double *,__cov_v4si,__cov_mmask8,int);
__cov_m128d __builtin_ia32_gather3siv2df(__cov_v2df,const double *,__cov_v4si,__cov_mmask8,int);
__cov_m128 __builtin_ia32_gather3div8sf(__cov_v4sf,const float *,__cov_v4di,__cov_mmask8,int);
__cov_m128 __builtin_ia32_gather3div4sf(__cov_v4sf,const float *,__cov_v2di,__cov_mmask8,int);
__cov_m256d __builtin_ia32_gather3div4df(__cov_v4df,const double *,__cov_v4di,__cov_mmask8,int);
__cov_m128d __builtin_ia32_gather3div2df(__cov_v2df,const double *,__cov_v2di,__cov_mmask8,int);
__cov_m256i __builtin_ia32_gather3siv8si(__cov_v8si,const int *,__cov_v8si,__cov_mmask8,int);
__cov_m128i __builtin_ia32_gather3siv4si(__cov_v4si,const int *,__cov_v4si,__cov_mmask8,int);
__cov_m256i __builtin_ia32_gather3siv4di(__cov_v4di,const long long *,__cov_v4si,__cov_mmask8,int);
__cov_m128i __builtin_ia32_gather3siv2di(__cov_v2di,const long long *,__cov_v4si,__cov_mmask8,int);
__cov_m128i __builtin_ia32_gather3div8si(__cov_v4si,const int *,__cov_v4di,__cov_mmask8,int);
__cov_m128i __builtin_ia32_gather3div4si(__cov_v4si,const int *,__cov_v2di,__cov_mmask8,int);
__cov_m256i __builtin_ia32_gather3div4di(__cov_v4di,const long long *,__cov_v4di,__cov_mmask8,int);
__cov_m128i __builtin_ia32_gather3div2di(__cov_v2di,const long long *,__cov_v2di,__cov_mmask8,int);
void __builtin_ia32_scattersiv8sf(float *,__cov_mmask8,__cov_v8si,__cov_v8sf,int);
void __builtin_ia32_scattersiv4sf(float*,__cov_mmask8,__cov_v4si,__cov_v4sf,int);
void __builtin_ia32_scattersiv4df(double *,__cov_mmask8,__cov_v4si,__cov_v4df,int);
void __builtin_ia32_scattersiv2df(double *,__cov_mmask8,__cov_v4si,__cov_v2df,int);
void __builtin_ia32_scatterdiv8sf(const float *,__cov_mmask8,__cov_v4di,__cov_v4sf,int);
void __builtin_ia32_scatterdiv4sf(const float *,__cov_mmask8,__cov_v2di,__cov_v4sf,int);
void __builtin_ia32_scatterdiv4df(const double *,__cov_mmask8,__cov_v4di,__cov_v4df,int);
void __builtin_ia32_scatterdiv2df(const double *,__cov_mmask8,__cov_v2di,__cov_v2df,int);
void __builtin_ia32_scattersiv8si(const int *,__cov_mmask8,__cov_v8si,__cov_v8si,int);
void __builtin_ia32_scattersiv4si(const int *,__cov_mmask8,__cov_v4si,__cov_v4si,int);
void __builtin_ia32_scattersiv4di(const long long *,__cov_mmask8,__cov_v4si,__cov_v4di,int);
void __builtin_ia32_scattersiv2di(const long long *,__cov_mmask8,__cov_v4si,__cov_v2di,int);
void __builtin_ia32_scatterdiv8si(const int *,__cov_mmask8,__cov_v4di,__cov_v4si,int);
void __builtin_ia32_scatterdiv4si(const int *,__cov_mmask8,__cov_v2di,__cov_v4si,int);
void __builtin_ia32_scatterdiv4di(const long long *,__cov_mmask8,__cov_v4di,__cov_v4di,int);
void __builtin_ia32_scatterdiv2di(const long long *,__cov_mmask8,__cov_v2di,__cov_v2di,int);
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_m512i __builtin_ia32_vpmultishiftqb512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,__cov_mmask64);
__cov_m512i __builtin_ia32_permvarqi512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,__cov_mmask64);
__cov_m512i __builtin_ia32_vpermt2varqi512_mask(__cov_v64qi,__cov_v64qi, __cov_v64qi,__cov_mmask64);
__cov_m512i __builtin_ia32_vpermi2varqi512_mask(__cov_v64qi,__cov_v64qi, __cov_v64qi,__cov_mmask64);
__cov_m512i __builtin_ia32_vpmultishiftqb512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,__cov_mmask64);
__cov_m512i __builtin_ia32_permvarqi512_mask(__cov_v64qi,__cov_v64qi,__cov_v64qi,__cov_mmask64);
__cov_m512i __builtin_ia32_vpermt2varqi512_maskz(__cov_v64qi,__cov_v64qi,__cov_v64qi,__cov_mmask64);
__cov_m256i __builtin_ia32_vpmultishiftqb256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,__cov_mmask32);
__cov_m128i __builtin_ia32_vpmultishiftqb128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_m256i __builtin_ia32_permvarqi256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,__cov_mmask32);
__cov_m128i __builtin_ia32_permvarqi128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_m256i __builtin_ia32_vpermt2varqi256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,__cov_mmask32);
__cov_m256i __builtin_ia32_vpermi2varqi256_mask(__cov_v32qi,__cov_v32qi,__cov_v32qi,__cov_mmask32);
__cov_m256i __builtin_ia32_vpermt2varqi256_maskz(__cov_v32qi,__cov_v32qi,__cov_v32qi,__cov_mmask32);
__cov_m128i __builtin_ia32_vpermt2varqi128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_m128i __builtin_ia32_vpermi2varqi128_mask(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
__cov_m128i __builtin_ia32_vpermt2varqi128_maskz(__cov_v16qi,__cov_v16qi,__cov_v16qi,__cov_mmask16);
void __builtin_ia32_clwb(void *);
void __builtin_ia32_pcommit(void);


#endif /* 5.0+ */

#if __COVERITY_GCC_VERSION_AT_LEAST(5, 2)
void __builtin_ia32_monitorx (const void *, unsigned int, unsigned int);
void __builtin_ia32_mwaitx (unsigned int, unsigned int, unsigned int);
#endif /* 5.2+ */

#if __COVERITY_GCC_VERSION_AT_LEAST(6, 0)
void __builtin_ia32_clzero(void*);
unsigned int __builtin_ia32_rdpkru(void);
void __builtin_ia32_wrpkru(unsigned int);
#endif /* 6.0+ */

#endif /* defined(__GNUC__) */

#ifdef __COVERITY_ALTIVEC__
/* Declarations of AltiVec PPU built-in intrinsic functions. */
/* Many of these are overloaded, so they can't be declared in C.  For those
 * that have operator equivalents those equivalents are used in C instead,
 * otherwise a generic version with no parameter types is declared.
 */

typedef vector int __coverity_generic_altivec_vector;

#ifdef __cplusplus
extern vector signed char __builtin_vec_abs (vector signed char a1);
extern vector signed short __builtin_vec_abs (vector signed short a1);
extern vector signed int __builtin_vec_abs (vector signed int a1);
extern vector float __builtin_vec_abs (vector float a1);

extern vector signed char __builtin_vec_abss (vector signed char a1);
extern vector signed short __builtin_vec_abss (vector signed short a1);
#else
__coverity_generic_altivec_vector  __builtin_vec_abs();
__coverity_generic_altivec_vector  __builtin_vec_abss();
#endif

#ifdef __cplusplus
    extern vector unsigned int __builtin_vec_vslw (vector unsigned int a1, vector unsigned int a2);
#else
__coverity_generic_altivec_vector  __builtin_vec_vslw ();
#endif

#ifdef __cplusplus
extern int __builtin_vec_vcmpgt_p(int, vector signed char a1, vector signed char a2);
extern int __builtin_vec_vcmpgt_p(int, vector unsigned char a1, vector unsigned char a2);
extern int __builtin_vec_vcmpgt_p(int, vector signed short a1, vector signed short a2);
extern int __builtin_vec_vcmpgt_p(int, vector unsigned short a1, vector unsigned short a2);
extern int __builtin_vec_vcmpgt_p(int, vector signed int a1, vector signed int a2);
extern int __builtin_vec_vcmpgt_p(int, vector unsigned int a1, vector unsigned int a2);
extern int __builtin_vec_vcmpgt_p(int, vector signed char a1, vector signed char a2);
extern int __builtin_vec_vcmpgt_p(int, vector float a1, vector float a2);
#else
int __builtin_vec_vcmpgt_p();
#endif

#ifdef __cplusplus
extern vector signed char __builtin_vec_add (vector signed char a1, vector signed char a2);
extern vector unsigned char __builtin_vec_add (vector signed char a1, vector unsigned char a2);

extern vector unsigned char __builtin_vec_add (vector unsigned char a1, vector signed char a2);

extern vector unsigned char __builtin_vec_add (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed short __builtin_vec_add (vector signed short a1, vector signed short a2);
extern vector unsigned short __builtin_vec_add (vector signed short a1,
                               vector unsigned short a2);
extern vector unsigned short __builtin_vec_add (vector unsigned short a1,
                               vector signed short a2);
extern vector unsigned short __builtin_vec_add (vector unsigned short a1,
                               vector unsigned short a2);
extern vector signed int __builtin_vec_add (vector signed int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_add (vector signed int a1, vector unsigned int a2);
extern vector unsigned int __builtin_vec_add (vector unsigned int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_add (vector unsigned int a1, vector unsigned int a2);
extern vector float __builtin_vec_add (vector float a1, vector float a2);
#else
# define __builtin_vec_add(a, b) ((a) + (b))
#endif

extern vector unsigned int __builtin_vec_addc (vector unsigned int a1, vector unsigned int a2);

#ifdef __cplusplus
extern vector unsigned char __builtin_vec_adds (vector signed char a1,
                               vector unsigned char a2);
extern vector unsigned char __builtin_vec_adds (vector unsigned char a1,
                               vector signed char a2);
extern vector unsigned char __builtin_vec_adds (vector unsigned char a1,
                               vector unsigned char a2);
extern vector signed char __builtin_vec_adds (vector signed char a1, vector signed char a2);
extern vector unsigned short __builtin_vec_adds (vector signed short a1,
                                vector unsigned short a2);
extern vector unsigned short __builtin_vec_adds (vector unsigned short a1,
                                vector signed short a2);
extern vector unsigned short __builtin_vec_adds (vector unsigned short a1,
                                vector unsigned short a2);
extern vector signed short __builtin_vec_adds (vector signed short a1, vector signed short a2);

extern vector unsigned int __builtin_vec_adds (vector signed int a1, vector unsigned int a2);
extern vector unsigned int __builtin_vec_adds (vector unsigned int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_adds (vector unsigned int a1, vector unsigned int a2);

extern vector signed int __builtin_vec_adds (vector signed int a1, vector signed int a2);
#else
__coverity_generic_altivec_vector  __builtin_vec_adds ();
#endif

#ifdef __cplusplus
extern vector float __builtin_vec_and (vector float a1, vector float a2);
extern vector float __builtin_vec_and (vector float a1, vector signed int a2);
extern vector float __builtin_vec_and (vector signed int a1, vector float a2);
extern vector signed int __builtin_vec_and (vector signed int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_and (vector signed int a1, vector unsigned int a2);
extern vector unsigned int __builtin_vec_and (vector unsigned int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_and (vector unsigned int a1, vector unsigned int a2);
extern vector signed short __builtin_vec_and (vector signed short a1, vector signed short a2);
extern vector unsigned short __builtin_vec_and (vector signed short a1,
                               vector unsigned short a2);
extern vector unsigned short __builtin_vec_and (vector unsigned short a1,
                               vector signed short a2);
extern vector unsigned short __builtin_vec_and (vector unsigned short a1,
                               vector unsigned short a2);
extern  vector signed char __builtin_vec_and (vector signed char a1, vector signed char a2);
extern vector unsigned char __builtin_vec_and (vector signed char a1, vector unsigned char a2);

extern vector unsigned char __builtin_vec_and (vector unsigned char a1, vector signed char a2);

extern vector unsigned char __builtin_vec_and (vector unsigned char a1,
                              vector unsigned char a2);
#else
# define __builtin_vec_and(a, b) ((a) & (b))
#endif

#ifdef __cplusplus
extern vector float __builtin_vec_andc (vector float a1, vector float a2);
extern vector float __builtin_vec_andc (vector float a1, vector signed int a2);
extern vector float __builtin_vec_andc (vector signed int a1, vector float a2);
extern vector signed int __builtin_vec_andc (vector signed int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_andc (vector signed int a1, vector unsigned int a2);
extern vector unsigned int __builtin_vec_andc (vector unsigned int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_andc (vector unsigned int a1, vector unsigned int a2);

extern vector signed short __builtin_vec_andc (vector signed short a1, vector signed short a2);

extern vector unsigned short __builtin_vec_andc (vector signed short a1,
                                vector unsigned short a2);
extern vector unsigned short __builtin_vec_andc (vector unsigned short a1,
                                vector signed short a2);
extern vector unsigned short __builtin_vec_andc (vector unsigned short a1,
                                vector unsigned short a2);
extern vector signed char __builtin_vec_andc (vector signed char a1, vector signed char a2);
extern vector unsigned char __builtin_vec_andc (vector signed char a1,
                               vector unsigned char a2);
extern vector unsigned char __builtin_vec_andc (vector unsigned char a1,
                               vector signed char a2);
extern vector unsigned char __builtin_vec_andc (vector unsigned char a1,
                               vector unsigned char a2);
#else
__coverity_generic_altivec_vector  __builtin_vec_andc ();
#endif

#ifdef __cplusplus
extern vector unsigned char __builtin_vec_avg (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed char __builtin_vec_avg (vector signed char a1, vector signed char a2);
extern vector unsigned short __builtin_vec_avg (vector unsigned short a1,
                               vector unsigned short a2);
extern vector signed short __builtin_vec_avg (vector signed short a1, vector signed short a2);
extern vector unsigned int __builtin_vec_avg (vector unsigned int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_avg (vector signed int a1, vector signed int a2);
#else
__coverity_generic_altivec_vector  __builtin_vec_avg ();
#endif

extern vector float __builtin_vec_ceil (vector float a1);

extern vector signed int __builtin_vec_cmpb (vector float a1, vector float a2);

#ifdef __cplusplus
extern vector signed char __builtin_vec_cmpeq (vector signed char a1, vector signed char a2);
extern vector signed char __builtin_vec_cmpeq (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed short __builtin_vec_cmpeq (vector signed short a1,
                               vector signed short a2);
extern vector signed short __builtin_vec_cmpeq (vector unsigned short a1,
                               vector unsigned short a2);
extern vector signed int __builtin_vec_cmpeq (vector signed int a1, vector signed int a2);
extern vector signed int __builtin_vec_cmpeq (vector unsigned int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_cmpeq (vector float a1, vector float a2);

extern vector signed int __builtin_vec_cmpge (vector float a1, vector float a2);

extern vector signed char __builtin_vec_cmpgt (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed char __builtin_vec_cmpgt (vector signed char a1, vector signed char a2);
extern vector signed short __builtin_vec_cmpgt (vector unsigned short a1,
                               vector unsigned short a2);
extern vector signed short __builtin_vec_cmpgt (vector signed short a1,
                               vector signed short a2);
extern vector signed int __builtin_vec_cmpgt (vector unsigned int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_cmpgt (vector signed int a1, vector signed int a2);
extern vector signed int __builtin_vec_cmpgt (vector float a1, vector float a2);

extern vector signed int __builtin_vec_cmple (vector float a1, vector float a2);

extern vector signed char __builtin_vec_cmplt (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed char __builtin_vec_cmplt (vector signed char a1, vector signed char a2);
extern vector signed short __builtin_vec_cmplt (vector unsigned short a1,
                               vector unsigned short a2);
extern vector signed short __builtin_vec_cmplt (vector signed short a1,
                               vector signed short a2);
extern vector signed int __builtin_vec_cmplt (vector unsigned int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_cmplt (vector signed int a1, vector signed int a2);
extern vector signed int __builtin_vec_cmplt (vector float a1, vector float a2);
#else
__coverity_generic_altivec_vector  __builtin_vec_cmpeq ();
__coverity_generic_altivec_vector  __builtin_vec_cmpge ();
__coverity_generic_altivec_vector  __builtin_vec_cmpgt ();
__coverity_generic_altivec_vector  __builtin_vec_cmple ();
__coverity_generic_altivec_vector  __builtin_vec_cmplt ();
#endif

#ifdef __cplusplus
extern vector float __builtin_vec_ctf (vector unsigned int a1, const char a2);
extern vector float __builtin_vec_ctf (vector signed int a1, const char a2);
#else
extern vector float __builtin_vec_ctf ();
#endif

extern vector signed int __builtin_vec_cts (vector float a1, const char a2);

extern vector unsigned int __builtin_vec_ctu (vector float a1, const char a2);

extern void __builtin_vec_dss (const char a1);

extern void __builtin_vec_dssall (void);

extern void __builtin_vec_dst (const void * a1, int a2, const char a3);

extern void __builtin_vec_dstst (const void * a1, int a2, const char a3);

extern void __builtin_vec_dststt (const void * a1, int a2, const char a3);

extern void __builtin_vec_dstt (const void * a1, int a2, const char a3);

#ifdef __cplusplus
extern vector float __builtin_vec_expte (vector float a1, vector float a2);
extern vector float __builtin_vec_expte (vector float a1);

extern vector float __builtin_vec_floor (vector float a1, vector float a2);
extern vector float __builtin_vec_floor (vector float a1);
#else
__coverity_generic_altivec_vector __builtin_vec_expte();
__coverity_generic_altivec_vector __builtin_vec_floor();
#endif

#ifdef __cplusplus
extern vector float __builtin_vec_ld (int a1, vector float * a2);
extern vector float __builtin_vec_ld (int a1, float * a2);
extern vector signed int __builtin_vec_ld (int a1, int * a2);
extern vector signed int __builtin_vec_ld (int a1, vector signed int * a2);
extern vector unsigned int __builtin_vec_ld (int a1, vector unsigned int * a2);
extern vector unsigned int __builtin_vec_ld (int a1, unsigned int * a2);
extern vector signed short __builtin_vec_ld (int a1, short * a2, vector signed short * a3);
extern vector unsigned short __builtin_vec_ld (int a1, unsigned short * a2,
                              vector unsigned short * a3);
extern vector signed char __builtin_vec_ld (int a1, signed char * a2);
extern vector signed char __builtin_vec_ld (int a1, vector signed char * a2);
extern vector unsigned char __builtin_vec_ld (int a1, unsigned char * a2);
extern vector unsigned char __builtin_vec_ld (int a1, vector unsigned char * a2);

extern vector signed char __builtin_vec_lde (int a1, signed char * a2);
extern vector unsigned char __builtin_vec_lde (int a1, unsigned char * a2);
extern vector signed short __builtin_vec_lde (int a1, short * a2);
extern vector unsigned short __builtin_vec_lde (int a1, unsigned short * a2);
extern vector float __builtin_vec_lde (int a1, float * a2);
extern vector signed int __builtin_vec_lde (int a1, int * a2);
extern vector unsigned int __builtin_vec_lde (int a1, unsigned int * a2);

extern vector float __builtin_vec_ldl (int a1, float * a2);
extern vector float __builtin_vec_ldl (int a1, vector float * a2);
extern vector signed int __builtin_vec_ldl (int a1, vector signed int * a2);
extern vector signed int __builtin_vec_ldl (int a1, int * a2);
extern vector unsigned int __builtin_vec_ldl (int a1, unsigned int * a2);
extern vector unsigned int __builtin_vec_ldl (int a1, vector unsigned int * a2);
extern vector signed short __builtin_vec_ldl (int a1, vector signed short * a2);
extern vector signed short __builtin_vec_ldl (int a1, short * a2);
extern vector unsigned short __builtin_vec_ldl (int a1, vector unsigned short * a2);
extern vector unsigned short __builtin_vec_ldl (int a1, unsigned short * a2);
extern vector signed char __builtin_vec_ldl (int a1, vector signed char * a2);
extern vector signed char __builtin_vec_ldl (int a1, signed char * a2);
extern vector unsigned char __builtin_vec_ldl (int a1, vector unsigned char * a2);
extern vector unsigned char __builtin_vec_ldl (int a1, unsigned char * a2);
#else
__coverity_generic_altivec_vector  __builtin_vec_ld ();
__coverity_generic_altivec_vector  __builtin_vec_lde ();
__coverity_generic_altivec_vector  __builtin_vec_ldl ();
#endif

extern vector float __builtin_vec_loge (vector float a1);

extern vector unsigned char __builtin_vec_lvsl (int a1, const void * a2, int * a3);

extern vector unsigned char __builtin_vec_lvsr (int a1, const void * a2, int * a3);

extern vector float __builtin_vec_madd (vector float a1, vector float a2, vector float a3);

extern vector signed short __builtin_vec_madds (vector signed short a1, vector signed short a2,
                               vector signed short a3);

#ifdef __cplusplus
extern vector unsigned char __builtin_vec_max (vector signed char a1, vector unsigned char a2);

extern vector unsigned char __builtin_vec_max (vector unsigned char a1, vector signed char a2);

extern vector unsigned char __builtin_vec_max (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed char __builtin_vec_max (vector signed char a1, vector signed char a2);
extern vector unsigned short __builtin_vec_max (vector signed short a1,
                               vector unsigned short a2);
extern vector unsigned short __builtin_vec_max (vector unsigned short a1,
                               vector signed short a2);
extern vector unsigned short __builtin_vec_max (vector unsigned short a1,
                               vector unsigned short a2);
extern vector signed short __builtin_vec_max (vector signed short a1, vector signed short a2);
extern vector unsigned int __builtin_vec_max (vector signed int a1, vector unsigned int a2);
extern vector unsigned int __builtin_vec_max (vector unsigned int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_max (vector unsigned int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_max (vector signed int a1, vector signed int a2);
extern vector float __builtin_vec_max (vector float a1, vector float a2);

extern vector signed char __builtin_vec_mergeh (vector signed char a1, vector signed char a2);
extern vector unsigned char __builtin_vec_mergeh (vector unsigned char a1,
                                 vector unsigned char a2);
extern vector signed short __builtin_vec_mergeh (vector signed short a1,
                                vector signed short a2);
extern vector unsigned short __builtin_vec_mergeh (vector unsigned short a1,
                                  vector unsigned short a2);
extern vector float __builtin_vec_mergeh (vector float a1, vector float a2);
extern vector signed int __builtin_vec_mergeh (vector signed int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_mergeh (vector unsigned int a1,
                                vector unsigned int a2);

extern vector signed char __builtin_vec_mergel (vector signed char a1, vector signed char a2);
extern vector unsigned char __builtin_vec_mergel (vector unsigned char a1,
                                 vector unsigned char a2);
extern vector signed short __builtin_vec_mergel (vector signed short a1,
                                vector signed short a2);
extern vector unsigned short __builtin_vec_mergel (vector unsigned short a1,
                                  vector unsigned short a2);
extern vector float __builtin_vec_mergel (vector float a1, vector float a2);
extern vector signed int __builtin_vec_mergel (vector signed int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_mergel (vector unsigned int a1,
                                vector unsigned int a2);
#else
__coverity_generic_altivec_vector  __builtin_vec_max ();
__coverity_generic_altivec_vector  __builtin_vec_mergeh ();
__coverity_generic_altivec_vector  __builtin_vec_mergel ();
#endif

extern vector unsigned short __builtin_vec_mfvscr (void);

#ifdef __cplusplus
extern vector unsigned char __builtin_vec_min (vector signed char a1, vector unsigned char a2);

extern vector unsigned char __builtin_vec_min (vector unsigned char a1, vector signed char a2);

extern vector unsigned char __builtin_vec_min (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed char __builtin_vec_min (vector signed char a1, vector signed char a2);
extern vector unsigned short __builtin_vec_min (vector signed short a1,
                               vector unsigned short a2);
extern vector unsigned short __builtin_vec_min (vector unsigned short a1,
                               vector signed short a2);
extern vector unsigned short __builtin_vec_min (vector unsigned short a1,
                               vector unsigned short a2);
extern vector signed short __builtin_vec_min (vector signed short a1, vector signed short a2);
extern vector unsigned int __builtin_vec_min (vector signed int a1, vector unsigned int a2);
extern vector unsigned int __builtin_vec_min (vector unsigned int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_min (vector unsigned int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_min (vector signed int a1, vector signed int a2);
extern vector float __builtin_vec_min (vector float a1, vector float a2);

extern vector signed short __builtin_vec_mladd (vector signed short a1, vector signed short a2,
                               vector signed short a3);
extern vector signed short __builtin_vec_mladd (vector signed short a1,
                               vector unsigned short a2,
                               vector unsigned short a3);
extern vector signed short __builtin_vec_mladd (vector unsigned short a1,
                               vector signed short a2,
                               vector signed short a3);
extern vector unsigned short __builtin_vec_mladd (vector unsigned short a1,
                                 vector unsigned short a2,
                                 vector unsigned short a3);
#else
__coverity_generic_altivec_vector __builtin_vec_min ();
__coverity_generic_altivec_vector __builtin_vec_mladd ();
#endif

extern vector signed short __builtin_vec_mradds (vector signed short a1,
                                vector signed short a2,
                                vector signed short a3);

#ifdef __cplusplus
extern vector unsigned int __builtin_vec_msum (vector unsigned char a1,
                              vector unsigned char a2,
                              vector unsigned int a3);
extern vector signed int __builtin_vec_msum (vector signed char a1, vector unsigned char a2,
                            vector signed int a3);
extern vector unsigned int __builtin_vec_msum (vector unsigned short a1,
                              vector unsigned short a2,
                              vector unsigned int a3);
extern vector signed int __builtin_vec_msum (vector signed short a1, vector signed short a2,
                            vector signed int a3);

extern vector unsigned int __builtin_vec_msums (vector unsigned short a1,
                               vector unsigned short a2,
                               vector unsigned int a3);
extern vector signed int __builtin_vec_msums (vector signed short a1, vector signed short a2,
                             vector signed int a3);

extern void __builtin_vec_mtvscr (vector signed int a1);
extern void __builtin_vec_mtvscr (vector unsigned int a1);
extern void __builtin_vec_mtvscr (vector signed short a1);
extern void __builtin_vec_mtvscr (vector unsigned short a1);
extern void __builtin_vec_mtvscr (vector signed char a1);
extern void __builtin_vec_mtvscr (vector unsigned char a1);

extern vector unsigned short __builtin_vec_mule (vector unsigned char a1,
                                vector unsigned char a2);
extern vector signed short __builtin_vec_mule (vector signed char a1, vector signed char a2);
extern vector unsigned int __builtin_vec_mule (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_mule (vector signed short a1, vector signed short a2);

extern vector unsigned short __builtin_vec_mulo (vector unsigned char a1,
                                vector unsigned char a2);
extern vector signed short __builtin_vec_mulo (vector signed char a1, vector signed char a2);
extern vector unsigned int __builtin_vec_mulo (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_mulo (vector signed short a1, vector signed short a2);
#else
__coverity_generic_altivec_vector __builtin_vec_msum ();
__coverity_generic_altivec_vector __builtin_vec_msums ();
extern void __builtin_vec_mtvscr ();
__coverity_generic_altivec_vector __builtin_vec_mule ();
__coverity_generic_altivec_vector __builtin_vec_mulo ();
#endif

extern vector float __builtin_vec_nmsub (vector float a1, vector float a2, vector float a3);

#ifdef __cplusplus
extern vector float __builtin_vec_nor (vector float a1, vector float a2);
extern vector signed int __builtin_vec_nor (vector signed int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_nor (vector unsigned int a1, vector unsigned int a2);
extern vector signed short __builtin_vec_nor (vector signed short a1, vector signed short a2);
extern vector unsigned short __builtin_vec_nor (vector unsigned short a1,
                               vector unsigned short a2);
extern vector signed char __builtin_vec_nor (vector signed char a1, vector signed char a2);
extern vector unsigned char __builtin_vec_nor (vector unsigned char a1,
                              vector unsigned char a2);

extern vector float __builtin_vec_or (vector float a1, vector float a2);
extern vector float __builtin_vec_or (vector float a1, vector signed int a2);
extern vector float __builtin_vec_or (vector signed int a1, vector float a2);
extern vector signed int __builtin_vec_or (vector signed int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_or (vector signed int a1, vector unsigned int a2);
extern vector unsigned int __builtin_vec_or (vector unsigned int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_or (vector unsigned int a1, vector unsigned int a2);
extern vector signed short __builtin_vec_or (vector signed short a1, vector signed short a2);
extern vector unsigned short __builtin_vec_or (vector signed short a1,
                              vector unsigned short a2);
extern vector unsigned short __builtin_vec_or (vector unsigned short a1,
                              vector signed short a2);
extern vector unsigned short __builtin_vec_or (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed char __builtin_vec_or (vector signed char a1, vector signed char a2);
extern vector unsigned char __builtin_vec_or (vector signed char a1, vector unsigned char a2);
extern vector unsigned char __builtin_vec_or (vector unsigned char a1, vector signed char a2);
extern vector unsigned char __builtin_vec_or (vector unsigned char a1,
                             vector unsigned char a2);

extern vector signed char __builtin_vec_pack (vector signed short a1, vector signed short a2);
extern vector unsigned char __builtin_vec_pack (vector unsigned short a1,
                               vector unsigned short a2);
extern vector signed short __builtin_vec_pack (vector signed int a1, vector signed int a2);
extern vector unsigned short __builtin_vec_pack (vector unsigned int a1,
                                vector unsigned int a2);
#else
# define __builtin_vec_nor(a, b) (~((a) | (b)))
# define __builtin_vec_or(a, b) ((a) | (b))
__coverity_generic_altivec_vector __builtin_vec_pack ();
#endif

extern vector signed short __builtin_vec_packpx (vector unsigned int a1,
                                vector unsigned int a2);

#ifdef __cplusplus
extern vector unsigned char __builtin_vec_packs (vector unsigned short a1,
                                vector unsigned short a2);
extern vector signed char __builtin_vec_packs (vector signed short a1, vector signed short a2);

extern vector unsigned short __builtin_vec_packs (vector unsigned int a1,
                                 vector unsigned int a2);
extern vector signed short __builtin_vec_packs (vector signed int a1, vector signed int a2);

extern vector unsigned char __builtin_vec_packsu (vector unsigned short a1,
                                 vector unsigned short a2);
extern vector unsigned char __builtin_vec_packsu (vector signed short a1,
                                 vector signed short a2);
extern vector unsigned short __builtin_vec_packsu (vector unsigned int a1,
                                  vector unsigned int a2);
extern vector unsigned short __builtin_vec_packsu (vector signed int a1, vector signed int a2);

extern vector float __builtin_vec_perm (vector float a1, vector float a2,
                       vector unsigned char a3);
extern vector signed int __builtin_vec_perm (vector signed int a1, vector signed int a2,
                            vector unsigned char a3);
extern vector unsigned int __builtin_vec_perm (vector unsigned int a1, vector unsigned int a2,
                              vector unsigned char a3);
extern vector signed short __builtin_vec_perm (vector signed short a1, vector signed short a2,
                              vector unsigned char a3);
extern vector unsigned short __builtin_vec_perm (vector unsigned short a1,
                                vector unsigned short a2,
                                vector unsigned char a3);
extern vector signed char __builtin_vec_perm (vector signed char a1, vector signed char a2,
                             vector unsigned char a3);
extern vector unsigned char __builtin_vec_perm (vector unsigned char a1,
                               vector unsigned char a2,
                               vector unsigned char a3);
#else
__coverity_generic_altivec_vector __builtin_vec_packs ();
__coverity_generic_altivec_vector __builtin_vec_packsu ();
__coverity_generic_altivec_vector __builtin_vec_perm ();
#endif

extern vector float __builtin_vec_re (vector float a1);

#ifdef __cplusplus
extern vector signed char __builtin_vec_rl (vector signed char a1, vector unsigned char a2);
extern vector unsigned char __builtin_vec_rl (vector unsigned char a1,
                             vector unsigned char a2);
extern vector signed short __builtin_vec_rl (vector signed short a1, vector unsigned short a2);

extern vector unsigned short __builtin_vec_rl (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_rl (vector signed int a1, vector unsigned int a2);
extern vector unsigned int __builtin_vec_rl (vector unsigned int a1, vector unsigned int a2);
#else
__coverity_generic_altivec_vector __builtin_vec_rl ();
#endif

extern vector float __builtin_vec_round (vector float a1);

extern vector float __builtin_vec_rsqrte (vector float a1);

#ifdef __cplusplus
extern vector float __builtin_vec_sel (vector float a1, vector float a2, vector signed int a3);
extern vector float __builtin_vec_sel (vector float a1, vector float a2, vector unsigned int a3);
extern vector signed int __builtin_vec_sel (vector signed int a1, vector signed int a2,
                           vector signed int a3);
extern vector signed int __builtin_vec_sel (vector signed int a1, vector signed int a2,
                           vector unsigned int a3);
extern vector unsigned int __builtin_vec_sel (vector unsigned int a1, vector unsigned int a2,
                             vector signed int a3);
extern vector unsigned int __builtin_vec_sel (vector unsigned int a1, vector unsigned int a2,
                             vector unsigned int a3);
extern vector signed short __builtin_vec_sel (vector signed short a1, vector signed short a2,
                             vector signed short a3);
extern vector signed short __builtin_vec_sel (vector signed short a1, vector signed short a2,
                             vector unsigned short a3);
extern vector unsigned short __builtin_vec_sel (vector unsigned short a1,
                               vector unsigned short a2,
                               vector signed short a3);
extern vector unsigned short __builtin_vec_sel (vector unsigned short a1,
                               vector unsigned short a2,
                               vector unsigned short a3);
extern vector signed char __builtin_vec_sel (vector signed char a1, vector signed char a2,
                            vector signed char a3);
extern vector signed char __builtin_vec_sel (vector signed char a1, vector signed char a2,
                            vector unsigned char a3);
extern vector unsigned char __builtin_vec_sel (vector unsigned char a1,
                              vector unsigned char a2,
                              vector signed char a3);
extern vector unsigned char __builtin_vec_sel (vector unsigned char a1,
                              vector unsigned char a2,
                              vector unsigned char a3);

extern vector signed char __builtin_vec_sl (vector signed char a1, vector unsigned char a2);
extern vector unsigned char __builtin_vec_sl (vector unsigned char a1,
                             vector unsigned char a2);
extern vector signed short __builtin_vec_sl (vector signed short a1, vector unsigned short a2);

extern vector unsigned short __builtin_vec_sl (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_sl (vector signed int a1, vector unsigned int a2);
extern vector unsigned int __builtin_vec_sl (vector unsigned int a1, vector unsigned int a2);

extern vector float __builtin_vec_sld (vector float a1, vector float a2, const char a3);
extern vector signed int __builtin_vec_sld (vector signed int a1, vector signed int a2,
                           const char a3);
extern vector unsigned int __builtin_vec_sld (vector unsigned int a1, vector unsigned int a2,
                             const char a3);
extern vector signed short __builtin_vec_sld (vector signed short a1, vector signed short a2,
                             const char a3);
extern vector unsigned short __builtin_vec_sld (vector unsigned short a1,
                               vector unsigned short a2, const char a3);
extern vector signed char __builtin_vec_sld (vector signed char a1, vector signed char a2,
                            const char a3);
extern vector unsigned char __builtin_vec_sld (vector unsigned char a1,
                              vector unsigned char a2,
                              const char a3);

extern vector signed int __builtin_vec_sll (vector signed int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_sll (vector signed int a1, vector unsigned short a2);
extern vector signed int __builtin_vec_sll (vector signed int a1, vector unsigned char a2);
extern vector unsigned int __builtin_vec_sll (vector unsigned int a1, vector unsigned int a2);
extern vector unsigned int __builtin_vec_sll (vector unsigned int a1,
                             vector unsigned short a2);
extern vector unsigned int __builtin_vec_sll (vector unsigned int a1, vector unsigned char a2);

extern vector signed short __builtin_vec_sll (vector signed short a1, vector unsigned int a2);
extern vector signed short __builtin_vec_sll (vector signed short a1,
                             vector unsigned short a2);
extern vector signed short __builtin_vec_sll (vector signed short a1, vector unsigned char a2);

extern vector unsigned short __builtin_vec_sll (vector unsigned short a1,
                               vector unsigned int a2);
extern vector unsigned short __builtin_vec_sll (vector unsigned short a1,
                               vector unsigned short a2);
extern vector unsigned short __builtin_vec_sll (vector unsigned short a1,
                               vector unsigned char a2);
extern vector signed char __builtin_vec_sll (vector signed char a1, vector unsigned int a2);
extern vector signed char __builtin_vec_sll (vector signed char a1, vector unsigned short a2);
extern vector signed char __builtin_vec_sll (vector signed char a1, vector unsigned char a2);
extern vector unsigned char __builtin_vec_sll (vector unsigned char a1,
                              vector unsigned int a2);
extern vector unsigned char __builtin_vec_sll (vector unsigned char a1,
                              vector unsigned short a2);
extern vector unsigned char __builtin_vec_sll (vector unsigned char a1,
                              vector unsigned char a2);

extern vector float __builtin_vec_slo (vector float a1, vector signed char a2);
extern vector float __builtin_vec_slo (vector float a1, vector unsigned char a2);
extern vector signed int __builtin_vec_slo (vector signed int a1, vector signed char a2);
extern vector signed int __builtin_vec_slo (vector signed int a1, vector unsigned char a2);
extern vector unsigned int __builtin_vec_slo (vector unsigned int a1, vector signed char a2);
extern vector unsigned int __builtin_vec_slo (vector unsigned int a1, vector unsigned char a2);

extern vector signed short __builtin_vec_slo (vector signed short a1, vector signed char a2);
extern vector signed short __builtin_vec_slo (vector signed short a1, vector unsigned char a2);

extern vector unsigned short __builtin_vec_slo (vector unsigned short a1,
                               vector signed char a2);
extern vector unsigned short __builtin_vec_slo (vector unsigned short a1,
                               vector unsigned char a2);
extern vector signed char __builtin_vec_slo (vector signed char a1, vector signed char a2);
extern vector signed char __builtin_vec_slo (vector signed char a1, vector unsigned char a2);
extern vector unsigned char __builtin_vec_slo (vector unsigned char a1, vector signed char a2);

extern vector unsigned char __builtin_vec_slo (vector unsigned char a1,
                              vector unsigned char a2);

extern vector signed char __builtin_vec_splat (vector signed char a1, const char a2);
extern vector unsigned char __builtin_vec_splat (vector unsigned char a1, const char a2);
extern vector signed short __builtin_vec_splat (vector signed short a1, const char a2);
extern vector unsigned short __builtin_vec_splat (vector unsigned short a1, const char a2);
extern vector float __builtin_vec_splat (vector float a1, const char a2);
extern vector signed int __builtin_vec_splat (vector signed int a1, const char a2);
extern vector unsigned int __builtin_vec_splat (vector unsigned int a1, const char a2);
#else
# define __builtin_vec_sl(a, b) ((a) << (b))
__coverity_generic_altivec_vector __builtin_vec_sel ();
__coverity_generic_altivec_vector __builtin_vec_sel ();
__coverity_generic_altivec_vector __builtin_vec_sld ();
__coverity_generic_altivec_vector __builtin_vec_sll ();
__coverity_generic_altivec_vector __builtin_vec_slo ();
#endif

extern vector signed char __builtin_vec_splat_s8 (const char a1);

extern vector signed short __builtin_vec_splat_s16 (const char a1);

extern vector signed int __builtin_vec_splat_s32 (const char a1);

extern vector unsigned char __builtin_vec_splat_u8 (const char a1);

extern vector unsigned short __builtin_vec_splat_u16 (const char a1);

extern vector unsigned int __builtin_vec_splat_u32 (const char a1);

#ifdef __cplusplus
extern vector signed char __builtin_vec_sr (vector signed char a1, vector unsigned char a2);
extern vector unsigned char __builtin_vec_sr (vector unsigned char a1,
                             vector unsigned char a2);
extern vector signed short __builtin_vec_sr (vector signed short a1, vector unsigned short a2);

extern vector unsigned short __builtin_vec_sr (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_sr (vector signed int a1, vector unsigned int a2);
extern vector unsigned int __builtin_vec_sr (vector unsigned int a1, vector unsigned int a2);

extern vector signed char __builtin_vec_sra (vector signed char a1, vector unsigned char a2);
extern vector unsigned char __builtin_vec_sra (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed short __builtin_vec_sra (vector signed short a1,
                             vector unsigned short a2);
extern vector unsigned short __builtin_vec_sra (vector unsigned short a1,
                               vector unsigned short a2);
extern vector signed int __builtin_vec_sra (vector signed int a1, vector unsigned int a2);
extern vector unsigned int __builtin_vec_sra (vector unsigned int a1, vector unsigned int a2);

extern vector signed int __builtin_vec_srl (vector signed int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_srl (vector signed int a1, vector unsigned short a2);
extern vector signed int __builtin_vec_srl (vector signed int a1, vector unsigned char a2);
extern vector unsigned int __builtin_vec_srl (vector unsigned int a1, vector unsigned int a2);
extern vector unsigned int __builtin_vec_srl (vector unsigned int a1,
                             vector unsigned short a2);
extern vector unsigned int __builtin_vec_srl (vector unsigned int a1, vector unsigned char a2);

extern vector signed short __builtin_vec_srl (vector signed short a1, vector unsigned int a2);
extern vector signed short __builtin_vec_srl (vector signed short a1,
                             vector unsigned short a2);
extern vector signed short __builtin_vec_srl (vector signed short a1, vector unsigned char a2);

extern vector unsigned short __builtin_vec_srl (vector unsigned short a1,
                               vector unsigned int a2);
extern vector unsigned short __builtin_vec_srl (vector unsigned short a1,
                               vector unsigned short a2);
extern vector unsigned short __builtin_vec_srl (vector unsigned short a1,
                               vector unsigned char a2);
extern vector signed char __builtin_vec_srl (vector signed char a1, vector unsigned int a2);
extern vector signed char __builtin_vec_srl (vector signed char a1, vector unsigned short a2);
extern vector signed char __builtin_vec_srl (vector signed char a1, vector unsigned char a2);
extern vector unsigned char __builtin_vec_srl (vector unsigned char a1,
                              vector unsigned int a2);
extern vector unsigned char __builtin_vec_srl (vector unsigned char a1,
                              vector unsigned short a2);
extern vector unsigned char __builtin_vec_srl (vector unsigned char a1,
                              vector unsigned char a2);

extern vector float __builtin_vec_sro (vector float a1, vector signed char a2);
extern vector float __builtin_vec_sro (vector float a1, vector unsigned char a2);
extern vector signed int __builtin_vec_sro (vector signed int a1, vector signed char a2);
extern vector signed int __builtin_vec_sro (vector signed int a1, vector unsigned char a2);
extern vector unsigned int __builtin_vec_sro (vector unsigned int a1, vector signed char a2);
extern vector unsigned int __builtin_vec_sro (vector unsigned int a1, vector unsigned char a2);

extern vector signed short __builtin_vec_sro (vector signed short a1, vector signed char a2);
extern vector signed short __builtin_vec_sro (vector signed short a1, vector unsigned char a2);

extern vector unsigned short __builtin_vec_sro (vector unsigned short a1,
                               vector signed char a2);
extern vector unsigned short __builtin_vec_sro (vector unsigned short a1,
                               vector unsigned char a2);
extern vector signed char __builtin_vec_sro (vector signed char a1, vector signed char a2);
extern vector signed char __builtin_vec_sro (vector signed char a1, vector unsigned char a2);
extern vector unsigned char __builtin_vec_sro (vector unsigned char a1, vector signed char a2);

extern vector unsigned char __builtin_vec_sro (vector unsigned char a1,
                              vector unsigned char a2);

extern void __builtin_vec_st (vector float a1, int a2, const void * a3);
extern void __builtin_vec_st (vector signed int a1, int a2, const void * a3);
extern void __builtin_vec_st (vector unsigned int a1, int a2, const void * a3);
extern void __builtin_vec_st (vector signed short a1, int a2, const void * a3);
extern void __builtin_vec_st (vector unsigned short a1, int a2, const void * a3);
extern void __builtin_vec_st (vector signed char a1, int a2, const void * a3);
extern void __builtin_vec_st (vector unsigned char a1, int a2, const void * a3);

extern void __builtin_vec_ste (vector signed char a1, int a2, const void * a3);
extern void __builtin_vec_ste (vector unsigned char a1, int a2, unsigned char * a3);
extern void __builtin_vec_ste (vector signed short a1, int a2, const void * a3);
extern void __builtin_vec_ste (vector unsigned short a1, int a2, const void * a3);
extern void __builtin_vec_ste (vector signed int a1, int a2, const void * a3);
extern void __builtin_vec_ste (vector unsigned int a1, int a2, unsigned int * a3);
extern void __builtin_vec_ste (vector float a1, int a2, float * a3);

extern void __builtin_vec_stl (vector float a1, int a2, const void * a3);
extern void __builtin_vec_stl (vector signed int a1, int a2, const void * a3);
extern void __builtin_vec_stl (vector unsigned int a1, int a2, const void * a3);
extern void __builtin_vec_stl (vector signed short a1, int a2, const void * a3);
extern void __builtin_vec_stl (vector unsigned short a1, int a2, const void * a3);
extern void __builtin_vec_stl (vector signed char a1, int a2, const void * a3);
extern void __builtin_vec_stl (vector unsigned char a1, int a2, const void * a3);

extern vector signed char __builtin_vec_sub (vector signed char a1, vector signed char a2);
extern vector unsigned char __builtin_vec_sub (vector signed char a1, vector unsigned char a2);

extern vector unsigned char __builtin_vec_sub (vector unsigned char a1, vector signed char a2);

extern vector unsigned char __builtin_vec_sub (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed short __builtin_vec_sub (vector signed short a1, vector signed short a2);
extern vector unsigned short __builtin_vec_sub (vector signed short a1,
                               vector unsigned short a2);
extern vector unsigned short __builtin_vec_sub (vector unsigned short a1,
                               vector signed short a2);
extern vector unsigned short __builtin_vec_sub (vector unsigned short a1,
                               vector unsigned short a2);
extern vector signed int __builtin_vec_sub (vector signed int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_sub (vector signed int a1, vector unsigned int a2);
extern vector unsigned int __builtin_vec_sub (vector unsigned int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_sub (vector unsigned int a1, vector unsigned int a2);
extern vector float __builtin_vec_sub (vector float a1, vector float a2);

extern vector unsigned int __builtin_vec_subc (vector unsigned int a1, vector unsigned int a2);

extern vector unsigned char __builtin_vec_subs (vector signed char a1,
                               vector unsigned char a2);
extern vector unsigned char __builtin_vec_subs (vector unsigned char a1,
                               vector signed char a2);
extern vector unsigned char __builtin_vec_subs (vector unsigned char a1,
                               vector unsigned char a2);
extern vector signed char __builtin_vec_subs (vector signed char a1, vector signed char a2);
extern vector unsigned short __builtin_vec_subs (vector signed short a1,
                                vector unsigned short a2);
extern vector unsigned short __builtin_vec_subs (vector unsigned short a1,
                                vector signed short a2);
extern vector unsigned short __builtin_vec_subs (vector unsigned short a1,
                                vector unsigned short a2);
extern vector signed short __builtin_vec_subs (vector signed short a1, vector signed short a2);

extern vector unsigned int __builtin_vec_subs (vector signed int a1, vector unsigned int a2);
extern vector unsigned int __builtin_vec_subs (vector unsigned int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_subs (vector unsigned int a1, vector unsigned int a2);

extern vector signed int __builtin_vec_subs (vector signed int a1, vector signed int a2);

extern vector unsigned int __builtin_vec_sum4s (vector unsigned char a1,
                               vector unsigned int a2);
extern vector signed int __builtin_vec_sum4s (vector signed char a1, vector signed int a2);
extern vector signed int __builtin_vec_sum4s (vector signed short a1, vector signed int a2);
#else
# define __builtin_vec_sr(a, b) ((a) >> (b))
# define __builtin_vec_sub(a, b) ((a) - (b))
__coverity_generic_altivec_vector __builtin_vec_sra ();
__coverity_generic_altivec_vector __builtin_vec_srl ();
__coverity_generic_altivec_vector __builtin_vec_sro ();
extern void __builtin_vec_st ();
extern void __builtin_vec_ste ();
extern void __builtin_vec_stl ();
__coverity_generic_altivec_vector __builtin_vec_subc ();
__coverity_generic_altivec_vector __builtin_vec_subs ();
__coverity_generic_altivec_vector __builtin_vec_sum4s ();
#endif

extern vector signed int __builtin_vec_sum2s (vector signed int a1, vector signed int a2);

extern vector signed int __builtin_vec_sums (vector signed int a1, vector signed int a2);

extern vector float __builtin_vec_trunc (vector float a1);

#ifdef __cplusplus
extern vector signed short __builtin_vec_unpackh (vector signed char a1);
extern vector unsigned int __builtin_vec_unpackh (vector unsigned short a1);
extern vector signed int __builtin_vec_unpackh (vector signed short a1);

extern vector signed short __builtin_vec_unpackl (vector signed char a1);
extern vector unsigned int __builtin_vec_unpackl (vector unsigned short a1);
extern vector signed int __builtin_vec_unpackl (vector signed short a1);

extern vector float __builtin_vec_xor (vector float a1, vector float a2);
extern vector float __builtin_vec_xor (vector float a1, vector signed int a2);
extern vector float __builtin_vec_xor (vector signed int a1, vector float a2);
extern vector signed int __builtin_vec_xor (vector signed int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_xor (vector signed int a1, vector unsigned int a2);
extern vector unsigned int __builtin_vec_xor (vector unsigned int a1, vector signed int a2);
extern vector unsigned int __builtin_vec_xor (vector unsigned int a1, vector unsigned int a2);
extern vector signed short __builtin_vec_xor (vector signed short a1, vector signed short a2);
extern vector unsigned short __builtin_vec_xor (vector signed short a1,
                               vector unsigned short a2);
extern vector unsigned short __builtin_vec_xor (vector unsigned short a1,
                               vector signed short a2);
extern vector unsigned short __builtin_vec_xor (vector unsigned short a1,
                               vector unsigned short a2);
extern vector signed char __builtin_vec_xor (vector signed char a1, vector signed char a2);
extern vector unsigned char __builtin_vec_xor (vector signed char a1, vector unsigned char a2);

extern vector unsigned char __builtin_vec_xor (vector unsigned char a1, vector signed char a2);

extern vector unsigned char __builtin_vec_xor (vector unsigned char a1,
                              vector unsigned char a2);

extern vector signed int __builtin_vec_all_eq (vector signed char a1, vector unsigned char a2);

extern vector signed int __builtin_vec_all_eq (vector signed char a1, vector signed char a2);
extern vector signed int __builtin_vec_all_eq (vector unsigned char a1, vector signed char a2);

extern vector signed int __builtin_vec_all_eq (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed int __builtin_vec_all_eq (vector signed short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_all_eq (vector signed short a1, vector signed short a2);

extern vector signed int __builtin_vec_all_eq (vector unsigned short a1,
                              vector signed short a2);
extern vector signed int __builtin_vec_all_eq (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_all_eq (vector signed int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_all_eq (vector signed int a1, vector signed int a2);
extern vector signed int __builtin_vec_all_eq (vector unsigned int a1, vector signed int a2);
extern vector signed int __builtin_vec_all_eq (vector unsigned int a1, vector unsigned int a2);

extern vector signed int __builtin_vec_all_eq (vector float a1, vector float a2);

extern vector signed int __builtin_vec_all_ge (vector signed char a1, vector unsigned char a2);

extern vector signed int __builtin_vec_all_ge (vector unsigned char a1, vector signed char a2);

extern vector signed int __builtin_vec_all_ge (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed int __builtin_vec_all_ge (vector signed char a1, vector signed char a2);
extern vector signed int __builtin_vec_all_ge (vector signed short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_all_ge (vector unsigned short a1,
                              vector signed short a2);
extern vector signed int __builtin_vec_all_ge (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_all_ge (vector signed short a1, vector signed short a2);

extern vector signed int __builtin_vec_all_ge (vector signed int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_all_ge (vector unsigned int a1, vector signed int a2);
extern vector signed int __builtin_vec_all_ge (vector unsigned int a1, vector unsigned int a2);

extern vector signed int __builtin_vec_all_ge (vector signed int a1, vector signed int a2);
extern vector signed int __builtin_vec_all_ge (vector float a1, vector float a2);

extern vector signed int __builtin_vec_all_gt (vector signed char a1, vector unsigned char a2);

extern vector signed int __builtin_vec_all_gt (vector unsigned char a1, vector signed char a2);

extern vector signed int __builtin_vec_all_gt (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed int __builtin_vec_all_gt (vector signed char a1, vector signed char a2);
extern vector signed int __builtin_vec_all_gt (vector signed short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_all_gt (vector unsigned short a1,
                              vector signed short a2);
extern vector signed int __builtin_vec_all_gt (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_all_gt (vector signed short a1, vector signed short a2);

extern vector signed int __builtin_vec_all_gt (vector signed int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_all_gt (vector unsigned int a1, vector signed int a2);
extern vector signed int __builtin_vec_all_gt (vector unsigned int a1, vector unsigned int a2);

extern vector signed int __builtin_vec_all_gt (vector signed int a1, vector signed int a2);
extern vector signed int __builtin_vec_all_gt (vector float a1, vector float a2);

extern vector signed int __builtin_vec_all_in (vector float a1, vector float a2);

extern vector signed int __builtin_vec_all_le (vector signed char a1, vector unsigned char a2);

extern vector signed int __builtin_vec_all_le (vector unsigned char a1, vector signed char a2);

extern vector signed int __builtin_vec_all_le (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed int __builtin_vec_all_le (vector signed char a1, vector signed char a2);
extern vector signed int __builtin_vec_all_le (vector signed short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_all_le (vector unsigned short a1,
                              vector signed short a2);
extern vector signed int __builtin_vec_all_le (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_all_le (vector signed short a1, vector signed short a2);

extern vector signed int __builtin_vec_all_le (vector signed int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_all_le (vector unsigned int a1, vector signed int a2);
extern vector signed int __builtin_vec_all_le (vector unsigned int a1, vector unsigned int a2);

extern vector signed int __builtin_vec_all_le (vector signed int a1, vector signed int a2);
extern vector signed int __builtin_vec_all_le (vector float a1, vector float a2);

extern vector signed int __builtin_vec_all_lt (vector signed char a1, vector unsigned char a2);

extern vector signed int __builtin_vec_all_lt (vector unsigned char a1, vector signed char a2);

extern vector signed int __builtin_vec_all_lt (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed int __builtin_vec_all_lt (vector signed char a1, vector signed char a2);
extern vector signed int __builtin_vec_all_lt (vector signed short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_all_lt (vector unsigned short a1,
                              vector signed short a2);
extern vector signed int __builtin_vec_all_lt (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_all_lt (vector signed short a1, vector signed short a2);

extern vector signed int __builtin_vec_all_lt (vector signed int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_all_lt (vector unsigned int a1, vector signed int a2);
extern vector signed int __builtin_vec_all_lt (vector unsigned int a1, vector unsigned int a2);

extern vector signed int __builtin_vec_all_lt (vector signed int a1, vector signed int a2);
extern vector signed int __builtin_vec_all_lt (vector float a1, vector float a2);
#else
__coverity_generic_altivec_vector __builtin_vec_unpackh ();
__coverity_generic_altivec_vector __builtin_vec_unpackl ();
#define __builtin_vec_xor(a1, a2) ((a1) ^ (a2))
__coverity_generic_altivec_vector __builtin_vec_all_eq ();
__coverity_generic_altivec_vector __builtin_vec_all_ge ();
__coverity_generic_altivec_vector __builtin_vec_all_gt ();
__coverity_generic_altivec_vector __builtin_vec_all_le ();
__coverity_generic_altivec_vector __builtin_vec_all_lt ();
#endif

extern vector signed int __builtin_vec_all_nan (vector float a1);

#ifdef __cplusplus
extern vector signed int __builtin_vec_all_ne (vector signed char a1, vector unsigned char a2);

extern vector signed int __builtin_vec_all_ne (vector signed char a1, vector signed char a2);
extern vector signed int __builtin_vec_all_ne (vector unsigned char a1, vector signed char a2);

extern vector signed int __builtin_vec_all_ne (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed int __builtin_vec_all_ne (vector signed short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_all_ne (vector signed short a1, vector signed short a2);

extern vector signed int __builtin_vec_all_ne (vector unsigned short a1,
                              vector signed short a2);
extern vector signed int __builtin_vec_all_ne (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_all_ne (vector signed int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_all_ne (vector signed int a1, vector signed int a2);
extern vector signed int __builtin_vec_all_ne (vector unsigned int a1, vector signed int a2);
extern vector signed int __builtin_vec_all_ne (vector unsigned int a1, vector unsigned int a2);

extern vector signed int __builtin_vec_all_ne (vector float a1, vector float a2);

#else
__coverity_generic_altivec_vector __builtin_vec_all_ne ();
#endif

extern vector signed int __builtin_vec_all_nge (vector float a1, vector float a2);

extern vector signed int __builtin_vec_all_ngt (vector float a1, vector float a2);

extern vector signed int __builtin_vec_all_nle (vector float a1, vector float a2);

extern vector signed int __builtin_vec_all_nlt (vector float a1, vector float a2);

extern vector signed int __builtin_vec_all_numeric (vector float a1);

#ifdef __cplusplus
extern vector signed int __builtin_vec_any_eq (vector signed char a1, vector unsigned char a2);

extern vector signed int __builtin_vec_any_eq (vector signed char a1, vector signed char a2);
extern vector signed int __builtin_vec_any_eq (vector unsigned char a1, vector signed char a2);

extern vector signed int __builtin_vec_any_eq (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed int __builtin_vec_any_eq (vector signed short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_any_eq (vector signed short a1, vector signed short a2);

extern vector signed int __builtin_vec_any_eq (vector unsigned short a1,
                              vector signed short a2);
extern vector signed int __builtin_vec_any_eq (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_any_eq (vector signed int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_any_eq (vector signed int a1, vector signed int a2);
extern vector signed int __builtin_vec_any_eq (vector unsigned int a1, vector signed int a2);
extern vector signed int __builtin_vec_any_eq (vector unsigned int a1, vector unsigned int a2);

extern vector signed int __builtin_vec_any_eq (vector float a1, vector float a2);

extern vector signed int __builtin_vec_any_ge (vector signed char a1, vector unsigned char a2);

extern vector signed int __builtin_vec_any_ge (vector unsigned char a1, vector signed char a2);

extern vector signed int __builtin_vec_any_ge (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed int __builtin_vec_any_ge (vector signed char a1, vector signed char a2);
extern vector signed int __builtin_vec_any_ge (vector signed short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_any_ge (vector unsigned short a1,
                              vector signed short a2);
extern vector signed int __builtin_vec_any_ge (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_any_ge (vector signed short a1, vector signed short a2);

extern vector signed int __builtin_vec_any_ge (vector signed int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_any_ge (vector unsigned int a1, vector signed int a2);
extern vector signed int __builtin_vec_any_ge (vector unsigned int a1, vector unsigned int a2);

extern vector signed int __builtin_vec_any_ge (vector signed int a1, vector signed int a2);
extern vector signed int __builtin_vec_any_ge (vector float a1, vector float a2);

extern vector signed int __builtin_vec_any_gt (vector signed char a1, vector unsigned char a2);

extern vector signed int __builtin_vec_any_gt (vector unsigned char a1, vector signed char a2);

extern vector signed int __builtin_vec_any_gt (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed int __builtin_vec_any_gt (vector signed char a1, vector signed char a2);
extern vector signed int __builtin_vec_any_gt (vector signed short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_any_gt (vector unsigned short a1,
                              vector signed short a2);
extern vector signed int __builtin_vec_any_gt (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_any_gt (vector signed short a1, vector signed short a2);

extern vector signed int __builtin_vec_any_gt (vector signed int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_any_gt (vector unsigned int a1, vector signed int a2);
extern vector signed int __builtin_vec_any_gt (vector unsigned int a1, vector unsigned int a2);

extern vector signed int __builtin_vec_any_gt (vector signed int a1, vector signed int a2);
extern vector signed int __builtin_vec_any_gt (vector float a1, vector float a2);

extern vector signed int __builtin_vec_any_le (vector signed char a1, vector unsigned char a2);

extern vector signed int __builtin_vec_any_le (vector unsigned char a1, vector signed char a2);

extern vector signed int __builtin_vec_any_le (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed int __builtin_vec_any_le (vector signed char a1, vector signed char a2);
extern vector signed int __builtin_vec_any_le (vector signed short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_any_le (vector unsigned short a1,
                              vector signed short a2);
extern vector signed int __builtin_vec_any_le (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_any_le (vector signed short a1, vector signed short a2);

extern vector signed int __builtin_vec_any_le (vector signed int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_any_le (vector unsigned int a1, vector signed int a2);
extern vector signed int __builtin_vec_any_le (vector unsigned int a1, vector unsigned int a2);

extern vector signed int __builtin_vec_any_le (vector signed int a1, vector signed int a2);
extern vector signed int __builtin_vec_any_le (vector float a1, vector float a2);

extern vector signed int __builtin_vec_any_lt (vector signed char a1, vector unsigned char a2);

extern vector signed int __builtin_vec_any_lt (vector unsigned char a1, vector signed char a2);

extern vector signed int __builtin_vec_any_lt (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed int __builtin_vec_any_lt (vector signed char a1, vector signed char a2);
extern vector signed int __builtin_vec_any_lt (vector signed short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_any_lt (vector unsigned short a1,
                              vector signed short a2);
extern vector signed int __builtin_vec_any_lt (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_any_lt (vector signed short a1, vector signed short a2);

extern vector signed int __builtin_vec_any_lt (vector signed int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_any_lt (vector unsigned int a1, vector signed int a2);
extern vector signed int __builtin_vec_any_lt (vector unsigned int a1, vector unsigned int a2);

extern vector signed int __builtin_vec_any_lt (vector signed int a1, vector signed int a2);
extern vector signed int __builtin_vec_any_lt (vector float a1, vector float a2);
#else
__coverity_generic_altivec_vector __builtin_vec_any_eq ();
__coverity_generic_altivec_vector __builtin_vec_any_ge ();
__coverity_generic_altivec_vector __builtin_vec_any_gt ();
__coverity_generic_altivec_vector __builtin_vec_any_le ();
__coverity_generic_altivec_vector __builtin_vec_any_lt ();
#endif

extern vector signed int __builtin_vec_any_nan (vector float a1);

#ifdef __cplusplus
extern vector signed int __builtin_vec_any_ne (vector signed char a1, vector unsigned char a2);

extern vector signed int __builtin_vec_any_ne (vector signed char a1, vector signed char a2);
extern vector signed int __builtin_vec_any_ne (vector unsigned char a1, vector signed char a2);

extern vector signed int __builtin_vec_any_ne (vector unsigned char a1,
                              vector unsigned char a2);
extern vector signed int __builtin_vec_any_ne (vector signed short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_any_ne (vector signed short a1, vector signed short a2);

extern vector signed int __builtin_vec_any_ne (vector unsigned short a1,
                              vector signed short a2);
extern vector signed int __builtin_vec_any_ne (vector unsigned short a1,
                              vector unsigned short a2);
extern vector signed int __builtin_vec_any_ne (vector signed int a1, vector unsigned int a2);
extern vector signed int __builtin_vec_any_ne (vector signed int a1, vector signed int a2);
extern vector signed int __builtin_vec_any_ne (vector unsigned int a1, vector signed int a2);
extern vector signed int __builtin_vec_any_ne (vector unsigned int a1, vector unsigned int a2);

extern vector signed int __builtin_vec_any_ne (vector float a1, vector float a2);
#else
__coverity_generic_altivec_vector __builtin_vec_any_ne ();
#endif

extern vector signed int __builtin_vec_any_nge (vector float a1, vector float a2);

extern vector signed int __builtin_vec_any_ngt (vector float a1, vector float a2);

extern vector signed int __builtin_vec_any_nle (vector float a1, vector float a2);

extern vector signed int __builtin_vec_any_nlt (vector float a1, vector float a2);

extern vector signed int __builtin_vec_any_numeric (vector float a1);

extern vector signed int __builtin_vec_any_out (vector float a1, vector float a2);

# ifndef OFFSET_T
#  ifdef _ARCH_PPC64
#   define OFFSET_T long
#  else
#   define OFFSET_T int
# endif
# endif

extern int __builtin_altivec_vcmpbfp_p(int a1, vector float a2, vector float a3);
extern int __builtin_altivec_vcmpeqfp_p(int a1, vector float a2, vector float a3);
extern int __builtin_altivec_vcmpeqfp_p(int, vector float a1, vector float a2);
extern int __builtin_altivec_vcmpequb_p(int a1, vector signed char a2, vector signed char a3);
extern int __builtin_altivec_vcmpequh_p(int a1, vector signed short a2, vector signed short a3);
extern int __builtin_altivec_vcmpequw_p(int a1, vector signed int a2, vector signed int a3);
extern int __builtin_altivec_vcmpgefp_p(int a1, vector float a2, vector float a3);
extern int __builtin_altivec_vcmpgtfp_p(int a1, vector float a2, vector float a3);
extern int __builtin_altivec_vcmpgtsb_p(int a1, vector signed char a2, vector signed char a3);
extern int __builtin_altivec_vcmpgtsh_p(int a1, vector signed short a2, vector signed short a3);
extern int __builtin_altivec_vcmpgtsw_p(int a1, vector signed int a2, vector signed int a3);
extern int __builtin_altivec_vcmpgtub_p(int a1, vector signed char a2, vector signed char a3);
extern int __builtin_altivec_vcmpgtuh_p(int a1, vector signed short a2, vector signed short a3);
extern int __builtin_altivec_vcmpgtuw_p(int a1, vector signed int a2, vector signed int a3);
extern vector bool char __builtin_altivec_lvx(OFFSET_T a1, const void * a2);
extern vector bool char __builtin_altivec_lvxl(OFFSET_T a1, const void * a2);
extern vector bool char __builtin_altivec_vand(vector signed int a1, vector signed int a2);
extern vector bool char __builtin_altivec_vandc(vector signed int a1, vector signed int a2);
extern vector bool char __builtin_altivec_vcmpequb(vector signed char a1, vector signed char a2);
extern vector bool char __builtin_altivec_vcmpgtsb(vector signed char a1, vector signed char a2);
extern vector bool char __builtin_altivec_vcmpgtub(vector signed char a1, vector signed char a2);
extern vector bool char __builtin_altivec_vmrghb(vector signed char a1, vector signed char a2);
extern vector bool char __builtin_altivec_vmrglb(vector signed char a1, vector signed char a2);
extern vector bool char __builtin_altivec_vnor(vector signed int a1, vector signed int a2);
extern vector bool char __builtin_altivec_vor(vector signed int a1, vector signed int a2);
extern vector bool char __builtin_altivec_vperm_4si(vector signed int a1, vector signed int a2, vector signed char a3);
extern vector bool char __builtin_altivec_vpkuhum(vector signed short a1, vector signed short a2);
extern vector bool char __builtin_altivec_vsel_4si(vector signed int a1, vector signed int a2, vector signed int a3);
extern vector bool char __builtin_altivec_vsl(vector signed int a1, vector signed int a2);
extern vector bool char __builtin_altivec_vsldoi_4si(vector signed int a1, vector signed int a2, const int a3);
extern vector bool char __builtin_altivec_vsldoi_4sf(vector float a1, vector float a2, const int a3);
extern vector bool char __builtin_altivec_vspltb(vector signed char a1, const int a2);
extern vector bool char __builtin_altivec_vsr(vector signed int a1, vector signed int a2);
extern vector bool char __builtin_altivec_vxor(vector signed int a1, vector signed int a2);
extern vector bool int __builtin_altivec_vcmpeqfp(vector float a1, vector float a2);
extern vector bool int __builtin_altivec_vcmpequw(vector signed int a1, vector signed int a2);
extern vector bool int __builtin_altivec_vcmpgefp(vector float a1, vector float a2);
extern vector bool int __builtin_altivec_vcmpgtfp(vector float a1, vector float a2);
extern vector bool int __builtin_altivec_vcmpgtsw(vector signed int a1, vector signed int a2);
extern vector bool int __builtin_altivec_vcmpgtuw(vector signed int a1, vector signed int a2);
extern vector bool int __builtin_altivec_vmrghw(vector signed int a1, vector signed int a2);
extern vector bool int __builtin_altivec_vmrglw(vector signed int a1, vector signed int a2);
extern vector bool int __builtin_altivec_vspltw(vector signed int a1, const int a2);
extern vector bool int __builtin_altivec_vupkhsh(vector signed short a1);
extern vector bool int __builtin_altivec_vupklsh(vector signed short a1);
extern vector bool short __builtin_altivec_vcmpequh(vector signed short a1, vector signed short a2);
extern vector bool short __builtin_altivec_vcmpgtsh(vector signed short a1, vector signed short a2);
extern vector bool short __builtin_altivec_vcmpgtuh(vector signed short a1, vector signed short a2);
extern vector bool short __builtin_altivec_vmrghh(vector signed short a1, vector signed short a2);
extern vector bool short __builtin_altivec_vmrglh(vector signed short a1, vector signed short a2);
extern vector bool short __builtin_altivec_vpkuwum(vector signed int a1, vector signed int a2);
extern vector bool short __builtin_altivec_vsplth(vector signed short a1, const int a2);
extern vector bool short __builtin_altivec_vupkhsb(vector signed char a1);
extern vector bool short __builtin_altivec_vupklsb(vector signed char a1);
extern vector float __builtin_altivec_abs_v4sf(vector float a1);
extern vector float __builtin_altivec_lvewx(OFFSET_T a1, const void * a2);
extern vector float __builtin_altivec_vaddfp(vector float a1, vector float a2);
extern vector float __builtin_altivec_vcfsx(vector signed int a1, const int a2);
extern vector float __builtin_altivec_vcfux(vector signed int a1, const int a2);
extern vector float __builtin_altivec_vexptefp(vector float a1);
extern vector float __builtin_altivec_vlogefp(vector float a1);
extern vector float __builtin_altivec_vmaddfp(vector float a1, vector float a2, vector float a3);
extern vector float __builtin_altivec_vmaxfp(vector float a1, vector float a2);
extern vector float __builtin_altivec_vminfp(vector float a1, vector float a2);
extern vector float __builtin_altivec_vnmsubfp(vector float a1, vector float a2, vector float a3);
extern vector float __builtin_altivec_vrefp(vector float a1);
extern vector float __builtin_altivec_vrfim(vector float a1);
extern vector float __builtin_altivec_vrfin(vector float a1);
extern vector float __builtin_altivec_vrfip(vector float a1);
extern vector float __builtin_altivec_vrfiz(vector float a1);
extern vector float __builtin_altivec_vrsqrtefp(vector float a1);
extern vector float __builtin_altivec_vslo(vector signed int a1, vector signed int a2);
extern vector float __builtin_altivec_vsro(vector signed int a1, vector signed int a2);
extern vector float __builtin_altivec_vsubfp(vector float a1, vector float a2);
extern vector pixel __builtin_altivec_vpkpx(vector signed int a1, vector signed int a2);
extern vector signed char __builtin_altivec_abs_v16qi(vector signed char a1);
extern vector signed char __builtin_altivec_abss_v16qi(vector signed char a1);
extern vector signed char __builtin_altivec_lvebx(OFFSET_T a1, const void * a2);
extern vector signed char __builtin_altivec_vaddsbs(vector signed char a1, vector signed char a2);
extern vector signed char __builtin_altivec_vaddubm(vector signed char a1, vector signed char a2);
extern vector signed char __builtin_altivec_vavgsb(vector signed char a1, vector signed char a2);
extern vector signed char __builtin_altivec_vmaxsb(vector signed char a1, vector signed char a2);
extern vector signed char __builtin_altivec_vminsb(vector signed char a1, vector signed char a2);
extern vector signed char __builtin_altivec_vpkshss(vector signed short a1, vector signed short a2);
extern vector signed char __builtin_altivec_vrlb(vector signed char a1, vector signed char a2);
extern vector signed char __builtin_altivec_vslb(vector signed char a1, vector signed char a2);
extern vector signed char __builtin_altivec_vspltisb(const int a1);
extern vector signed char __builtin_altivec_vsrab(vector signed char a1, vector signed char a2);
extern vector signed char __builtin_altivec_vsrb(vector signed char a1, vector signed char a2);
extern vector signed char __builtin_altivec_vsubsbs(vector signed char a1, vector signed char a2);
extern vector signed char __builtin_altivec_vsububm(vector signed char a1, vector signed char a2);
extern vector signed int __builtin_altivec_abs_v4si(vector signed int a1);
extern vector signed int __builtin_altivec_abss_v4si(vector signed int a1);
extern vector signed int __builtin_altivec_vaddsws(vector signed int a1, vector signed int a2);
extern vector signed int __builtin_altivec_vadduwm(vector signed int a1, vector signed int a2);
extern vector signed int __builtin_altivec_vavgsw(vector signed int a1, vector signed int a2);
extern vector signed int __builtin_altivec_vcmpbfp(vector float a1, vector float a2);
extern vector signed int __builtin_altivec_vctsxs(vector float a1, const int a2);
extern vector signed int __builtin_altivec_vmaxsw(vector signed int a1, vector signed int a2);
extern vector signed int __builtin_altivec_vminsw(vector signed int a1, vector signed int a2);
extern vector signed int __builtin_altivec_vmsummbm(vector signed char a1, vector signed char a2, vector signed int a3);
extern vector signed int __builtin_altivec_vmsumshm(vector signed short a1, vector signed short a2, vector signed int a3);
extern vector signed int __builtin_altivec_vmsumshs(vector signed short a1, vector signed short a2, vector signed int a3);
extern vector signed int __builtin_altivec_vmulesh(vector signed short a1, vector signed short a2);
extern vector signed int __builtin_altivec_vmulosh(vector signed short a1, vector signed short a2);
extern vector signed int __builtin_altivec_vrlw(vector signed int a1, vector signed int a2);
extern vector signed int __builtin_altivec_vslw(vector signed int a1, vector signed int a2);
extern vector signed int __builtin_altivec_vspltisw(const int a1);
extern vector signed int __builtin_altivec_vsraw(vector signed int a1, vector signed int a2);
extern vector signed int __builtin_altivec_vsrw(vector signed int a1, vector signed int a2);
extern vector signed int __builtin_altivec_vsubsws(vector signed int a1, vector signed int a2);
extern vector signed int __builtin_altivec_vsubuwm(vector signed int a1, vector signed int a2);
extern vector signed int __builtin_altivec_vsum2sws(vector signed int a1, vector signed int a2);
extern vector signed int __builtin_altivec_vsum4sbs(vector signed char a1, vector signed int a2);
extern vector signed int __builtin_altivec_vsum4shs(vector signed short a1, vector signed int a2);
extern vector signed int __builtin_altivec_vsumsws(vector signed int a1, vector signed int a2);
extern vector signed short __builtin_altivec_abs_v8hi(vector signed short a1);
extern vector signed short __builtin_altivec_abss_v8hi(vector signed short a1);
extern vector signed short __builtin_altivec_lvehx(OFFSET_T a1, const void * a2);
extern vector signed short __builtin_altivec_vaddshs(vector signed short a1, vector signed short a2);
extern vector signed short __builtin_altivec_vadduhm(vector signed short a1, vector signed short a2);
extern vector signed short __builtin_altivec_vavgsh(vector signed short a1, vector signed short a2);
extern vector signed short __builtin_altivec_vmaxsh(vector signed short a1, vector signed short a2);
extern vector signed short __builtin_altivec_vmhaddshs(vector signed short a1, vector signed short a2, vector signed short a3);
extern vector signed short __builtin_altivec_vmhraddshs(vector signed short a1, vector signed short a2, vector signed short a3);
extern vector signed short __builtin_altivec_vminsh(vector signed short a1, vector signed short a2);
extern vector signed short __builtin_altivec_vmladduhm(vector signed short a1, vector signed short a2, vector signed short a3);
extern vector signed short __builtin_altivec_vmulesb(vector signed char a1, vector signed char a2);
extern vector signed short __builtin_altivec_vmuleub(vector signed char a1, vector signed char a2);
extern vector signed short __builtin_altivec_vmulosb(vector signed char a1, vector signed char a2);
extern vector signed short __builtin_altivec_vpkswss(vector signed int a1, vector signed int a2);
extern vector signed short __builtin_altivec_vrlh(vector signed short a1, vector signed short a2);
extern vector signed short __builtin_altivec_vslh(vector signed short a1, vector signed short a2);
extern vector signed short __builtin_altivec_vspltish(const int a1);
extern vector signed short __builtin_altivec_vsrah(vector signed short a1, vector signed short a2);
extern vector signed short __builtin_altivec_vsrh(vector signed short a1, vector signed short a2);
extern vector signed short __builtin_altivec_vsubshs(vector signed short a1, vector signed short a2);
extern vector signed short __builtin_altivec_vsubuhm(vector signed short a1, vector signed short a2);
extern vector unsigned char __builtin_altivec_lvlx(OFFSET_T a1, const void * a2);
extern vector unsigned char __builtin_altivec_lvlxl(OFFSET_T a1, const void * a2);
extern vector unsigned char __builtin_altivec_lvrx(OFFSET_T a1, const void * a2);
extern vector unsigned char __builtin_altivec_lvrxl(OFFSET_T a1, const void * a2);
extern vector unsigned char __builtin_altivec_lvsl(OFFSET_T a1, const void * a2);
extern vector unsigned char __builtin_altivec_lvsr(OFFSET_T a1, const void * a2);
extern vector unsigned char __builtin_altivec_vaddubs(vector signed char a1, vector signed char a2);
extern vector unsigned char __builtin_altivec_vavgub(vector signed char a1, vector signed char a2);
extern vector unsigned char __builtin_altivec_vmaxub(vector signed char a1, vector signed char a2);
extern vector unsigned char __builtin_altivec_vminub(vector signed char a1, vector signed char a2);
extern vector unsigned char __builtin_altivec_vpkshus(vector signed short a1, vector signed short a2);
extern vector unsigned char __builtin_altivec_vpkuhus(vector signed short a1, vector signed short a2);
extern vector unsigned char __builtin_altivec_vsububs(vector signed char a1, vector signed char a2);
extern vector unsigned int __builtin_altivec_vaddcuw(vector signed int a1, vector signed int a2);
extern vector unsigned int __builtin_altivec_vadduws(vector signed int a1, vector signed int a2);
extern vector unsigned int __builtin_altivec_vavguw(vector signed int a1, vector signed int a2);
extern vector unsigned int __builtin_altivec_vctuxs(vector float a1, const int a2);
extern vector unsigned int __builtin_altivec_vmaxuw(vector signed int a1, vector signed int a2);
extern vector unsigned int __builtin_altivec_vminuw(vector signed int a1, vector signed int a2);
extern vector unsigned int __builtin_altivec_vmsumubm(vector signed char a1, vector signed char a2, vector signed int a3);
extern vector unsigned int __builtin_altivec_vmsumuhm(vector signed short a1, vector signed short a2, vector signed int a3);
extern vector unsigned int __builtin_altivec_vmsumuhs(vector signed short a1, vector signed short a2, vector signed int a3);
extern vector unsigned int __builtin_altivec_vmuleuh(vector signed short a1, vector signed short a2);
extern vector unsigned int __builtin_altivec_vmulouh(vector signed short a1, vector signed short a2);
extern vector unsigned int __builtin_altivec_vsubcuw(vector signed int a1, vector signed int a2);
extern vector unsigned int __builtin_altivec_vsubuws(vector signed int a1, vector signed int a2);
extern vector unsigned int __builtin_altivec_vsum4ubs(vector signed char a1, vector signed int a2);
extern vector unsigned int __builtin_altivec_vupkhpx(vector signed short a1);
extern vector unsigned int __builtin_altivec_vupklpx(vector signed short a1);
extern vector unsigned short __builtin_altivec_mfvscr();
extern vector unsigned short __builtin_altivec_vadduhs(vector signed short a1, vector signed short a2);
extern vector unsigned short __builtin_altivec_vavguh(vector signed short a1, vector signed short a2);
extern vector unsigned short __builtin_altivec_vmaxuh(vector signed short a1, vector signed short a2);
extern vector unsigned short __builtin_altivec_vminuh(vector signed short a1, vector signed short a2);
extern vector unsigned short __builtin_altivec_vmuloub(vector signed char a1, vector signed char a2);
extern vector unsigned short __builtin_altivec_vpkswus(vector signed int a1, vector signed int a2);
extern vector unsigned short __builtin_altivec_vpkuwus(vector signed int a1, vector signed int a2);
extern vector unsigned short __builtin_altivec_vsubuhs(vector signed short a1, vector signed short a2);
extern void __builtin_altivec_dss(int a1);
extern void __builtin_altivec_dssall();
extern void __builtin_altivec_dst(const void *a1, OFFSET_T a2, const int a3);
extern void __builtin_altivec_dstst(const void *a1, OFFSET_T a2, const int a3);
extern void __builtin_altivec_dststt(const void *a1, OFFSET_T a2, const int a3);
extern void __builtin_altivec_dstt(const void *a1, OFFSET_T a2, const int a3);
extern void __builtin_altivec_mtvscr(vector signed int a1);
#ifdef __cplusplus
extern void __builtin_altivec_stvebx(vector signed char a1, OFFSET_T a2, const void * a3);
extern void __builtin_altivec_stvehx(vector signed short a1, OFFSET_T a2, const void * a3);
extern void __builtin_altivec_stvewx(vector signed int a1, OFFSET_T a2, const void * a3);
extern void __builtin_altivec_stvlx(vector signed char a1, OFFSET_T a2, const void * a3);
extern void __builtin_altivec_stvlxl(vector signed char a1, OFFSET_T a2, const void * a3);
extern void __builtin_altivec_stvrx(vector signed char a1, OFFSET_T a2, const void * a3);
extern void __builtin_altivec_stvrxl(vector signed char a1, OFFSET_T a2, const void * a3);
extern void __builtin_altivec_stvx(vector signed int a1, OFFSET_T a2, const void * a3);
extern void __builtin_altivec_stvxl(vector signed int a1, OFFSET_T a2, const void * a3);
extern vector bool char __builtin_altivec_vsel_4sf(vector float a1, vector float a2, vector float a3);
extern vector bool char __builtin_altivec_vsel_4sf(vector float a1, vector float a2, vector signed int a3);
#else
extern void __builtin_altivec_stvebx();
extern void __builtin_altivec_stvehx();
extern void __builtin_altivec_stvewx();
extern void __builtin_altivec_stvlx();
extern void __builtin_altivec_stvlxl();
extern void __builtin_altivec_stvrx();
extern void __builtin_altivec_stvrxl();
extern void __builtin_altivec_stvx();
extern void __builtin_altivec_stvxl();
extern __coverity_generic_altivec_vector __builtin_altivec_vsel_4sf();
#endif
#endif /* __ALTIVEC__ */

//SPU intrinsic
#if defined(COVERITY_SPU_COMPILER_2_5) && !defined(__COVERITY_DISABLE_BUILTIN_DECLS)
#define __cov_qword __vector signed char
#define __cov_imm int
#define __cov_ra __cov_qword
#define __cov_rb __cov_qword
#define __cov_rc __cov_qword
#define __cov_rd __cov_qword
#define __cov_rt __cov_qword
#define __cov_scalar double 
#define __cov_count unsigned int
#define __cov_pos unsigned int

__cov_qword __builtin_si_lqd(__cov_ra,__cov_imm);
__cov_qword __builtin_si_lqx(__cov_ra,__cov_rb);
__cov_qword __builtin_si_lqa(__cov_imm);
__cov_qword __builtin_si_lqr(__cov_imm);
__cov_qword __builtin_si_stqd(__cov_rt,__cov_ra,__cov_imm);
__cov_qword __builtin_si_stqx(__cov_rt,__cov_ra,__cov_rb);
__cov_qword __builtin_si_stqa(__cov_rt,__cov_imm);
__cov_qword __builtin_si_stqr(__cov_rt,__cov_imm);
__cov_qword __builtin_si_cbd(__cov_ra,__cov_imm);
__cov_qword __builtin_si_cbx(__cov_ra,__cov_rb);
__cov_qword __builtin_si_chd(__cov_ra,__cov_imm);
__cov_qword __builtin_si_chx(__cov_ra,__cov_rb);
__cov_qword __builtin_si_cwd(__cov_ra,__cov_imm);
__cov_qword __builtin_si_cwx(__cov_ra,__cov_rb);
__cov_qword __builtin_si_cdd(__cov_ra,__cov_imm);
__cov_qword __builtin_si_cdx(__cov_ra,__cov_rb);
__cov_qword __builtin_si_ilh(__cov_imm);
__cov_qword __builtin_si_ilhu(__cov_imm);
__cov_qword __builtin_si_il(__cov_imm);
__cov_qword __builtin_si_ila(__cov_imm);
__cov_qword __builtin_si_iohl(__cov_ra,__cov_imm);
__cov_qword __builtin_si_fsmbi(__cov_imm);
__cov_qword __builtin_si_ah(__cov_ra,__cov_rb);
__cov_qword __builtin_si_ahi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_a(__cov_ra,__cov_rb);
__cov_qword __builtin_si_ai(__cov_ra,__cov_imm);
__cov_qword __builtin_si_addx(__cov_ra,__cov_rb,__cov_rt);
__cov_qword __builtin_si_cg(__cov_ra,__cov_rb);
__cov_qword __builtin_si_cgx(__cov_ra,__cov_rb,__cov_rt);
__cov_qword __builtin_si_sfh(__cov_ra,__cov_rb);
//__cov_qword __builtin_si_sfhi(__cov_imm,__cov_ra);
__cov_qword __builtin_si_sf(__cov_ra,__cov_rb);
__cov_qword __builtin_si_sfi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_sfx(__cov_ra,__cov_rb,__cov_rt);
__cov_qword __builtin_si_bg(__cov_ra,__cov_rb);
__cov_qword __builtin_si_bgx(__cov_ra,__cov_rb,__cov_rt);
__cov_qword __builtin_si_mpy(__cov_ra,__cov_rb);
__cov_qword __builtin_si_mpyu(__cov_ra,__cov_rb);
__cov_qword __builtin_si_mpyi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_mpyui(__cov_ra,__cov_imm);
__cov_qword __builtin_si_mpya(__cov_ra,__cov_rb,__cov_rc);
__cov_qword __builtin_si_mpyh(__cov_ra,__cov_rb);
__cov_qword __builtin_si_mpys(__cov_ra,__cov_rb);
__cov_qword __builtin_si_mpyhh(__cov_ra,__cov_rb);
__cov_qword __builtin_si_mpyhhu(__cov_ra,__cov_rb);
__cov_qword __builtin_si_mpyhha(__cov_ra,__cov_rb,__cov_rc);
__cov_qword __builtin_si_mpyhhau(__cov_ra,__cov_rb,__cov_rc);
__cov_qword __builtin_si_clz(__cov_ra);
__cov_qword __builtin_si_cntb(__cov_ra);
__cov_qword __builtin_si_fsmb(__cov_ra);
__cov_qword __builtin_si_fsmh(__cov_ra);
__cov_qword __builtin_si_fsm(__cov_ra);
__cov_qword __builtin_si_gbb(__cov_ra);
__cov_qword __builtin_si_gbh(__cov_ra);
__cov_qword __builtin_si_gb(__cov_ra);
__cov_qword __builtin_si_avgb(__cov_ra,__cov_rb);
__cov_qword __builtin_si_absdb(__cov_ra,__cov_rb);
__cov_qword __builtin_si_sumb(__cov_ra,__cov_rb);
__cov_qword __builtin_si_xsbh(__cov_ra);
__cov_qword __builtin_si_xshw(__cov_ra);
__cov_qword __builtin_si_xswd(__cov_ra);
__cov_qword __builtin_si_and(__cov_ra,__cov_rb);
__cov_qword __builtin_si_andc(__cov_ra,__cov_rb);
__cov_qword __builtin_si_andbi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_andhi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_andi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_or(__cov_ra,__cov_rb);
__cov_qword __builtin_si_orc(__cov_ra,__cov_rb);
__cov_qword __builtin_si_orbi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_orhi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_ori(__cov_ra,__cov_imm);
__cov_qword __builtin_si_orx(__cov_ra);
__cov_qword __builtin_si_xor(__cov_ra,__cov_rb);
__cov_qword __builtin_si_xorbi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_xorhi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_xori(__cov_ra,__cov_imm);
__cov_qword __builtin_si_nand(__cov_ra,__cov_rb);
__cov_qword __builtin_si_nor(__cov_ra,__cov_rb);
__cov_qword __builtin_si_eqv(__cov_ra,__cov_rb);
__cov_qword __builtin_si_selb(__cov_ra,__cov_rb,__cov_rc);
__cov_qword __builtin_si_shufb(__cov_ra,__cov_rb,__cov_rc);
__cov_qword __builtin_si_shlh(__cov_ra,__cov_rb);
__cov_qword __builtin_si_shlhi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_shl(__cov_ra,__cov_rb);
__cov_qword __builtin_si_shli(__cov_ra,__cov_imm);
__cov_qword __builtin_si_shlqbi(__cov_ra,__cov_rb);
__cov_qword __builtin_si_shlqbii(__cov_ra,__cov_imm);
__cov_qword __builtin_si_shlqby(__cov_ra,__cov_rb);
__cov_qword __builtin_si_shlqbyi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_shlqbybi(__cov_ra,__cov_rb);
__cov_qword __builtin_si_roth(__cov_ra,__cov_rb);
__cov_qword __builtin_si_rothi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_rot(__cov_ra,__cov_rb);
__cov_qword __builtin_si_roti(__cov_ra,__cov_imm);
__cov_qword __builtin_si_rotqby(__cov_ra,__cov_rb);
__cov_qword __builtin_si_rotqbyi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_rotqbybi(__cov_ra,__cov_rb);
__cov_qword __builtin_si_rotqbi(__cov_ra,__cov_rb);
__cov_qword __builtin_si_rotqbii(__cov_ra,__cov_imm);
__cov_qword __builtin_si_rothm(__cov_ra,__cov_rb);
__cov_qword __builtin_si_rothmi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_rotm(__cov_ra,__cov_rb);
__cov_qword __builtin_si_rotmi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_rotqmby(__cov_ra,__cov_rb);
__cov_qword __builtin_si_rotqmbyi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_rotqmbi(__cov_ra,__cov_rb);
__cov_qword __builtin_si_rotqmbii(__cov_ra,__cov_imm);
__cov_qword __builtin_si_rotqmbybi(__cov_ra,__cov_rb);
__cov_qword __builtin_si_rotmah(__cov_ra,__cov_rb);
__cov_qword __builtin_si_rotmahi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_rotma(__cov_ra,__cov_rb);
__cov_qword __builtin_si_rotmai(__cov_ra,__cov_imm);
__cov_qword __builtin_si_heq(__cov_ra,__cov_rb);
__cov_qword __builtin_si_heqi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_hgt(__cov_ra,__cov_rb);
__cov_qword __builtin_si_hgti(__cov_ra,__cov_imm);
__cov_qword __builtin_si_hlgt(__cov_ra,__cov_rb);
__cov_qword __builtin_si_hlgti(__cov_ra,__cov_imm);
__cov_qword __builtin_si_ceqb(__cov_ra,__cov_rb);
__cov_qword __builtin_si_ceqbi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_ceqh(__cov_ra,__cov_rb);
__cov_qword __builtin_si_ceqhi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_ceq(__cov_ra,__cov_rb);
__cov_qword __builtin_si_ceqi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_cgtb(__cov_ra,__cov_rb);
__cov_qword __builtin_si_cgtbi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_cgth(__cov_ra,__cov_rb);
__cov_qword __builtin_si_cgthi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_cgt(__cov_ra,__cov_rb);
__cov_qword __builtin_si_cgti(__cov_ra,__cov_imm);
__cov_qword __builtin_si_clgtb(__cov_ra,__cov_rb);
__cov_qword __builtin_si_clgtbi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_clgth(__cov_ra,__cov_rb);
__cov_qword __builtin_si_clgthi(__cov_ra,__cov_imm);
__cov_qword __builtin_si_clgt(__cov_ra,__cov_rb);
__cov_qword __builtin_si_clgti(__cov_ra,__cov_imm);
//__cov_qword __builtin_si_bisled(__cov_ra,int);
//__cov_qword __builtin_si_bisledd(__cov_ra,int);
//__cov_qword __builtin_si_bislede(__cov_ra,int);
__cov_qword __builtin_si_fa(__cov_ra,__cov_rb);
__cov_qword __builtin_si_dfa(__cov_ra,__cov_rb);
__cov_qword __builtin_si_fs(__cov_ra,__cov_rb);
__cov_qword __builtin_si_dfs(__cov_ra,__cov_rb);
__cov_qword __builtin_si_fm(__cov_ra,__cov_rb);
__cov_qword __builtin_si_dfm(__cov_ra,__cov_rb);
__cov_qword __builtin_si_fma(__cov_ra,__cov_rb,__cov_rc);
__cov_qword __builtin_si_dfma(__cov_ra,__cov_rb,__cov_rc);
__cov_qword __builtin_si_dfnma(__cov_ra,__cov_rb,__cov_rc);
__cov_qword __builtin_si_fnms(__cov_ra,__cov_rb,__cov_rc);
__cov_qword __builtin_si_dfnms(__cov_ra,__cov_rb,__cov_rc);
__cov_qword __builtin_si_fms(__cov_ra,__cov_rb,__cov_rc);
__cov_qword __builtin_si_dfms(__cov_ra,__cov_rb,__cov_rc);
__cov_qword __builtin_si_frest(__cov_ra);
__cov_qword __builtin_si_frsqest(__cov_ra);
__cov_qword __builtin_si_fi(__cov_ra,__cov_rb);
__cov_qword __builtin_si_csflt(__cov_ra,__cov_imm);
__cov_qword __builtin_si_cflts(__cov_ra,__cov_imm);
__cov_qword __builtin_si_cuflt(__cov_ra,__cov_imm);
__cov_qword __builtin_si_cfltu(__cov_ra,__cov_imm);
__cov_qword __builtin_si_frds(__cov_ra);
__cov_qword __builtin_si_fesd(__cov_ra);
__cov_qword __builtin_si_fceq(__cov_ra,__cov_rb);
__cov_qword __builtin_si_fcmeq(__cov_ra,__cov_rb);
__cov_qword __builtin_si_fcgt(__cov_ra,__cov_rb);
__cov_qword __builtin_si_fcmgt(__cov_ra,__cov_rb);
__cov_qword __builtin_si_stop(__cov_imm);
__cov_qword __builtin_si_stopd(__cov_ra,__cov_rb,__cov_rc);
__cov_qword __builtin_si_lnop();
__cov_qword __builtin_si_nop();
__cov_qword __builtin_si_sync();
__cov_qword __builtin_si_syncc();
__cov_qword __builtin_si_dsync();
__cov_qword __builtin_si_mfspr(__cov_imm);
__cov_qword __builtin_si_mtspr(__cov_imm,__cov_ra);
__cov_qword __builtin_si_fscrrd();
__cov_qword __builtin_si_fscrwr(__cov_ra);
__cov_qword __builtin_si_rdch(__cov_imm);
__cov_qword __builtin_si_rchcnt(__cov_imm);
__cov_qword __builtin_si_wrch(__cov_imm,__cov_ra);

__cov_qword __builtin_si_dfceq(__cov_ra,__cov_rb);
__cov_qword __builtin_si_dfcmeq(__cov_ra,__cov_rb);
__cov_qword __builtin_si_dfcgt(__cov_ra,__cov_rb);
__cov_qword __builtin_si_dfcmgt(__cov_ra,__cov_rb);
__cov_qword __builtin_si_dftsv(__cov_ra,__cov_imm);

__cov_qword __builtin_si_from_char(signed char);
__cov_qword __builtin_si_from_uchar(unsigned char);
__cov_qword __builtin_si_from_short(short);
__cov_qword __builtin_si_from_ushort(unsigned short);
__cov_qword __builtin_si_from_int(int);
__cov_qword __builtin_si_from_uint(unsigned int);
//__cov_qword __builtin_si_from_long(long);
//__cov_qword __builtin_si_from_ulong(unsigned long);
__cov_qword __builtin_si_from_float(float);
__cov_qword __builtin_si_from_double(double);
//__cov_qword __builtin_si_from_ptr(volatile void *);

signed char __builtin_si_to_char(__cov_ra);
unsigned char __builtin_si_to_uchar(__cov_ra);
short __builtin_si_to_short(__cov_ra);
unsigned short __builtin_si_to_ushort(__cov_ra);
int __builtin_si_to_int(__cov_ra);
unsigned short __builtin_si_to_uint(__cov_ra);
long __builtin_si_to_long(__cov_ra);
unsigned long __builtin_si_to_ulong(__cov_ra);
float __builtin_si_to_float(__cov_ra);
double __builtin_si_to_double(__cov_ra);
void * __builtin_si_to_ptr(__cov_ra);

//__builtin_spu_align_hint(ptr,base,offset);
void __builtin_spu_align_hint(void *,int,int);

/* generic spu_* intrinsics */

__cov_qword __builtin_spu_splats(__cov_scalar); 
__cov_qword __builtin_spu_convtf(__cov_ra,__cov_imm);
//__cov_qword __builtin_spu_convts(__cov_ra,__cov_imm);
//__cov_qword __builtin_spu_convtu(__cov_ra,__cov_imm);
__cov_qword __builtin_spu_extend(__cov_ra); 
//__cov_qword __builtin_spu_roundtf(__cov_ra);
__cov_qword __builtin_spu_add(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_addx(__cov_ra,__cov_rb,__cov_rt); 
__cov_qword __builtin_spu_genc(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_gencx(__cov_ra,__cov_rb,__cov_rt); 
__cov_qword __builtin_spu_madd(__cov_ra,__cov_rb,__cov_rc);
//__cov_qword __builtin_spu_nmadd(__cov_ra,__cov_rb,__cov_rc);
__cov_qword __builtin_spu_mhhadd(__cov_ra,__cov_rb,__cov_rc);
__cov_qword __builtin_spu_msub(__cov_ra,__cov_rb,__cov_rc); 
__cov_qword __builtin_spu_mul(__cov_ra,__cov_rb); 
//__cov_qword __builtin_spu_mulh(__cov_ra,__cov_rb);
__cov_qword __builtin_spu_mule(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_mulo(__cov_ra,__cov_rb); 
//__cov_qword __builtin_spu_mulsr(__cov_ra,__cov_rb);
__cov_qword __builtin_spu_nmsub(__cov_ra,__cov_rb,__cov_rc); 
__cov_qword __builtin_spu_sub(__cov_ra,__cov_rb);
__cov_qword __builtin_spu_subx(__cov_ra,__cov_rb,__cov_rt); 
__cov_qword __builtin_spu_genb(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_genbx(__cov_ra,__cov_rb,__cov_rt); 
//__cov_qword __builtin_spu_absd(__cov_ra,__cov_rb);
//__cov_qword __builtin_spu_avg(__cov_ra,__cov_rb);
//__cov_qword __builtin_spu_sumb(__cov_ra,__cov_rb);
//__cov_qword __builtin_spu_bisled(__cov_ra, int);
//__cov_qword __builtin_spu_bisled_d(__cov_ra, int);
//__cov_qword __builtin_spu_bisled_e(__cov_ra, int);
__cov_qword __builtin_spu_cmpabseq(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_cmpabsgt(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_cmpeq(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_cmpgt(__cov_ra,__cov_rb); 
//__cov_qword __builtin_spu_testsv(__cov_ra,__cov_imm);
__cov_qword __builtin_spu_hcmpeq(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_hcmpgt(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_cntb(__cov_ra); 
__cov_qword __builtin_spu_cntlz(__cov_ra); 
__cov_qword __builtin_spu_gather(__cov_ra); 
__cov_qword __builtin_spu_maskb(__cov_ra); 
__cov_qword __builtin_spu_maskh(__cov_ra); 
__cov_qword __builtin_spu_maskw(__cov_ra); 
__cov_qword __builtin_spu_sel(__cov_ra,__cov_rb,__cov_rc); 
__cov_qword __builtin_spu_shuffle(__cov_ra,__cov_rb,__cov_rc); 
__cov_qword __builtin_spu_and(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_andc(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_eqv(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_nand(__cov_ra,__cov_rb);
__cov_qword __builtin_spu_nor(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_or(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_orc(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_orx(__cov_ra);
__cov_qword __builtin_spu_xor(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_rl(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_rlqw(__cov_ra,__cov_count); 
__cov_qword __builtin_spu_rlqwbyte(__cov_ra,__cov_count); 
__cov_qword __builtin_spu_rlqwbytebc(__cov_ra,__cov_count); 
__cov_qword __builtin_spu_rlmask(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_rlmaska(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_rlmaskqw(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_rlmaskqwbyte(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_rlmaskqwbytebc(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_sl(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_slqw(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_slqwbyte(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_slqwbytebc(__cov_ra,__cov_rb); 
__cov_qword __builtin_spu_extract(__cov_ra,__cov_pos); 
__cov_qword __builtin_spu_insert(__cov_scalar,__cov_ra,__cov_pos); 
__cov_qword __builtin_spu_promote(__cov_scalar,__cov_pos); 

void __builtin_spu_idisable();
void __builtin_spu_ienable();

#if defined(__PS3_GCC_REVISION__)
void __builtin_spu_hcmpeq_0 (int a, int b);
void __builtin_spu_hcmpeq_1 (unsigned int a, unsigned int b);
void __builtin_spu_hcmpgt_0 (int a, int b);
void __builtin_spu_hcmpgt_1 (unsigned int a, unsigned int b);
#endif

/* See BZ31369 for how the SPU builtins were generated from spu-builtins.def */
/* BEGIN: Generated from spu-builtins.def */
void __builtin_si_bisled(__vector signed char arg0, void *arg1);
void __builtin_si_bisledd(__vector signed char arg0, void *arg1);
void __builtin_si_bislede(__vector signed char arg0, void *arg1);
void __builtin_spu_bisled(void *arg0, void *arg1);
void __builtin_spu_bisled_d(void *arg0, void *arg1);
void __builtin_spu_bisled_e(void *arg0, void *arg1);
double __builtin_dfmas(double arg0, double arg1, double arg2);
float __builtin_fmas(float arg0, float arg1, float arg2);
__vector signed char __builtin_si_a(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_absdb(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_addx(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
__vector signed char __builtin_si_ah(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_ahi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_ai(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_and(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_andbi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_andc(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_andhi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_andi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_avgb(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_bg(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_bgx(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
__vector signed char __builtin_si_cbd(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_cbx(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_cdd(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_cdx(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_ceq(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_ceqb(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_ceqbi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_ceqh(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_ceqhi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_ceqi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_cflts(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_cfltu(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_cg(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_cgt(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_cgtb(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_cgtbi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_cgth(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_cgthi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_cgti(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_cgx(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
__vector signed char __builtin_si_chd(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_chx(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_clgt(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_clgtb(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_clgtbi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_clgth(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_clgthi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_clgti(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_clz(__vector signed char arg0);
__vector signed char __builtin_si_cntb(__vector signed char arg0);
__vector signed char __builtin_si_csflt(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_cuflt(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_cwd(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_cwx(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_dfa(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_dfceq(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_dfcgt(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_dfcmeq(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_dfcmgt(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_dfm(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_dfma(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
__vector signed char __builtin_si_dfms(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
__vector signed char __builtin_si_dfnma(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
__vector signed char __builtin_si_dfnms(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
__vector signed char __builtin_si_dfs(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_dftsv(__vector signed char arg0, int arg1);
void __builtin_si_dsync();
__vector signed char __builtin_si_eqv(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_fa(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_fceq(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_fcgt(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_fcmeq(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_fcmgt(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_fesd(__vector signed char arg0);
__vector signed char __builtin_si_fi(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_fm(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_fma(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
__vector signed char __builtin_si_fms(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
__vector signed char __builtin_si_fnms(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
__vector signed char __builtin_si_frds(__vector signed char arg0);
__vector signed char __builtin_si_frest(__vector signed char arg0);
__vector signed char __builtin_si_from_char(signed char arg0);
__vector signed char __builtin_si_from_double(double arg0);
__vector signed char __builtin_si_from_float(float arg0);
__vector signed char __builtin_si_from_int(signed int arg0);
__vector signed char __builtin_si_from_long(signed long long arg0);
__vector signed char __builtin_si_from_ptr(volatile void *arg0);
__vector signed char __builtin_si_from_short(signed short arg0);
__vector signed char __builtin_si_from_uchar(unsigned char arg0);
__vector signed char __builtin_si_from_uint(unsigned int arg0);
__vector signed char __builtin_si_from_ulong(unsigned long long arg0);
__vector signed char __builtin_si_from_ushort(unsigned short arg0);
__vector signed char __builtin_si_frsqest(__vector signed char arg0);
__vector signed char __builtin_si_fs(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_fscrrd();
void __builtin_si_fscrwr(__vector signed char arg0);
__vector signed char __builtin_si_fsm(__vector signed char arg0);
__vector signed char __builtin_si_fsmb(__vector signed char arg0);
__vector signed char __builtin_si_fsmbi(int arg0);
__vector signed char __builtin_si_fsmh(__vector signed char arg0);
__vector signed char __builtin_si_gb(__vector signed char arg0);
__vector signed char __builtin_si_gbb(__vector signed char arg0);
__vector signed char __builtin_si_gbh(__vector signed char arg0);
void __builtin_si_heq(__vector signed char arg0, __vector signed char arg1);
void __builtin_si_heqi(__vector signed char arg0, int arg1);
void __builtin_si_hgt(__vector signed char arg0, __vector signed char arg1);
void __builtin_si_hgti(__vector signed char arg0, int arg1);
void __builtin_si_hlgt(__vector signed char arg0, __vector signed char arg1);
void __builtin_si_hlgti(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_il(int arg0);
__vector signed char __builtin_si_ila(int arg0);
__vector signed char __builtin_si_ilh(int arg0);
__vector signed char __builtin_si_ilhu(int arg0);
__vector signed char __builtin_si_iohl(__vector signed char arg0, int arg1);
void __builtin_si_lnop();
__vector signed char __builtin_si_lqa(int arg0);
__vector signed char __builtin_si_lqd(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_lqr(int arg0);
__vector signed char __builtin_si_lqx(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_mfspr(int arg0);
__vector signed char __builtin_si_mpy(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_mpya(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
__vector signed char __builtin_si_mpyh(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_mpyhh(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_mpyhha(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
__vector signed char __builtin_si_mpyhhau(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
__vector signed char __builtin_si_mpyhhu(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_mpyi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_mpys(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_mpyu(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_mpyui(__vector signed char arg0, int arg1);
void __builtin_si_mtspr(int arg0, __vector signed char arg1);
__vector signed char __builtin_si_nand(__vector signed char arg0, __vector signed char arg1);
void __builtin_si_nop();
__vector signed char __builtin_si_nor(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_or(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_orbi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_orc(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_orhi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_ori(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_orx(__vector signed char arg0);
__vector signed char __builtin_si_rchcnt(int arg0);
__vector signed char __builtin_si_rdch(int arg0);
__vector signed char __builtin_si_rot(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_roth(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_rothi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_rothm(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_rothmi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_roti(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_rotm(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_rotma(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_rotmah(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_rotmahi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_rotmai(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_rotmi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_rotqbi(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_rotqbii(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_rotqby(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_rotqbybi(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_rotqbyi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_rotqmbi(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_rotqmbii(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_rotqmby(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_rotqmbybi(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_rotqmbyi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_selb(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
__vector signed char __builtin_si_sf(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_sfh(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_sfhi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_sfi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_sfx(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
__vector signed char __builtin_si_shl(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_shlh(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_shlhi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_shli(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_shlqbi(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_shlqbii(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_shlqby(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_shlqbybi(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_shlqbyi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_shufb(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
void __builtin_si_stop(int arg0);
void __builtin_si_stopd(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
void __builtin_si_stqa(__vector signed char arg0, int arg1);
void __builtin_si_stqd(__vector signed char arg0, __vector signed char arg1, int arg2);
void __builtin_si_stqr(__vector signed char arg0, int arg1);
void __builtin_si_stqx(__vector signed char arg0, __vector signed char arg1, __vector signed char arg2);
__vector signed char __builtin_si_sumb(__vector signed char arg0, __vector signed char arg1);
void __builtin_si_sync();
void __builtin_si_syncc();
signed char __builtin_si_to_char(__vector signed char arg0);
double __builtin_si_to_double(__vector signed char arg0);
float __builtin_si_to_float(__vector signed char arg0);
signed int __builtin_si_to_int(__vector signed char arg0);
signed long long __builtin_si_to_long(__vector signed char arg0);
void * __builtin_si_to_ptr(__vector signed char arg0);
signed short __builtin_si_to_short(__vector signed char arg0);
unsigned char __builtin_si_to_uchar(__vector signed char arg0);
unsigned int __builtin_si_to_uint(__vector signed char arg0);
unsigned long long __builtin_si_to_ulong(__vector signed char arg0);
unsigned short __builtin_si_to_ushort(__vector signed char arg0);
void __builtin_si_wrch(int arg0, __vector signed char arg1);
__vector signed char __builtin_si_xor(__vector signed char arg0, __vector signed char arg1);
__vector signed char __builtin_si_xorbi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_xorhi(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_xori(__vector signed char arg0, int arg1);
__vector signed char __builtin_si_xsbh(__vector signed char arg0);
__vector signed char __builtin_si_xshw(__vector signed char arg0);
__vector signed char __builtin_si_xswd(__vector signed char arg0);
__vector unsigned char __builtin_spu_absd(__vector unsigned char arg0, __vector unsigned char arg1);
void __builtin_spu_align_hint(void *arg0, int arg1, int arg2);
__vector unsigned char __builtin_spu_avg(__vector unsigned char arg0, __vector unsigned char arg1);
__vector signed int __builtin_spu_convts(__vector float arg0, signed int arg1);
__vector unsigned int __builtin_spu_convtu(__vector float arg0, signed int arg1);
__vector float __builtin_spu_frest(__vector float arg0);
__vector float __builtin_spu_frsqest(__vector float arg0);
void __builtin_spu_idisable();
void __builtin_spu_ienable();
__vector signed char __builtin_spu_lvsr(void *arg0);
__vector signed int __builtin_spu_mulh(__vector signed short arg0, __vector signed short arg1);
__vector signed int __builtin_spu_mulsr(__vector signed short arg0, __vector signed short arg1);
__vector double __builtin_spu_nmadd(__vector double arg0, __vector double arg1, __vector double arg2);
__vector float __builtin_spu_roundtf(__vector double arg0);
__vector unsigned short __builtin_spu_sumb(__vector unsigned char arg0, __vector unsigned char arg1);
__vector unsigned long long __builtin_spu_testsv(__vector double arg0, int arg1);
#if __cplusplus
__vector unsigned int __builtin_spu_add(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_add(__vector signed int arg0, __vector signed int arg1);
__vector unsigned short __builtin_spu_add(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_add(__vector signed short arg0, __vector signed short arg1);
__vector float __builtin_spu_add(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_add(__vector double arg0, __vector double arg1);
__vector unsigned short __builtin_spu_add(__vector unsigned short arg0, unsigned short arg1);
__vector signed short __builtin_spu_add(__vector signed short arg0, signed short arg1);
__vector unsigned int __builtin_spu_add(__vector unsigned int arg0, unsigned int arg1);
__vector signed int __builtin_spu_add(__vector signed int arg0, signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned int __builtin_spu_add_0(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_add_1(__vector signed int arg0, __vector signed int arg1);
__vector unsigned short __builtin_spu_add_2(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_add_3(__vector signed short arg0, __vector signed short arg1);
__vector float __builtin_spu_add_4(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_add_5(__vector double arg0, __vector double arg1);
__vector unsigned short __builtin_spu_add_6(__vector unsigned short arg0, unsigned short arg1);
__vector signed short __builtin_spu_add_7(__vector signed short arg0, signed short arg1);
__vector unsigned int __builtin_spu_add_8(__vector unsigned int arg0, unsigned int arg1);
__vector signed int __builtin_spu_add_9(__vector signed int arg0, signed int arg1);
#if __cplusplus
__vector signed int __builtin_spu_addx(__vector signed int arg0, __vector signed int arg1, __vector signed int arg2);
__vector unsigned int __builtin_spu_addx(__vector unsigned int arg0, __vector unsigned int arg1, __vector unsigned int arg2);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed int __builtin_spu_addx_0(__vector signed int arg0, __vector signed int arg1, __vector signed int arg2);
__vector unsigned int __builtin_spu_addx_1(__vector unsigned int arg0, __vector unsigned int arg1, __vector unsigned int arg2);
#if __cplusplus
__vector unsigned char __builtin_spu_and(__vector unsigned char arg0, __vector unsigned char arg1);
__vector signed char __builtin_spu_and(__vector signed char arg0, __vector signed char arg1);
__vector unsigned short __builtin_spu_and(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_and(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_and(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_and(__vector signed int arg0, __vector signed int arg1);
__vector unsigned long long __builtin_spu_and(__vector unsigned long long arg0, __vector unsigned long long arg1);
__vector signed long long __builtin_spu_and(__vector signed long long arg0, __vector signed long long arg1);
__vector float __builtin_spu_and(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_and(__vector double arg0, __vector double arg1);
__vector unsigned char __builtin_spu_and(__vector unsigned char arg0, unsigned char arg1);
__vector signed char __builtin_spu_and(__vector signed char arg0, signed char arg1);
__vector unsigned short __builtin_spu_and(__vector unsigned short arg0, unsigned short arg1);
__vector signed short __builtin_spu_and(__vector signed short arg0, signed short arg1);
__vector unsigned int __builtin_spu_and(__vector unsigned int arg0, unsigned int arg1);
__vector signed int __builtin_spu_and(__vector signed int arg0, signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned char __builtin_spu_and_0(__vector unsigned char arg0, __vector unsigned char arg1);
__vector signed char __builtin_spu_and_1(__vector signed char arg0, __vector signed char arg1);
__vector unsigned short __builtin_spu_and_2(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_and_3(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_and_4(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_and_5(__vector signed int arg0, __vector signed int arg1);
__vector unsigned long long __builtin_spu_and_6(__vector unsigned long long arg0, __vector unsigned long long arg1);
__vector signed long long __builtin_spu_and_7(__vector signed long long arg0, __vector signed long long arg1);
__vector float __builtin_spu_and_8(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_and_9(__vector double arg0, __vector double arg1);
__vector unsigned char __builtin_spu_and_10(__vector unsigned char arg0, unsigned char arg1);
__vector signed char __builtin_spu_and_11(__vector signed char arg0, signed char arg1);
__vector unsigned short __builtin_spu_and_12(__vector unsigned short arg0, unsigned short arg1);
__vector signed short __builtin_spu_and_13(__vector signed short arg0, signed short arg1);
__vector unsigned int __builtin_spu_and_14(__vector unsigned int arg0, unsigned int arg1);
__vector signed int __builtin_spu_and_15(__vector signed int arg0, signed int arg1);
#if __cplusplus
__vector signed long long __builtin_spu_andc(__vector signed long long arg0, __vector signed long long arg1);
__vector unsigned long long __builtin_spu_andc(__vector unsigned long long arg0, __vector unsigned long long arg1);
__vector signed int __builtin_spu_andc(__vector signed int arg0, __vector signed int arg1);
__vector unsigned int __builtin_spu_andc(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed short __builtin_spu_andc(__vector signed short arg0, __vector signed short arg1);
__vector unsigned short __builtin_spu_andc(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed char __builtin_spu_andc(__vector signed char arg0, __vector signed char arg1);
__vector unsigned char __builtin_spu_andc(__vector unsigned char arg0, __vector unsigned char arg1);
__vector float __builtin_spu_andc(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_andc(__vector double arg0, __vector double arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed long long __builtin_spu_andc_0(__vector signed long long arg0, __vector signed long long arg1);
__vector unsigned long long __builtin_spu_andc_1(__vector unsigned long long arg0, __vector unsigned long long arg1);
__vector signed int __builtin_spu_andc_2(__vector signed int arg0, __vector signed int arg1);
__vector unsigned int __builtin_spu_andc_3(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed short __builtin_spu_andc_4(__vector signed short arg0, __vector signed short arg1);
__vector unsigned short __builtin_spu_andc_5(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed char __builtin_spu_andc_6(__vector signed char arg0, __vector signed char arg1);
__vector unsigned char __builtin_spu_andc_7(__vector unsigned char arg0, __vector unsigned char arg1);
__vector float __builtin_spu_andc_8(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_andc_9(__vector double arg0, __vector double arg1);
#if __cplusplus
__vector unsigned int __builtin_spu_cmpabseq(__vector float arg0, __vector float arg1);
__vector unsigned long long __builtin_spu_cmpabseq(__vector double arg0, __vector double arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned int __builtin_spu_cmpabseq_0(__vector float arg0, __vector float arg1);
__vector unsigned long long __builtin_spu_cmpabseq_1(__vector double arg0, __vector double arg1);
#if __cplusplus
__vector unsigned int __builtin_spu_cmpabsgt(__vector float arg0, __vector float arg1);
__vector unsigned long long __builtin_spu_cmpabsgt(__vector double arg0, __vector double arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned int __builtin_spu_cmpabsgt_0(__vector float arg0, __vector float arg1);
__vector unsigned long long __builtin_spu_cmpabsgt_1(__vector double arg0, __vector double arg1);
#if __cplusplus
__vector unsigned char __builtin_spu_cmpeq(__vector unsigned char arg0, __vector unsigned char arg1);
__vector unsigned char __builtin_spu_cmpeq(__vector signed char arg0, __vector signed char arg1);
__vector unsigned short __builtin_spu_cmpeq(__vector unsigned short arg0, __vector unsigned short arg1);
__vector unsigned short __builtin_spu_cmpeq(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_cmpeq(__vector unsigned int arg0, __vector unsigned int arg1);
__vector unsigned int __builtin_spu_cmpeq(__vector signed int arg0, __vector signed int arg1);
__vector unsigned int __builtin_spu_cmpeq(__vector float arg0, __vector float arg1);
__vector unsigned char __builtin_spu_cmpeq(__vector unsigned char arg0, unsigned char arg1);
__vector unsigned char __builtin_spu_cmpeq(__vector signed char arg0, signed char arg1);
__vector unsigned short __builtin_spu_cmpeq(__vector unsigned short arg0, unsigned short arg1);
__vector unsigned short __builtin_spu_cmpeq(__vector signed short arg0, signed short arg1);
__vector unsigned int __builtin_spu_cmpeq(__vector unsigned int arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_cmpeq(__vector signed int arg0, signed int arg1);
__vector unsigned long long __builtin_spu_cmpeq(__vector double arg0, __vector double arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned char __builtin_spu_cmpeq_0(__vector unsigned char arg0, __vector unsigned char arg1);
__vector unsigned char __builtin_spu_cmpeq_1(__vector signed char arg0, __vector signed char arg1);
__vector unsigned short __builtin_spu_cmpeq_2(__vector unsigned short arg0, __vector unsigned short arg1);
__vector unsigned short __builtin_spu_cmpeq_3(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_cmpeq_4(__vector unsigned int arg0, __vector unsigned int arg1);
__vector unsigned int __builtin_spu_cmpeq_5(__vector signed int arg0, __vector signed int arg1);
__vector unsigned int __builtin_spu_cmpeq_6(__vector float arg0, __vector float arg1);
__vector unsigned char __builtin_spu_cmpeq_7(__vector unsigned char arg0, unsigned char arg1);
__vector unsigned char __builtin_spu_cmpeq_8(__vector signed char arg0, signed char arg1);
__vector unsigned short __builtin_spu_cmpeq_9(__vector unsigned short arg0, unsigned short arg1);
__vector unsigned short __builtin_spu_cmpeq_10(__vector signed short arg0, signed short arg1);
__vector unsigned int __builtin_spu_cmpeq_11(__vector unsigned int arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_cmpeq_12(__vector signed int arg0, signed int arg1);
__vector unsigned long long __builtin_spu_cmpeq_13(__vector double arg0, __vector double arg1);
#if __cplusplus
__vector unsigned char __builtin_spu_cmpgt(__vector unsigned char arg0, __vector unsigned char arg1);
__vector unsigned char __builtin_spu_cmpgt(__vector signed char arg0, __vector signed char arg1);
__vector unsigned short __builtin_spu_cmpgt(__vector unsigned short arg0, __vector unsigned short arg1);
__vector unsigned short __builtin_spu_cmpgt(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_cmpgt(__vector unsigned int arg0, __vector unsigned int arg1);
__vector unsigned int __builtin_spu_cmpgt(__vector signed int arg0, __vector signed int arg1);
__vector unsigned int __builtin_spu_cmpgt(__vector float arg0, __vector float arg1);
__vector unsigned char __builtin_spu_cmpgt(__vector unsigned char arg0, unsigned char arg1);
__vector unsigned char __builtin_spu_cmpgt(__vector signed char arg0, signed char arg1);
__vector unsigned short __builtin_spu_cmpgt(__vector unsigned short arg0, unsigned short arg1);
__vector unsigned short __builtin_spu_cmpgt(__vector signed short arg0, signed short arg1);
__vector unsigned int __builtin_spu_cmpgt(__vector signed int arg0, signed int arg1);
__vector unsigned int __builtin_spu_cmpgt(__vector unsigned int arg0, unsigned int arg1);
__vector unsigned long long __builtin_spu_cmpgt(__vector double arg0, __vector double arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned char __builtin_spu_cmpgt_0(__vector unsigned char arg0, __vector unsigned char arg1);
__vector unsigned char __builtin_spu_cmpgt_1(__vector signed char arg0, __vector signed char arg1);
__vector unsigned short __builtin_spu_cmpgt_2(__vector unsigned short arg0, __vector unsigned short arg1);
__vector unsigned short __builtin_spu_cmpgt_3(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_cmpgt_4(__vector unsigned int arg0, __vector unsigned int arg1);
__vector unsigned int __builtin_spu_cmpgt_5(__vector signed int arg0, __vector signed int arg1);
__vector unsigned int __builtin_spu_cmpgt_6(__vector float arg0, __vector float arg1);
__vector unsigned char __builtin_spu_cmpgt_7(__vector unsigned char arg0, unsigned char arg1);
__vector unsigned char __builtin_spu_cmpgt_8(__vector signed char arg0, signed char arg1);
__vector unsigned short __builtin_spu_cmpgt_9(__vector unsigned short arg0, unsigned short arg1);
__vector unsigned short __builtin_spu_cmpgt_10(__vector signed short arg0, signed short arg1);
__vector unsigned int __builtin_spu_cmpgt_11(__vector signed int arg0, signed int arg1);
__vector unsigned int __builtin_spu_cmpgt_12(__vector unsigned int arg0, unsigned int arg1);
__vector unsigned long long __builtin_spu_cmpgt_13(__vector double arg0, __vector double arg1);
#if __cplusplus
__vector unsigned char __builtin_spu_cntb(__vector signed char arg0);
__vector unsigned char __builtin_spu_cntb(__vector unsigned char arg0);
#else
void __builtin_spu_cntb();
#endif
__vector unsigned char __builtin_spu_cntb_0(__vector signed char arg0);
__vector unsigned char __builtin_spu_cntb_1(__vector unsigned char arg0);
#if __cplusplus
__vector unsigned int __builtin_spu_cntlz(__vector signed int arg0);
__vector unsigned int __builtin_spu_cntlz(__vector unsigned int arg0);
__vector unsigned int __builtin_spu_cntlz(__vector float arg0);
#else
void __builtin_spu_cntlz();
#endif
__vector unsigned int __builtin_spu_cntlz_0(__vector signed int arg0);
__vector unsigned int __builtin_spu_cntlz_1(__vector unsigned int arg0);
__vector unsigned int __builtin_spu_cntlz_2(__vector float arg0);
#if __cplusplus
__vector float __builtin_spu_convtf(__vector unsigned int arg0, unsigned int arg1);
__vector float __builtin_spu_convtf(__vector signed int arg0, unsigned int arg1);
#else
void __builtin_spu_convtf();
#endif
__vector float __builtin_spu_convtf_0(__vector unsigned int arg0, unsigned int arg1);
__vector float __builtin_spu_convtf_1(__vector signed int arg0, unsigned int arg1);
#if __cplusplus
__vector signed long long __builtin_spu_eqv(__vector signed long long arg0, __vector signed long long arg1);
__vector unsigned long long __builtin_spu_eqv(__vector unsigned long long arg0, __vector unsigned long long arg1);
__vector signed int __builtin_spu_eqv(__vector signed int arg0, __vector signed int arg1);
__vector unsigned int __builtin_spu_eqv(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed short __builtin_spu_eqv(__vector signed short arg0, __vector signed short arg1);
__vector unsigned short __builtin_spu_eqv(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed char __builtin_spu_eqv(__vector signed char arg0, __vector signed char arg1);
__vector unsigned char __builtin_spu_eqv(__vector unsigned char arg0, __vector unsigned char arg1);
__vector float __builtin_spu_eqv(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_eqv(__vector double arg0, __vector double arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed long long __builtin_spu_eqv_0(__vector signed long long arg0, __vector signed long long arg1);
__vector unsigned long long __builtin_spu_eqv_1(__vector unsigned long long arg0, __vector unsigned long long arg1);
__vector signed int __builtin_spu_eqv_2(__vector signed int arg0, __vector signed int arg1);
__vector unsigned int __builtin_spu_eqv_3(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed short __builtin_spu_eqv_4(__vector signed short arg0, __vector signed short arg1);
__vector unsigned short __builtin_spu_eqv_5(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed char __builtin_spu_eqv_6(__vector signed char arg0, __vector signed char arg1);
__vector unsigned char __builtin_spu_eqv_7(__vector unsigned char arg0, __vector unsigned char arg1);
__vector float __builtin_spu_eqv_8(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_eqv_9(__vector double arg0, __vector double arg1);
#if __cplusplus
__vector signed short __builtin_spu_extend(__vector signed char arg0);
__vector signed int __builtin_spu_extend(__vector signed short arg0);
__vector signed long long __builtin_spu_extend(__vector signed int arg0);
__vector double __builtin_spu_extend(__vector float arg0);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed short __builtin_spu_extend_0(__vector signed char arg0);
__vector signed int __builtin_spu_extend_1(__vector signed short arg0);
__vector signed long long __builtin_spu_extend_2(__vector signed int arg0);
__vector double __builtin_spu_extend_3(__vector float arg0);
#if __cplusplus
unsigned char __builtin_spu_extract(__vector unsigned char arg0, signed int arg1);
signed char __builtin_spu_extract(__vector signed char arg0, signed int arg1);
unsigned short __builtin_spu_extract(__vector unsigned short arg0, signed int arg1);
signed short __builtin_spu_extract(__vector signed short arg0, signed int arg1);
unsigned int __builtin_spu_extract(__vector unsigned int arg0, signed int arg1);
signed int __builtin_spu_extract(__vector signed int arg0, signed int arg1);
unsigned long long __builtin_spu_extract(__vector unsigned long long arg0, signed int arg1);
signed long long __builtin_spu_extract(__vector signed long long arg0, signed int arg1);
float __builtin_spu_extract(__vector float arg0, signed int arg1);
double __builtin_spu_extract(__vector double arg0, signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
unsigned char __builtin_spu_extract_0(__vector unsigned char arg0, signed int arg1);
signed char __builtin_spu_extract_1(__vector signed char arg0, signed int arg1);
unsigned short __builtin_spu_extract_2(__vector unsigned short arg0, signed int arg1);
signed short __builtin_spu_extract_3(__vector signed short arg0, signed int arg1);
unsigned int __builtin_spu_extract_4(__vector unsigned int arg0, signed int arg1);
signed int __builtin_spu_extract_5(__vector signed int arg0, signed int arg1);
unsigned long long __builtin_spu_extract_6(__vector unsigned long long arg0, signed int arg1);
signed long long __builtin_spu_extract_7(__vector signed long long arg0, signed int arg1);
float __builtin_spu_extract_8(__vector float arg0, signed int arg1);
double __builtin_spu_extract_9(__vector double arg0, signed int arg1);
#if __cplusplus
__vector unsigned int __builtin_spu_gather(__vector signed int arg0);
__vector unsigned int __builtin_spu_gather(__vector unsigned int arg0);
__vector unsigned int __builtin_spu_gather(__vector signed short arg0);
__vector unsigned int __builtin_spu_gather(__vector unsigned short arg0);
__vector unsigned int __builtin_spu_gather(__vector signed char arg0);
__vector unsigned int __builtin_spu_gather(__vector unsigned char arg0);
__vector unsigned int __builtin_spu_gather(__vector float arg0);
#else
void __builtin_spu_gather();
#endif
__vector unsigned int __builtin_spu_gather_0(__vector signed int arg0);
__vector unsigned int __builtin_spu_gather_1(__vector unsigned int arg0);
__vector unsigned int __builtin_spu_gather_2(__vector signed short arg0);
__vector unsigned int __builtin_spu_gather_3(__vector unsigned short arg0);
__vector unsigned int __builtin_spu_gather_4(__vector signed char arg0);
__vector unsigned int __builtin_spu_gather_5(__vector unsigned char arg0);
__vector unsigned int __builtin_spu_gather_6(__vector float arg0);
#if __cplusplus
__vector unsigned int __builtin_spu_genb(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_genb(__vector signed int arg0, __vector signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned int __builtin_spu_genb_0(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_genb_1(__vector signed int arg0, __vector signed int arg1);
#if __cplusplus
__vector unsigned int __builtin_spu_genbx(__vector unsigned int arg0, __vector unsigned int arg1, __vector unsigned int arg2);
__vector signed int __builtin_spu_genbx(__vector signed int arg0, __vector signed int arg1, __vector signed int arg2);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned int __builtin_spu_genbx_0(__vector unsigned int arg0, __vector unsigned int arg1, __vector unsigned int arg2);
__vector signed int __builtin_spu_genbx_1(__vector signed int arg0, __vector signed int arg1, __vector signed int arg2);
#if __cplusplus
__vector signed int __builtin_spu_genc(__vector signed int arg0, __vector signed int arg1);
__vector unsigned int __builtin_spu_genc(__vector unsigned int arg0, __vector unsigned int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed int __builtin_spu_genc_0(__vector signed int arg0, __vector signed int arg1);
__vector unsigned int __builtin_spu_genc_1(__vector unsigned int arg0, __vector unsigned int arg1);
#if __cplusplus
__vector signed int __builtin_spu_gencx(__vector signed int arg0, __vector signed int arg1, __vector signed int arg2);
__vector unsigned int __builtin_spu_gencx(__vector unsigned int arg0, __vector unsigned int arg1, __vector unsigned int arg2);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed int __builtin_spu_gencx_0(__vector signed int arg0, __vector signed int arg1, __vector signed int arg2);
__vector unsigned int __builtin_spu_gencx_1(__vector unsigned int arg0, __vector unsigned int arg1, __vector unsigned int arg2);
#if __cplusplus
void __builtin_spu_hcmpeq(signed int arg0, signed int arg1);
void __builtin_spu_hcmpeq(unsigned int arg0, unsigned int arg1);
#else
void __builtin_spu_hcmpeq();
#endif
void __builtin_spu_hcmpeq_0(signed int arg0, signed int arg1);
void __builtin_spu_hcmpeq_1(unsigned int arg0, unsigned int arg1);
#if __cplusplus
void __builtin_spu_hcmpgt(signed int arg0, signed int arg1);
void __builtin_spu_hcmpgt(unsigned int arg0, unsigned int arg1);
#else
void __builtin_spu_hcmpgt();
#endif
void __builtin_spu_hcmpgt_0(signed int arg0, signed int arg1);
void __builtin_spu_hcmpgt_1(unsigned int arg0, unsigned int arg1);
#if __cplusplus
__vector unsigned char __builtin_spu_insert(unsigned char arg0, __vector unsigned char arg1, signed int arg2);
__vector signed char __builtin_spu_insert(signed char arg0, __vector signed char arg1, signed int arg2);
__vector unsigned short __builtin_spu_insert(unsigned short arg0, __vector unsigned short arg1, signed int arg2);
__vector signed short __builtin_spu_insert(signed short arg0, __vector signed short arg1, signed int arg2);
__vector unsigned int __builtin_spu_insert(unsigned int arg0, __vector unsigned int arg1, signed int arg2);
__vector signed int __builtin_spu_insert(signed int arg0, __vector signed int arg1, signed int arg2);
__vector unsigned long long __builtin_spu_insert(unsigned long long arg0, __vector unsigned long long arg1, signed int arg2);
__vector signed long long __builtin_spu_insert(signed long long arg0, __vector signed long long arg1, signed int arg2);
__vector float __builtin_spu_insert(float arg0, __vector float arg1, signed int arg2);
__vector double __builtin_spu_insert(double arg0, __vector double arg1, signed int arg2);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned char __builtin_spu_insert_0(unsigned char arg0, __vector unsigned char arg1, signed int arg2);
__vector signed char __builtin_spu_insert_1(signed char arg0, __vector signed char arg1, signed int arg2);
__vector unsigned short __builtin_spu_insert_2(unsigned short arg0, __vector unsigned short arg1, signed int arg2);
__vector signed short __builtin_spu_insert_3(signed short arg0, __vector signed short arg1, signed int arg2);
__vector unsigned int __builtin_spu_insert_4(unsigned int arg0, __vector unsigned int arg1, signed int arg2);
__vector signed int __builtin_spu_insert_5(signed int arg0, __vector signed int arg1, signed int arg2);
__vector unsigned long long __builtin_spu_insert_6(unsigned long long arg0, __vector unsigned long long arg1, signed int arg2);
__vector signed long long __builtin_spu_insert_7(signed long long arg0, __vector signed long long arg1, signed int arg2);
__vector float __builtin_spu_insert_8(float arg0, __vector float arg1, signed int arg2);
__vector double __builtin_spu_insert_9(double arg0, __vector double arg1, signed int arg2);
#if __cplusplus
__vector signed int __builtin_spu_madd(__vector signed short arg0, __vector signed short arg1, __vector signed int arg2);
__vector float __builtin_spu_madd(__vector float arg0, __vector float arg1, __vector float arg2);
__vector double __builtin_spu_madd(__vector double arg0, __vector double arg1, __vector double arg2);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed int __builtin_spu_madd_0(__vector signed short arg0, __vector signed short arg1, __vector signed int arg2);
__vector float __builtin_spu_madd_1(__vector float arg0, __vector float arg1, __vector float arg2);
__vector double __builtin_spu_madd_2(__vector double arg0, __vector double arg1, __vector double arg2);
#if __cplusplus
__vector unsigned char __builtin_spu_maskb(unsigned short arg0);
__vector unsigned char __builtin_spu_maskb(signed short arg0);
__vector unsigned char __builtin_spu_maskb(unsigned int arg0);
__vector unsigned char __builtin_spu_maskb(signed int arg0);
#else
void __builtin_spu_maskb();
#endif
__vector unsigned char __builtin_spu_maskb_0(unsigned short arg0);
__vector unsigned char __builtin_spu_maskb_1(signed short arg0);
__vector unsigned char __builtin_spu_maskb_2(unsigned int arg0);
__vector unsigned char __builtin_spu_maskb_3(signed int arg0);
#if __cplusplus
__vector unsigned short __builtin_spu_maskh(unsigned char arg0);
__vector unsigned short __builtin_spu_maskh(signed char arg0);
__vector unsigned short __builtin_spu_maskh(unsigned short arg0);
__vector unsigned short __builtin_spu_maskh(signed short arg0);
__vector unsigned short __builtin_spu_maskh(unsigned int arg0);
__vector unsigned short __builtin_spu_maskh(signed int arg0);
#else
void __builtin_spu_maskh();
#endif
__vector unsigned short __builtin_spu_maskh_0(unsigned char arg0);
__vector unsigned short __builtin_spu_maskh_1(signed char arg0);
__vector unsigned short __builtin_spu_maskh_2(unsigned short arg0);
__vector unsigned short __builtin_spu_maskh_3(signed short arg0);
__vector unsigned short __builtin_spu_maskh_4(unsigned int arg0);
__vector unsigned short __builtin_spu_maskh_5(signed int arg0);
#if __cplusplus
__vector unsigned int __builtin_spu_maskw(unsigned char arg0);
__vector unsigned int __builtin_spu_maskw(signed char arg0);
__vector unsigned int __builtin_spu_maskw(unsigned short arg0);
__vector unsigned int __builtin_spu_maskw(signed short arg0);
__vector unsigned int __builtin_spu_maskw(unsigned int arg0);
__vector unsigned int __builtin_spu_maskw(signed int arg0);
#else
void __builtin_spu_maskw();
#endif
__vector unsigned int __builtin_spu_maskw_0(unsigned char arg0);
__vector unsigned int __builtin_spu_maskw_1(signed char arg0);
__vector unsigned int __builtin_spu_maskw_2(unsigned short arg0);
__vector unsigned int __builtin_spu_maskw_3(signed short arg0);
__vector unsigned int __builtin_spu_maskw_4(unsigned int arg0);
__vector unsigned int __builtin_spu_maskw_5(signed int arg0);
#if __cplusplus
__vector unsigned int __builtin_spu_mhhadd(__vector unsigned short arg0, __vector unsigned short arg1, __vector unsigned int arg2);
__vector signed int __builtin_spu_mhhadd(__vector signed short arg0, __vector signed short arg1, __vector signed int arg2);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned int __builtin_spu_mhhadd_0(__vector unsigned short arg0, __vector unsigned short arg1, __vector unsigned int arg2);
__vector signed int __builtin_spu_mhhadd_1(__vector signed short arg0, __vector signed short arg1, __vector signed int arg2);
#if __cplusplus
__vector float __builtin_spu_msub(__vector float arg0, __vector float arg1, __vector float arg2);
__vector double __builtin_spu_msub(__vector double arg0, __vector double arg1, __vector double arg2);
#else
  /* No generic function generated since return types differ */
#endif
__vector float __builtin_spu_msub_0(__vector float arg0, __vector float arg1, __vector float arg2);
__vector double __builtin_spu_msub_1(__vector double arg0, __vector double arg1, __vector double arg2);
#if __cplusplus
__vector float __builtin_spu_mul(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_mul(__vector double arg0, __vector double arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector float __builtin_spu_mul_0(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_mul_1(__vector double arg0, __vector double arg1);
#if __cplusplus
__vector unsigned int __builtin_spu_mule(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed int __builtin_spu_mule(__vector signed short arg0, __vector signed short arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned int __builtin_spu_mule_0(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed int __builtin_spu_mule_1(__vector signed short arg0, __vector signed short arg1);
#if __cplusplus
__vector signed int __builtin_spu_mulo(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_mulo(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed int __builtin_spu_mulo(__vector signed short arg0, signed short arg1);
__vector unsigned int __builtin_spu_mulo(__vector unsigned short arg0, unsigned short arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed int __builtin_spu_mulo_0(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_mulo_1(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed int __builtin_spu_mulo_2(__vector signed short arg0, signed short arg1);
__vector unsigned int __builtin_spu_mulo_3(__vector unsigned short arg0, unsigned short arg1);
#if __cplusplus
__vector signed long long __builtin_spu_nand(__vector signed long long arg0, __vector signed long long arg1);
__vector unsigned long long __builtin_spu_nand(__vector unsigned long long arg0, __vector unsigned long long arg1);
__vector signed int __builtin_spu_nand(__vector signed int arg0, __vector signed int arg1);
__vector unsigned int __builtin_spu_nand(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed short __builtin_spu_nand(__vector signed short arg0, __vector signed short arg1);
__vector unsigned short __builtin_spu_nand(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed char __builtin_spu_nand(__vector signed char arg0, __vector signed char arg1);
__vector unsigned char __builtin_spu_nand(__vector unsigned char arg0, __vector unsigned char arg1);
__vector float __builtin_spu_nand(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_nand(__vector double arg0, __vector double arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed long long __builtin_spu_nand_0(__vector signed long long arg0, __vector signed long long arg1);
__vector unsigned long long __builtin_spu_nand_1(__vector unsigned long long arg0, __vector unsigned long long arg1);
__vector signed int __builtin_spu_nand_2(__vector signed int arg0, __vector signed int arg1);
__vector unsigned int __builtin_spu_nand_3(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed short __builtin_spu_nand_4(__vector signed short arg0, __vector signed short arg1);
__vector unsigned short __builtin_spu_nand_5(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed char __builtin_spu_nand_6(__vector signed char arg0, __vector signed char arg1);
__vector unsigned char __builtin_spu_nand_7(__vector unsigned char arg0, __vector unsigned char arg1);
__vector float __builtin_spu_nand_8(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_nand_9(__vector double arg0, __vector double arg1);
#if __cplusplus
__vector float __builtin_spu_nmsub(__vector float arg0, __vector float arg1, __vector float arg2);
__vector double __builtin_spu_nmsub(__vector double arg0, __vector double arg1, __vector double arg2);
#else
  /* No generic function generated since return types differ */
#endif
__vector float __builtin_spu_nmsub_0(__vector float arg0, __vector float arg1, __vector float arg2);
__vector double __builtin_spu_nmsub_1(__vector double arg0, __vector double arg1, __vector double arg2);
#if __cplusplus
__vector signed long long __builtin_spu_nor(__vector signed long long arg0, __vector signed long long arg1);
__vector unsigned long long __builtin_spu_nor(__vector unsigned long long arg0, __vector unsigned long long arg1);
__vector signed int __builtin_spu_nor(__vector signed int arg0, __vector signed int arg1);
__vector unsigned int __builtin_spu_nor(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed short __builtin_spu_nor(__vector signed short arg0, __vector signed short arg1);
__vector unsigned short __builtin_spu_nor(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed char __builtin_spu_nor(__vector signed char arg0, __vector signed char arg1);
__vector unsigned char __builtin_spu_nor(__vector unsigned char arg0, __vector unsigned char arg1);
__vector float __builtin_spu_nor(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_nor(__vector double arg0, __vector double arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed long long __builtin_spu_nor_0(__vector signed long long arg0, __vector signed long long arg1);
__vector unsigned long long __builtin_spu_nor_1(__vector unsigned long long arg0, __vector unsigned long long arg1);
__vector signed int __builtin_spu_nor_2(__vector signed int arg0, __vector signed int arg1);
__vector unsigned int __builtin_spu_nor_3(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed short __builtin_spu_nor_4(__vector signed short arg0, __vector signed short arg1);
__vector unsigned short __builtin_spu_nor_5(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed char __builtin_spu_nor_6(__vector signed char arg0, __vector signed char arg1);
__vector unsigned char __builtin_spu_nor_7(__vector unsigned char arg0, __vector unsigned char arg1);
__vector float __builtin_spu_nor_8(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_nor_9(__vector double arg0, __vector double arg1);
#if __cplusplus
__vector unsigned char __builtin_spu_or(__vector unsigned char arg0, __vector unsigned char arg1);
__vector signed char __builtin_spu_or(__vector signed char arg0, __vector signed char arg1);
__vector unsigned short __builtin_spu_or(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_or(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_or(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_or(__vector signed int arg0, __vector signed int arg1);
__vector unsigned long long __builtin_spu_or(__vector unsigned long long arg0, __vector unsigned long long arg1);
__vector signed long long __builtin_spu_or(__vector signed long long arg0, __vector signed long long arg1);
__vector float __builtin_spu_or(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_or(__vector double arg0, __vector double arg1);
__vector unsigned char __builtin_spu_or(__vector unsigned char arg0, unsigned char arg1);
__vector signed char __builtin_spu_or(__vector signed char arg0, signed char arg1);
__vector unsigned short __builtin_spu_or(__vector unsigned short arg0, unsigned short arg1);
__vector signed short __builtin_spu_or(__vector signed short arg0, signed short arg1);
__vector unsigned int __builtin_spu_or(__vector unsigned int arg0, unsigned int arg1);
__vector signed int __builtin_spu_or(__vector signed int arg0, signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned char __builtin_spu_or_0(__vector unsigned char arg0, __vector unsigned char arg1);
__vector signed char __builtin_spu_or_1(__vector signed char arg0, __vector signed char arg1);
__vector unsigned short __builtin_spu_or_2(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_or_3(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_or_4(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_or_5(__vector signed int arg0, __vector signed int arg1);
__vector unsigned long long __builtin_spu_or_6(__vector unsigned long long arg0, __vector unsigned long long arg1);
__vector signed long long __builtin_spu_or_7(__vector signed long long arg0, __vector signed long long arg1);
__vector float __builtin_spu_or_8(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_or_9(__vector double arg0, __vector double arg1);
__vector unsigned char __builtin_spu_or_10(__vector unsigned char arg0, unsigned char arg1);
__vector signed char __builtin_spu_or_11(__vector signed char arg0, signed char arg1);
__vector unsigned short __builtin_spu_or_12(__vector unsigned short arg0, unsigned short arg1);
__vector signed short __builtin_spu_or_13(__vector signed short arg0, signed short arg1);
__vector unsigned int __builtin_spu_or_14(__vector unsigned int arg0, unsigned int arg1);
__vector signed int __builtin_spu_or_15(__vector signed int arg0, signed int arg1);
#if __cplusplus
__vector signed long long __builtin_spu_orc(__vector signed long long arg0, __vector signed long long arg1);
__vector unsigned long long __builtin_spu_orc(__vector unsigned long long arg0, __vector unsigned long long arg1);
__vector signed int __builtin_spu_orc(__vector signed int arg0, __vector signed int arg1);
__vector unsigned int __builtin_spu_orc(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed short __builtin_spu_orc(__vector signed short arg0, __vector signed short arg1);
__vector unsigned short __builtin_spu_orc(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed char __builtin_spu_orc(__vector signed char arg0, __vector signed char arg1);
__vector unsigned char __builtin_spu_orc(__vector unsigned char arg0, __vector unsigned char arg1);
__vector float __builtin_spu_orc(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_orc(__vector double arg0, __vector double arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed long long __builtin_spu_orc_0(__vector signed long long arg0, __vector signed long long arg1);
__vector unsigned long long __builtin_spu_orc_1(__vector unsigned long long arg0, __vector unsigned long long arg1);
__vector signed int __builtin_spu_orc_2(__vector signed int arg0, __vector signed int arg1);
__vector unsigned int __builtin_spu_orc_3(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed short __builtin_spu_orc_4(__vector signed short arg0, __vector signed short arg1);
__vector unsigned short __builtin_spu_orc_5(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed char __builtin_spu_orc_6(__vector signed char arg0, __vector signed char arg1);
__vector unsigned char __builtin_spu_orc_7(__vector unsigned char arg0, __vector unsigned char arg1);
__vector float __builtin_spu_orc_8(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_orc_9(__vector double arg0, __vector double arg1);
#if __cplusplus
__vector signed int __builtin_spu_orx(__vector signed int arg0);
__vector unsigned int __builtin_spu_orx(__vector unsigned int arg0);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed int __builtin_spu_orx_0(__vector signed int arg0);
__vector unsigned int __builtin_spu_orx_1(__vector unsigned int arg0);
#if __cplusplus
__vector unsigned char __builtin_spu_promote(unsigned char arg0, signed int arg1);
__vector signed char __builtin_spu_promote(signed char arg0, signed int arg1);
__vector unsigned short __builtin_spu_promote(unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_promote(signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_promote(unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_promote(signed int arg0, signed int arg1);
__vector unsigned long long __builtin_spu_promote(unsigned long long arg0, signed int arg1);
__vector signed long long __builtin_spu_promote(signed long long arg0, signed int arg1);
__vector float __builtin_spu_promote(float arg0, signed int arg1);
__vector double __builtin_spu_promote(double arg0, signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned char __builtin_spu_promote_0(unsigned char arg0, signed int arg1);
__vector signed char __builtin_spu_promote_1(signed char arg0, signed int arg1);
__vector unsigned short __builtin_spu_promote_2(unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_promote_3(signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_promote_4(unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_promote_5(signed int arg0, signed int arg1);
__vector unsigned long long __builtin_spu_promote_6(unsigned long long arg0, signed int arg1);
__vector signed long long __builtin_spu_promote_7(signed long long arg0, signed int arg1);
__vector float __builtin_spu_promote_8(float arg0, signed int arg1);
__vector double __builtin_spu_promote_9(double arg0, signed int arg1);
#if __cplusplus
__vector unsigned short __builtin_spu_rl(__vector unsigned short arg0, __vector signed short arg1);
__vector signed short __builtin_spu_rl(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_rl(__vector unsigned int arg0, __vector signed int arg1);
__vector signed int __builtin_spu_rl(__vector signed int arg0, __vector signed int arg1);
__vector unsigned short __builtin_spu_rl(__vector unsigned short arg0, signed short arg1);
__vector signed short __builtin_spu_rl(__vector signed short arg0, signed short arg1);
__vector unsigned int __builtin_spu_rl(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rl(__vector signed int arg0, signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned short __builtin_spu_rl_0(__vector unsigned short arg0, __vector signed short arg1);
__vector signed short __builtin_spu_rl_1(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_rl_2(__vector unsigned int arg0, __vector signed int arg1);
__vector signed int __builtin_spu_rl_3(__vector signed int arg0, __vector signed int arg1);
__vector unsigned short __builtin_spu_rl_4(__vector unsigned short arg0, signed short arg1);
__vector signed short __builtin_spu_rl_5(__vector signed short arg0, signed short arg1);
__vector unsigned int __builtin_spu_rl_6(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rl_7(__vector signed int arg0, signed int arg1);
#if __cplusplus
__vector unsigned short __builtin_spu_rlmask(__vector unsigned short arg0, __vector signed short arg1);
__vector signed short __builtin_spu_rlmask(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_rlmask(__vector unsigned int arg0, __vector signed int arg1);
__vector signed int __builtin_spu_rlmask(__vector signed int arg0, __vector signed int arg1);
__vector unsigned short __builtin_spu_rlmask(__vector unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_rlmask(__vector signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_rlmask(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rlmask(__vector signed int arg0, signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned short __builtin_spu_rlmask_0(__vector unsigned short arg0, __vector signed short arg1);
__vector signed short __builtin_spu_rlmask_1(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_rlmask_2(__vector unsigned int arg0, __vector signed int arg1);
__vector signed int __builtin_spu_rlmask_3(__vector signed int arg0, __vector signed int arg1);
__vector unsigned short __builtin_spu_rlmask_4(__vector unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_rlmask_5(__vector signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_rlmask_6(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rlmask_7(__vector signed int arg0, signed int arg1);
#if __cplusplus
__vector unsigned short __builtin_spu_rlmaska(__vector unsigned short arg0, __vector signed short arg1);
__vector signed short __builtin_spu_rlmaska(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_rlmaska(__vector unsigned int arg0, __vector signed int arg1);
__vector signed int __builtin_spu_rlmaska(__vector signed int arg0, __vector signed int arg1);
__vector unsigned short __builtin_spu_rlmaska(__vector unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_rlmaska(__vector signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_rlmaska(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rlmaska(__vector signed int arg0, signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned short __builtin_spu_rlmaska_0(__vector unsigned short arg0, __vector signed short arg1);
__vector signed short __builtin_spu_rlmaska_1(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_rlmaska_2(__vector unsigned int arg0, __vector signed int arg1);
__vector signed int __builtin_spu_rlmaska_3(__vector signed int arg0, __vector signed int arg1);
__vector unsigned short __builtin_spu_rlmaska_4(__vector unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_rlmaska_5(__vector signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_rlmaska_6(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rlmaska_7(__vector signed int arg0, signed int arg1);
#if __cplusplus
__vector unsigned char __builtin_spu_rlmaskqw(__vector unsigned char arg0, signed int arg1);
__vector signed char __builtin_spu_rlmaskqw(__vector signed char arg0, signed int arg1);
__vector unsigned short __builtin_spu_rlmaskqw(__vector unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_rlmaskqw(__vector signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_rlmaskqw(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rlmaskqw(__vector signed int arg0, signed int arg1);
__vector unsigned long long __builtin_spu_rlmaskqw(__vector unsigned long long arg0, signed int arg1);
__vector signed long long __builtin_spu_rlmaskqw(__vector signed long long arg0, signed int arg1);
__vector float __builtin_spu_rlmaskqw(__vector float arg0, signed int arg1);
__vector double __builtin_spu_rlmaskqw(__vector double arg0, signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned char __builtin_spu_rlmaskqw_0(__vector unsigned char arg0, signed int arg1);
__vector signed char __builtin_spu_rlmaskqw_1(__vector signed char arg0, signed int arg1);
__vector unsigned short __builtin_spu_rlmaskqw_2(__vector unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_rlmaskqw_3(__vector signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_rlmaskqw_4(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rlmaskqw_5(__vector signed int arg0, signed int arg1);
__vector unsigned long long __builtin_spu_rlmaskqw_6(__vector unsigned long long arg0, signed int arg1);
__vector signed long long __builtin_spu_rlmaskqw_7(__vector signed long long arg0, signed int arg1);
__vector float __builtin_spu_rlmaskqw_8(__vector float arg0, signed int arg1);
__vector double __builtin_spu_rlmaskqw_9(__vector double arg0, signed int arg1);
#if __cplusplus
__vector unsigned char __builtin_spu_rlmaskqwbyte(__vector unsigned char arg0, signed int arg1);
__vector signed char __builtin_spu_rlmaskqwbyte(__vector signed char arg0, signed int arg1);
__vector unsigned short __builtin_spu_rlmaskqwbyte(__vector unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_rlmaskqwbyte(__vector signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_rlmaskqwbyte(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rlmaskqwbyte(__vector signed int arg0, signed int arg1);
__vector unsigned long long __builtin_spu_rlmaskqwbyte(__vector unsigned long long arg0, signed int arg1);
__vector signed long long __builtin_spu_rlmaskqwbyte(__vector signed long long arg0, signed int arg1);
__vector float __builtin_spu_rlmaskqwbyte(__vector float arg0, signed int arg1);
__vector double __builtin_spu_rlmaskqwbyte(__vector double arg0, signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned char __builtin_spu_rlmaskqwbyte_0(__vector unsigned char arg0, signed int arg1);
__vector signed char __builtin_spu_rlmaskqwbyte_1(__vector signed char arg0, signed int arg1);
__vector unsigned short __builtin_spu_rlmaskqwbyte_2(__vector unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_rlmaskqwbyte_3(__vector signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_rlmaskqwbyte_4(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rlmaskqwbyte_5(__vector signed int arg0, signed int arg1);
__vector unsigned long long __builtin_spu_rlmaskqwbyte_6(__vector unsigned long long arg0, signed int arg1);
__vector signed long long __builtin_spu_rlmaskqwbyte_7(__vector signed long long arg0, signed int arg1);
__vector float __builtin_spu_rlmaskqwbyte_8(__vector float arg0, signed int arg1);
__vector double __builtin_spu_rlmaskqwbyte_9(__vector double arg0, signed int arg1);
#if __cplusplus
__vector unsigned char __builtin_spu_rlmaskqwbytebc(__vector unsigned char arg0, signed int arg1);
__vector signed char __builtin_spu_rlmaskqwbytebc(__vector signed char arg0, signed int arg1);
__vector unsigned short __builtin_spu_rlmaskqwbytebc(__vector unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_rlmaskqwbytebc(__vector signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_rlmaskqwbytebc(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rlmaskqwbytebc(__vector signed int arg0, signed int arg1);
__vector unsigned long long __builtin_spu_rlmaskqwbytebc(__vector unsigned long long arg0, signed int arg1);
__vector signed long long __builtin_spu_rlmaskqwbytebc(__vector signed long long arg0, signed int arg1);
__vector float __builtin_spu_rlmaskqwbytebc(__vector float arg0, signed int arg1);
__vector double __builtin_spu_rlmaskqwbytebc(__vector double arg0, signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned char __builtin_spu_rlmaskqwbytebc_0(__vector unsigned char arg0, signed int arg1);
__vector signed char __builtin_spu_rlmaskqwbytebc_1(__vector signed char arg0, signed int arg1);
__vector unsigned short __builtin_spu_rlmaskqwbytebc_2(__vector unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_rlmaskqwbytebc_3(__vector signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_rlmaskqwbytebc_4(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rlmaskqwbytebc_5(__vector signed int arg0, signed int arg1);
__vector unsigned long long __builtin_spu_rlmaskqwbytebc_6(__vector unsigned long long arg0, signed int arg1);
__vector signed long long __builtin_spu_rlmaskqwbytebc_7(__vector signed long long arg0, signed int arg1);
__vector float __builtin_spu_rlmaskqwbytebc_8(__vector float arg0, signed int arg1);
__vector double __builtin_spu_rlmaskqwbytebc_9(__vector double arg0, signed int arg1);
#if __cplusplus
__vector unsigned char __builtin_spu_rlqw(__vector unsigned char arg0, signed int arg1);
__vector signed char __builtin_spu_rlqw(__vector signed char arg0, signed int arg1);
__vector unsigned short __builtin_spu_rlqw(__vector unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_rlqw(__vector signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_rlqw(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rlqw(__vector signed int arg0, signed int arg1);
__vector unsigned long long __builtin_spu_rlqw(__vector unsigned long long arg0, signed int arg1);
__vector signed long long __builtin_spu_rlqw(__vector signed long long arg0, signed int arg1);
__vector float __builtin_spu_rlqw(__vector float arg0, signed int arg1);
__vector double __builtin_spu_rlqw(__vector double arg0, signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned char __builtin_spu_rlqw_0(__vector unsigned char arg0, signed int arg1);
__vector signed char __builtin_spu_rlqw_1(__vector signed char arg0, signed int arg1);
__vector unsigned short __builtin_spu_rlqw_2(__vector unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_rlqw_3(__vector signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_rlqw_4(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rlqw_5(__vector signed int arg0, signed int arg1);
__vector unsigned long long __builtin_spu_rlqw_6(__vector unsigned long long arg0, signed int arg1);
__vector signed long long __builtin_spu_rlqw_7(__vector signed long long arg0, signed int arg1);
__vector float __builtin_spu_rlqw_8(__vector float arg0, signed int arg1);
__vector double __builtin_spu_rlqw_9(__vector double arg0, signed int arg1);
#if __cplusplus
__vector unsigned char __builtin_spu_rlqwbyte(__vector unsigned char arg0, signed int arg1);
__vector signed char __builtin_spu_rlqwbyte(__vector signed char arg0, signed int arg1);
__vector unsigned short __builtin_spu_rlqwbyte(__vector unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_rlqwbyte(__vector signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_rlqwbyte(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rlqwbyte(__vector signed int arg0, signed int arg1);
__vector unsigned long long __builtin_spu_rlqwbyte(__vector unsigned long long arg0, signed int arg1);
__vector signed long long __builtin_spu_rlqwbyte(__vector signed long long arg0, signed int arg1);
__vector float __builtin_spu_rlqwbyte(__vector float arg0, signed int arg1);
__vector double __builtin_spu_rlqwbyte(__vector double arg0, signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned char __builtin_spu_rlqwbyte_0(__vector unsigned char arg0, signed int arg1);
__vector signed char __builtin_spu_rlqwbyte_1(__vector signed char arg0, signed int arg1);
__vector unsigned short __builtin_spu_rlqwbyte_2(__vector unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_rlqwbyte_3(__vector signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_rlqwbyte_4(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rlqwbyte_5(__vector signed int arg0, signed int arg1);
__vector unsigned long long __builtin_spu_rlqwbyte_6(__vector unsigned long long arg0, signed int arg1);
__vector signed long long __builtin_spu_rlqwbyte_7(__vector signed long long arg0, signed int arg1);
__vector float __builtin_spu_rlqwbyte_8(__vector float arg0, signed int arg1);
__vector double __builtin_spu_rlqwbyte_9(__vector double arg0, signed int arg1);
#if __cplusplus
__vector unsigned char __builtin_spu_rlqwbytebc(__vector unsigned char arg0, signed int arg1);
__vector signed char __builtin_spu_rlqwbytebc(__vector signed char arg0, signed int arg1);
__vector unsigned short __builtin_spu_rlqwbytebc(__vector unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_rlqwbytebc(__vector signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_rlqwbytebc(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rlqwbytebc(__vector signed int arg0, signed int arg1);
__vector unsigned long long __builtin_spu_rlqwbytebc(__vector unsigned long long arg0, signed int arg1);
__vector signed long long __builtin_spu_rlqwbytebc(__vector signed long long arg0, signed int arg1);
__vector float __builtin_spu_rlqwbytebc(__vector float arg0, signed int arg1);
__vector double __builtin_spu_rlqwbytebc(__vector double arg0, signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned char __builtin_spu_rlqwbytebc_0(__vector unsigned char arg0, signed int arg1);
__vector signed char __builtin_spu_rlqwbytebc_1(__vector signed char arg0, signed int arg1);
__vector unsigned short __builtin_spu_rlqwbytebc_2(__vector unsigned short arg0, signed int arg1);
__vector signed short __builtin_spu_rlqwbytebc_3(__vector signed short arg0, signed int arg1);
__vector unsigned int __builtin_spu_rlqwbytebc_4(__vector unsigned int arg0, signed int arg1);
__vector signed int __builtin_spu_rlqwbytebc_5(__vector signed int arg0, signed int arg1);
__vector unsigned long long __builtin_spu_rlqwbytebc_6(__vector unsigned long long arg0, signed int arg1);
__vector signed long long __builtin_spu_rlqwbytebc_7(__vector signed long long arg0, signed int arg1);
__vector float __builtin_spu_rlqwbytebc_8(__vector float arg0, signed int arg1);
__vector double __builtin_spu_rlqwbytebc_9(__vector double arg0, signed int arg1);
#if __cplusplus
__vector signed long long __builtin_spu_sel(__vector signed long long arg0, __vector signed long long arg1, __vector unsigned long long arg2);
__vector unsigned long long __builtin_spu_sel(__vector unsigned long long arg0, __vector unsigned long long arg1, __vector unsigned long long arg2);
__vector signed int __builtin_spu_sel(__vector signed int arg0, __vector signed int arg1, __vector unsigned int arg2);
__vector unsigned int __builtin_spu_sel(__vector unsigned int arg0, __vector unsigned int arg1, __vector unsigned int arg2);
__vector signed short __builtin_spu_sel(__vector signed short arg0, __vector signed short arg1, __vector unsigned short arg2);
__vector unsigned short __builtin_spu_sel(__vector unsigned short arg0, __vector unsigned short arg1, __vector unsigned short arg2);
__vector signed char __builtin_spu_sel(__vector signed char arg0, __vector signed char arg1, __vector unsigned char arg2);
__vector unsigned char __builtin_spu_sel(__vector unsigned char arg0, __vector unsigned char arg1, __vector unsigned char arg2);
__vector float __builtin_spu_sel(__vector float arg0, __vector float arg1, __vector unsigned int arg2);
__vector double __builtin_spu_sel(__vector double arg0, __vector double arg1, __vector unsigned long long arg2);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed long long __builtin_spu_sel_0(__vector signed long long arg0, __vector signed long long arg1, __vector unsigned long long arg2);
__vector signed long long __builtin_spu_sel_0o(__vector signed long long arg0, __vector signed long long arg1, __vector unsigned char arg2);
__vector unsigned long long __builtin_spu_sel_1(__vector unsigned long long arg0, __vector unsigned long long arg1, __vector unsigned long long arg2);
__vector unsigned long long __builtin_spu_sel_1o(__vector unsigned long long arg0, __vector unsigned long long arg1, __vector unsigned char arg2);
__vector signed int __builtin_spu_sel_2(__vector signed int arg0, __vector signed int arg1, __vector unsigned int arg2);
__vector signed int __builtin_spu_sel_2o(__vector signed int arg0, __vector signed int arg1, __vector unsigned char arg2);
__vector unsigned int __builtin_spu_sel_3(__vector unsigned int arg0, __vector unsigned int arg1, __vector unsigned int arg2);
__vector unsigned int __builtin_spu_sel_3o(__vector unsigned int arg0, __vector unsigned int arg1, __vector unsigned char arg2);
__vector signed short __builtin_spu_sel_4(__vector signed short arg0, __vector signed short arg1, __vector unsigned short arg2);
__vector signed short __builtin_spu_sel_4o(__vector signed short arg0, __vector signed short arg1, __vector unsigned char arg2);
__vector unsigned short __builtin_spu_sel_5(__vector unsigned short arg0, __vector unsigned short arg1, __vector unsigned short arg2);
__vector unsigned short __builtin_spu_sel_5o(__vector unsigned short arg0, __vector unsigned short arg1, __vector unsigned char arg2);
__vector signed char __builtin_spu_sel_6(__vector signed char arg0, __vector signed char arg1, __vector unsigned char arg2);
__vector unsigned char __builtin_spu_sel_7(__vector unsigned char arg0, __vector unsigned char arg1, __vector unsigned char arg2);
__vector float __builtin_spu_sel_8(__vector float arg0, __vector float arg1, __vector unsigned int arg2);
__vector float __builtin_spu_sel_8o(__vector float arg0, __vector float arg1, __vector unsigned char arg2);
__vector double __builtin_spu_sel_9(__vector double arg0, __vector double arg1, __vector unsigned long long arg2);
__vector double __builtin_spu_sel_9o(__vector double arg0, __vector double arg1, __vector unsigned char arg2);
#if __cplusplus
__vector unsigned char __builtin_spu_shuffle(__vector unsigned char arg0, __vector unsigned char arg1, __vector unsigned char arg2);
__vector signed char __builtin_spu_shuffle(__vector signed char arg0, __vector signed char arg1, __vector unsigned char arg2);
__vector unsigned short __builtin_spu_shuffle(__vector unsigned short arg0, __vector unsigned short arg1, __vector unsigned char arg2);
__vector signed short __builtin_spu_shuffle(__vector signed short arg0, __vector signed short arg1, __vector unsigned char arg2);
__vector unsigned int __builtin_spu_shuffle(__vector unsigned int arg0, __vector unsigned int arg1, __vector unsigned char arg2);
__vector signed int __builtin_spu_shuffle(__vector signed int arg0, __vector signed int arg1, __vector unsigned char arg2);
__vector unsigned long long __builtin_spu_shuffle(__vector unsigned long long arg0, __vector unsigned long long arg1, __vector unsigned char arg2);
__vector signed long long __builtin_spu_shuffle(__vector signed long long arg0, __vector signed long long arg1, __vector unsigned char arg2);
__vector float __builtin_spu_shuffle(__vector float arg0, __vector float arg1, __vector unsigned char arg2);
__vector double __builtin_spu_shuffle(__vector double arg0, __vector double arg1, __vector unsigned char arg2);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned char __builtin_spu_shuffle_0(__vector unsigned char arg0, __vector unsigned char arg1, __vector unsigned char arg2);
__vector signed char __builtin_spu_shuffle_1(__vector signed char arg0, __vector signed char arg1, __vector unsigned char arg2);
__vector unsigned short __builtin_spu_shuffle_2(__vector unsigned short arg0, __vector unsigned short arg1, __vector unsigned char arg2);
__vector signed short __builtin_spu_shuffle_3(__vector signed short arg0, __vector signed short arg1, __vector unsigned char arg2);
__vector unsigned int __builtin_spu_shuffle_4(__vector unsigned int arg0, __vector unsigned int arg1, __vector unsigned char arg2);
__vector signed int __builtin_spu_shuffle_5(__vector signed int arg0, __vector signed int arg1, __vector unsigned char arg2);
__vector unsigned long long __builtin_spu_shuffle_6(__vector unsigned long long arg0, __vector unsigned long long arg1, __vector unsigned char arg2);
__vector signed long long __builtin_spu_shuffle_7(__vector signed long long arg0, __vector signed long long arg1, __vector unsigned char arg2);
__vector float __builtin_spu_shuffle_8(__vector float arg0, __vector float arg1, __vector unsigned char arg2);
__vector double __builtin_spu_shuffle_9(__vector double arg0, __vector double arg1, __vector unsigned char arg2);
#if __cplusplus
__vector unsigned short __builtin_spu_sl(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_sl(__vector signed short arg0, __vector unsigned short arg1);
__vector unsigned int __builtin_spu_sl(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_sl(__vector signed int arg0, __vector unsigned int arg1);
__vector unsigned short __builtin_spu_sl(__vector unsigned short arg0, unsigned int arg1);
__vector signed short __builtin_spu_sl(__vector signed short arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_sl(__vector unsigned int arg0, unsigned int arg1);
__vector signed int __builtin_spu_sl(__vector signed int arg0, unsigned int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned short __builtin_spu_sl_0(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_sl_1(__vector signed short arg0, __vector unsigned short arg1);
__vector unsigned int __builtin_spu_sl_2(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_sl_3(__vector signed int arg0, __vector unsigned int arg1);
__vector unsigned short __builtin_spu_sl_4(__vector unsigned short arg0, unsigned int arg1);
__vector signed short __builtin_spu_sl_5(__vector signed short arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_sl_6(__vector unsigned int arg0, unsigned int arg1);
__vector signed int __builtin_spu_sl_7(__vector signed int arg0, unsigned int arg1);
#if __cplusplus
__vector signed long long __builtin_spu_slqw(__vector signed long long arg0, unsigned int arg1);
__vector unsigned long long __builtin_spu_slqw(__vector unsigned long long arg0, unsigned int arg1);
__vector signed int __builtin_spu_slqw(__vector signed int arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_slqw(__vector unsigned int arg0, unsigned int arg1);
__vector signed short __builtin_spu_slqw(__vector signed short arg0, unsigned int arg1);
__vector unsigned short __builtin_spu_slqw(__vector unsigned short arg0, unsigned int arg1);
__vector signed char __builtin_spu_slqw(__vector signed char arg0, unsigned int arg1);
__vector unsigned char __builtin_spu_slqw(__vector unsigned char arg0, unsigned int arg1);
__vector float __builtin_spu_slqw(__vector float arg0, unsigned int arg1);
__vector double __builtin_spu_slqw(__vector double arg0, unsigned int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed long long __builtin_spu_slqw_0(__vector signed long long arg0, unsigned int arg1);
__vector unsigned long long __builtin_spu_slqw_1(__vector unsigned long long arg0, unsigned int arg1);
__vector signed int __builtin_spu_slqw_2(__vector signed int arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_slqw_3(__vector unsigned int arg0, unsigned int arg1);
__vector signed short __builtin_spu_slqw_4(__vector signed short arg0, unsigned int arg1);
__vector unsigned short __builtin_spu_slqw_5(__vector unsigned short arg0, unsigned int arg1);
__vector signed char __builtin_spu_slqw_6(__vector signed char arg0, unsigned int arg1);
__vector unsigned char __builtin_spu_slqw_7(__vector unsigned char arg0, unsigned int arg1);
__vector float __builtin_spu_slqw_8(__vector float arg0, unsigned int arg1);
__vector double __builtin_spu_slqw_9(__vector double arg0, unsigned int arg1);
#if __cplusplus
__vector signed long long __builtin_spu_slqwbyte(__vector signed long long arg0, unsigned int arg1);
__vector unsigned long long __builtin_spu_slqwbyte(__vector unsigned long long arg0, unsigned int arg1);
__vector signed int __builtin_spu_slqwbyte(__vector signed int arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_slqwbyte(__vector unsigned int arg0, unsigned int arg1);
__vector signed short __builtin_spu_slqwbyte(__vector signed short arg0, unsigned int arg1);
__vector unsigned short __builtin_spu_slqwbyte(__vector unsigned short arg0, unsigned int arg1);
__vector signed char __builtin_spu_slqwbyte(__vector signed char arg0, unsigned int arg1);
__vector unsigned char __builtin_spu_slqwbyte(__vector unsigned char arg0, unsigned int arg1);
__vector float __builtin_spu_slqwbyte(__vector float arg0, unsigned int arg1);
__vector double __builtin_spu_slqwbyte(__vector double arg0, unsigned int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed long long __builtin_spu_slqwbyte_0(__vector signed long long arg0, unsigned int arg1);
__vector unsigned long long __builtin_spu_slqwbyte_1(__vector unsigned long long arg0, unsigned int arg1);
__vector signed int __builtin_spu_slqwbyte_2(__vector signed int arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_slqwbyte_3(__vector unsigned int arg0, unsigned int arg1);
__vector signed short __builtin_spu_slqwbyte_4(__vector signed short arg0, unsigned int arg1);
__vector unsigned short __builtin_spu_slqwbyte_5(__vector unsigned short arg0, unsigned int arg1);
__vector signed char __builtin_spu_slqwbyte_6(__vector signed char arg0, unsigned int arg1);
__vector unsigned char __builtin_spu_slqwbyte_7(__vector unsigned char arg0, unsigned int arg1);
__vector float __builtin_spu_slqwbyte_8(__vector float arg0, unsigned int arg1);
__vector double __builtin_spu_slqwbyte_9(__vector double arg0, unsigned int arg1);
#if __cplusplus
__vector signed long long __builtin_spu_slqwbytebc(__vector signed long long arg0, unsigned int arg1);
__vector unsigned long long __builtin_spu_slqwbytebc(__vector unsigned long long arg0, unsigned int arg1);
__vector signed int __builtin_spu_slqwbytebc(__vector signed int arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_slqwbytebc(__vector unsigned int arg0, unsigned int arg1);
__vector signed short __builtin_spu_slqwbytebc(__vector signed short arg0, unsigned int arg1);
__vector unsigned short __builtin_spu_slqwbytebc(__vector unsigned short arg0, unsigned int arg1);
__vector signed char __builtin_spu_slqwbytebc(__vector signed char arg0, unsigned int arg1);
__vector unsigned char __builtin_spu_slqwbytebc(__vector unsigned char arg0, unsigned int arg1);
__vector float __builtin_spu_slqwbytebc(__vector float arg0, unsigned int arg1);
__vector double __builtin_spu_slqwbytebc(__vector double arg0, unsigned int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed long long __builtin_spu_slqwbytebc_0(__vector signed long long arg0, unsigned int arg1);
__vector unsigned long long __builtin_spu_slqwbytebc_1(__vector unsigned long long arg0, unsigned int arg1);
__vector signed int __builtin_spu_slqwbytebc_2(__vector signed int arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_slqwbytebc_3(__vector unsigned int arg0, unsigned int arg1);
__vector signed short __builtin_spu_slqwbytebc_4(__vector signed short arg0, unsigned int arg1);
__vector unsigned short __builtin_spu_slqwbytebc_5(__vector unsigned short arg0, unsigned int arg1);
__vector signed char __builtin_spu_slqwbytebc_6(__vector signed char arg0, unsigned int arg1);
__vector unsigned char __builtin_spu_slqwbytebc_7(__vector unsigned char arg0, unsigned int arg1);
__vector float __builtin_spu_slqwbytebc_8(__vector float arg0, unsigned int arg1);
__vector double __builtin_spu_slqwbytebc_9(__vector double arg0, unsigned int arg1);
#if __cplusplus
__vector unsigned char __builtin_spu_splats(unsigned char arg0);
__vector signed char __builtin_spu_splats(signed char arg0);
__vector unsigned short __builtin_spu_splats(unsigned short arg0);
__vector signed short __builtin_spu_splats(signed short arg0);
__vector unsigned int __builtin_spu_splats(unsigned int arg0);
__vector signed int __builtin_spu_splats(signed int arg0);
__vector unsigned long long __builtin_spu_splats(unsigned long long arg0);
__vector signed long long __builtin_spu_splats(signed long long arg0);
__vector float __builtin_spu_splats(float arg0);
__vector double __builtin_spu_splats(double arg0);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned char __builtin_spu_splats_0(unsigned char arg0);
__vector signed char __builtin_spu_splats_1(signed char arg0);
__vector unsigned short __builtin_spu_splats_2(unsigned short arg0);
__vector signed short __builtin_spu_splats_3(signed short arg0);
__vector unsigned int __builtin_spu_splats_4(unsigned int arg0);
__vector signed int __builtin_spu_splats_5(signed int arg0);
__vector unsigned long long __builtin_spu_splats_6(unsigned long long arg0);
__vector signed long long __builtin_spu_splats_7(signed long long arg0);
__vector float __builtin_spu_splats_8(float arg0);
__vector double __builtin_spu_splats_9(double arg0);
#if __cplusplus
__vector unsigned short __builtin_spu_sr(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_sr(__vector signed short arg0, __vector unsigned short arg1);
__vector unsigned int __builtin_spu_sr(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_sr(__vector signed int arg0, __vector unsigned int arg1);
__vector unsigned short __builtin_spu_sr(__vector unsigned short arg0, unsigned int arg1);
__vector signed short __builtin_spu_sr(__vector signed short arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_sr(__vector unsigned int arg0, unsigned int arg1);
__vector signed int __builtin_spu_sr(__vector signed int arg0, unsigned int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned short __builtin_spu_sr_0(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_sr_1(__vector signed short arg0, __vector unsigned short arg1);
__vector unsigned int __builtin_spu_sr_2(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_sr_3(__vector signed int arg0, __vector unsigned int arg1);
__vector unsigned short __builtin_spu_sr_4(__vector unsigned short arg0, unsigned int arg1);
__vector signed short __builtin_spu_sr_5(__vector signed short arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_sr_6(__vector unsigned int arg0, unsigned int arg1);
__vector signed int __builtin_spu_sr_7(__vector signed int arg0, unsigned int arg1);
#if __cplusplus
__vector unsigned short __builtin_spu_sra(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_sra(__vector signed short arg0, __vector unsigned short arg1);
__vector unsigned int __builtin_spu_sra(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_sra(__vector signed int arg0, __vector unsigned int arg1);
__vector unsigned short __builtin_spu_sra(__vector unsigned short arg0, unsigned int arg1);
__vector signed short __builtin_spu_sra(__vector signed short arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_sra(__vector unsigned int arg0, unsigned int arg1);
__vector signed int __builtin_spu_sra(__vector signed int arg0, unsigned int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned short __builtin_spu_sra_0(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_sra_1(__vector signed short arg0, __vector unsigned short arg1);
__vector unsigned int __builtin_spu_sra_2(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_sra_3(__vector signed int arg0, __vector unsigned int arg1);
__vector unsigned short __builtin_spu_sra_4(__vector unsigned short arg0, unsigned int arg1);
__vector signed short __builtin_spu_sra_5(__vector signed short arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_sra_6(__vector unsigned int arg0, unsigned int arg1);
__vector signed int __builtin_spu_sra_7(__vector signed int arg0, unsigned int arg1);
#if __cplusplus
__vector signed long long __builtin_spu_srqw(__vector signed long long arg0, unsigned int arg1);
__vector unsigned long long __builtin_spu_srqw(__vector unsigned long long arg0, unsigned int arg1);
__vector signed int __builtin_spu_srqw(__vector signed int arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_srqw(__vector unsigned int arg0, unsigned int arg1);
__vector signed short __builtin_spu_srqw(__vector signed short arg0, unsigned int arg1);
__vector unsigned short __builtin_spu_srqw(__vector unsigned short arg0, unsigned int arg1);
__vector signed char __builtin_spu_srqw(__vector signed char arg0, unsigned int arg1);
__vector unsigned char __builtin_spu_srqw(__vector unsigned char arg0, unsigned int arg1);
__vector float __builtin_spu_srqw(__vector float arg0, unsigned int arg1);
__vector double __builtin_spu_srqw(__vector double arg0, unsigned int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed long long __builtin_spu_srqw_0(__vector signed long long arg0, unsigned int arg1);
__vector unsigned long long __builtin_spu_srqw_1(__vector unsigned long long arg0, unsigned int arg1);
__vector signed int __builtin_spu_srqw_2(__vector signed int arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_srqw_3(__vector unsigned int arg0, unsigned int arg1);
__vector signed short __builtin_spu_srqw_4(__vector signed short arg0, unsigned int arg1);
__vector unsigned short __builtin_spu_srqw_5(__vector unsigned short arg0, unsigned int arg1);
__vector signed char __builtin_spu_srqw_6(__vector signed char arg0, unsigned int arg1);
__vector unsigned char __builtin_spu_srqw_7(__vector unsigned char arg0, unsigned int arg1);
__vector float __builtin_spu_srqw_8(__vector float arg0, unsigned int arg1);
__vector double __builtin_spu_srqw_9(__vector double arg0, unsigned int arg1);
#if __cplusplus
__vector signed long long __builtin_spu_srqwbyte(__vector signed long long arg0, unsigned int arg1);
__vector unsigned long long __builtin_spu_srqwbyte(__vector unsigned long long arg0, unsigned int arg1);
__vector signed int __builtin_spu_srqwbyte(__vector signed int arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_srqwbyte(__vector unsigned int arg0, unsigned int arg1);
__vector signed short __builtin_spu_srqwbyte(__vector signed short arg0, unsigned int arg1);
__vector unsigned short __builtin_spu_srqwbyte(__vector unsigned short arg0, unsigned int arg1);
__vector signed char __builtin_spu_srqwbyte(__vector signed char arg0, unsigned int arg1);
__vector unsigned char __builtin_spu_srqwbyte(__vector unsigned char arg0, unsigned int arg1);
__vector float __builtin_spu_srqwbyte(__vector float arg0, unsigned int arg1);
__vector double __builtin_spu_srqwbyte(__vector double arg0, unsigned int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed long long __builtin_spu_srqwbyte_0(__vector signed long long arg0, unsigned int arg1);
__vector unsigned long long __builtin_spu_srqwbyte_1(__vector unsigned long long arg0, unsigned int arg1);
__vector signed int __builtin_spu_srqwbyte_2(__vector signed int arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_srqwbyte_3(__vector unsigned int arg0, unsigned int arg1);
__vector signed short __builtin_spu_srqwbyte_4(__vector signed short arg0, unsigned int arg1);
__vector unsigned short __builtin_spu_srqwbyte_5(__vector unsigned short arg0, unsigned int arg1);
__vector signed char __builtin_spu_srqwbyte_6(__vector signed char arg0, unsigned int arg1);
__vector unsigned char __builtin_spu_srqwbyte_7(__vector unsigned char arg0, unsigned int arg1);
__vector float __builtin_spu_srqwbyte_8(__vector float arg0, unsigned int arg1);
__vector double __builtin_spu_srqwbyte_9(__vector double arg0, unsigned int arg1);
#if __cplusplus
__vector signed long long __builtin_spu_srqwbytebc(__vector signed long long arg0, unsigned int arg1);
__vector unsigned long long __builtin_spu_srqwbytebc(__vector unsigned long long arg0, unsigned int arg1);
__vector signed int __builtin_spu_srqwbytebc(__vector signed int arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_srqwbytebc(__vector unsigned int arg0, unsigned int arg1);
__vector signed short __builtin_spu_srqwbytebc(__vector signed short arg0, unsigned int arg1);
__vector unsigned short __builtin_spu_srqwbytebc(__vector unsigned short arg0, unsigned int arg1);
__vector signed char __builtin_spu_srqwbytebc(__vector signed char arg0, unsigned int arg1);
__vector unsigned char __builtin_spu_srqwbytebc(__vector unsigned char arg0, unsigned int arg1);
__vector float __builtin_spu_srqwbytebc(__vector float arg0, unsigned int arg1);
__vector double __builtin_spu_srqwbytebc(__vector double arg0, unsigned int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector signed long long __builtin_spu_srqwbytebc_0(__vector signed long long arg0, unsigned int arg1);
__vector unsigned long long __builtin_spu_srqwbytebc_1(__vector unsigned long long arg0, unsigned int arg1);
__vector signed int __builtin_spu_srqwbytebc_2(__vector signed int arg0, unsigned int arg1);
__vector unsigned int __builtin_spu_srqwbytebc_3(__vector unsigned int arg0, unsigned int arg1);
__vector signed short __builtin_spu_srqwbytebc_4(__vector signed short arg0, unsigned int arg1);
__vector unsigned short __builtin_spu_srqwbytebc_5(__vector unsigned short arg0, unsigned int arg1);
__vector signed char __builtin_spu_srqwbytebc_6(__vector signed char arg0, unsigned int arg1);
__vector unsigned char __builtin_spu_srqwbytebc_7(__vector unsigned char arg0, unsigned int arg1);
__vector float __builtin_spu_srqwbytebc_8(__vector float arg0, unsigned int arg1);
__vector double __builtin_spu_srqwbytebc_9(__vector double arg0, unsigned int arg1);
#if __cplusplus
__vector unsigned short __builtin_spu_sub(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_sub(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_sub(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_sub(__vector signed int arg0, __vector signed int arg1);
__vector float __builtin_spu_sub(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_sub(__vector double arg0, __vector double arg1);
__vector unsigned short __builtin_spu_sub(unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_sub(signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_sub(unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_sub(signed int arg0, __vector signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned short __builtin_spu_sub_0(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_sub_1(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_sub_2(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_sub_3(__vector signed int arg0, __vector signed int arg1);
__vector float __builtin_spu_sub_4(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_sub_5(__vector double arg0, __vector double arg1);
__vector unsigned short __builtin_spu_sub_6(unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_sub_7(signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_sub_8(unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_sub_9(signed int arg0, __vector signed int arg1);
#if __cplusplus
__vector unsigned int __builtin_spu_subx(__vector unsigned int arg0, __vector unsigned int arg1, __vector unsigned int arg2);
__vector signed int __builtin_spu_subx(__vector signed int arg0, __vector signed int arg1, __vector signed int arg2);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned int __builtin_spu_subx_0(__vector unsigned int arg0, __vector unsigned int arg1, __vector unsigned int arg2);
__vector signed int __builtin_spu_subx_1(__vector signed int arg0, __vector signed int arg1, __vector signed int arg2);
#if __cplusplus
__vector unsigned char __builtin_spu_xor(__vector unsigned char arg0, __vector unsigned char arg1);
__vector signed char __builtin_spu_xor(__vector signed char arg0, __vector signed char arg1);
__vector unsigned short __builtin_spu_xor(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_xor(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_xor(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_xor(__vector signed int arg0, __vector signed int arg1);
__vector unsigned long long __builtin_spu_xor(__vector unsigned long long arg0, __vector unsigned long long arg1);
__vector signed long long __builtin_spu_xor(__vector signed long long arg0, __vector signed long long arg1);
__vector float __builtin_spu_xor(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_xor(__vector double arg0, __vector double arg1);
__vector unsigned char __builtin_spu_xor(__vector unsigned char arg0, unsigned char arg1);
__vector signed char __builtin_spu_xor(__vector signed char arg0, signed char arg1);
__vector unsigned short __builtin_spu_xor(__vector unsigned short arg0, unsigned short arg1);
__vector signed short __builtin_spu_xor(__vector signed short arg0, signed short arg1);
__vector unsigned int __builtin_spu_xor(__vector unsigned int arg0, unsigned int arg1);
__vector signed int __builtin_spu_xor(__vector signed int arg0, signed int arg1);
#else
  /* No generic function generated since return types differ */
#endif
__vector unsigned char __builtin_spu_xor_0(__vector unsigned char arg0, __vector unsigned char arg1);
__vector signed char __builtin_spu_xor_1(__vector signed char arg0, __vector signed char arg1);
__vector unsigned short __builtin_spu_xor_2(__vector unsigned short arg0, __vector unsigned short arg1);
__vector signed short __builtin_spu_xor_3(__vector signed short arg0, __vector signed short arg1);
__vector unsigned int __builtin_spu_xor_4(__vector unsigned int arg0, __vector unsigned int arg1);
__vector signed int __builtin_spu_xor_5(__vector signed int arg0, __vector signed int arg1);
__vector unsigned long long __builtin_spu_xor_6(__vector unsigned long long arg0, __vector unsigned long long arg1);
__vector signed long long __builtin_spu_xor_7(__vector signed long long arg0, __vector signed long long arg1);
__vector float __builtin_spu_xor_8(__vector float arg0, __vector float arg1);
__vector double __builtin_spu_xor_9(__vector double arg0, __vector double arg1);
__vector unsigned char __builtin_spu_xor_10(__vector unsigned char arg0, unsigned char arg1);
__vector signed char __builtin_spu_xor_11(__vector signed char arg0, signed char arg1);
__vector unsigned short __builtin_spu_xor_12(__vector unsigned short arg0, unsigned short arg1);
__vector signed short __builtin_spu_xor_13(__vector signed short arg0, signed short arg1);
__vector unsigned int __builtin_spu_xor_14(__vector unsigned int arg0, unsigned int arg1);
__vector signed int __builtin_spu_xor_15(__vector signed int arg0, signed int arg1);
/* END: Generated from spu-builtins.def */

#endif /* COVERITY_SPU_COMPILER_2_5 */
#if __COVERITY_GCC_VERSION_AT_LEAST(4, 0)
int __builtin_va_arg_pack();
__COVERITY_SIZE_TYPE__ __builtin_va_arg_pack_len();

//Added with GCC 4.7
void *__builtin_assume_aligned (const void *exp, __COVERITY_SIZE_TYPE__ align, ...);
#endif

#endif /* __COVERITY_DISABLE_BUILTIN_DECLS */

#if (__GNUC__ < 4)
typedef int __g77_integer;
typedef unsigned int __g77_uinteger;
typedef long int __g77_longint;
typedef unsigned long int __g77_ulongint;
#endif

#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
__cov_m512 __builtin_ia32_4fmaddps (__cov_v16sf, __cov_v16sf, __cov_v16sf, __cov_v16sf, __cov_v16sf, const __cov_v4sf *);
__cov_m512 __builtin_ia32_4fmaddps_mask (__cov_v16sf, __cov_v16sf, __cov_v16sf, __cov_v16sf, __cov_v16sf, const __cov_v4sf *, __cov_v16sf, unsigned short);
__cov_m128 __builtin_ia32_4fmaddss (__cov_v4sf, __cov_v4sf, __cov_v4sf, __cov_v4sf, __cov_v4sf, const __cov_v4sf *);
__cov_m128 __builtin_ia32_4fmaddss_mask (__cov_v4sf, __cov_v4sf, __cov_v4sf, __cov_v4sf, __cov_v4sf, const __cov_v4sf *, __cov_v4sf, unsigned char);
__cov_m512 __builtin_ia32_4fnmaddps (__cov_v16sf, __cov_v16sf, __cov_v16sf, __cov_v16sf, __cov_v16sf, const __cov_v4sf *);
__cov_m512 __builtin_ia32_4fnmaddps_mask (__cov_v16sf, __cov_v16sf, __cov_v16sf, __cov_v16sf, __cov_v16sf, const __cov_v4sf *, __cov_v16sf, unsigned short);
__cov_m128 __builtin_ia32_4fnmaddss (__cov_v4sf, __cov_v4sf, __cov_v4sf, __cov_v4sf, __cov_v4sf, const __cov_v4sf *);
__cov_m128 __builtin_ia32_4fnmaddss_mask (__cov_v4sf, __cov_v4sf, __cov_v4sf, __cov_v4sf, __cov_v4sf, const __cov_v4sf *, __cov_v4sf, unsigned char);
__cov_m512i __builtin_ia32_vp4dpwssd (__cov_v16si, __cov_v16si, __cov_v16si, __cov_v16si, __cov_v16si, const __cov_v4si *);
__cov_m512i __builtin_ia32_vp4dpwssd_mask (__cov_v16si, __cov_v16si, __cov_v16si, __cov_v16si, __cov_v16si, const __cov_v4si *, __cov_v16si, unsigned short);
__cov_m512i __builtin_ia32_vp4dpwssds (__cov_v16si, __cov_v16si, __cov_v16si, __cov_v16si, __cov_v16si, const __cov_v4si *);
__cov_m512i __builtin_ia32_vp4dpwssds_mask (__cov_v16si, __cov_v16si, __cov_v16si, __cov_v16si, __cov_v16si, const __cov_v4si *, __cov_v16si, unsigned short);
__cov_m512i __builtin_ia32_vpopcountd_v16si (__cov_v16si);
__cov_m512i __builtin_ia32_vpopcountd_v16si_mask (__cov_v16si, __cov_v16si, unsigned short);
__cov_m512i __builtin_ia32_vpopcountq_v8di (__cov_v8di);
__cov_m512i __builtin_ia32_vpopcountq_v8di_mask (__cov_v8di, __cov_v8di, unsigned char);
unsigned int __builtin_ia32_kmovw(unsigned short);
unsigned char __builtin_ia32_ktestcsi (unsigned int, unsigned int);
unsigned char __builtin_ia32_ktestzsi (unsigned int, unsigned int);
unsigned char __builtin_ia32_ktestcdi (unsigned long long, unsigned long long);
unsigned char __builtin_ia32_ktestzdi (unsigned long long, unsigned long long);
unsigned char __builtin_ia32_kortestcsi (unsigned int, unsigned int);
unsigned char __builtin_ia32_kortestzsi (unsigned int, unsigned int);
unsigned char __builtin_ia32_kortestcdi (unsigned long long, unsigned long long);
unsigned char __builtin_ia32_kortestzdi (unsigned long long, unsigned long long);
unsigned int __builtin_ia32_kaddsi (unsigned int, unsigned int);
unsigned long long __builtin_ia32_kadddi (unsigned long long, unsigned long long);
unsigned int __builtin_ia32_kmovd (unsigned int);
unsigned long long __builtin_ia32_kmovq (unsigned long long);
unsigned int __builtin_ia32_knotsi (unsigned int);
unsigned long long __builtin_ia32_knotdi (unsigned long long);
unsigned int __builtin_ia32_korsi (unsigned int, unsigned int);
unsigned long long __builtin_ia32_kordi (unsigned long long, unsigned long long);
unsigned int __builtin_ia32_kxnorsi (unsigned int, unsigned int);
unsigned long long __builtin_ia32_kxnordi (unsigned long long, unsigned long long);
unsigned int __builtin_ia32_kxorsi (unsigned int, unsigned int);
unsigned long long __builtin_ia32_kxordi (unsigned long long, unsigned long long);
unsigned int __builtin_ia32_kandsi (unsigned int, unsigned int);
unsigned long long __builtin_ia32_kanddi (unsigned long long, unsigned long long);
unsigned int __builtin_ia32_kandnsi (unsigned int, unsigned int);
unsigned long long __builtin_ia32_kandndi (unsigned long long, unsigned long long);
unsigned char __builtin_ia32_ktestcqi (unsigned char, unsigned char);
unsigned char __builtin_ia32_ktestzqi (unsigned char, unsigned char);
unsigned char __builtin_ia32_ktestchi (unsigned short, unsigned short);
unsigned char __builtin_ia32_ktestzhi (unsigned short, unsigned short);
unsigned char __builtin_ia32_kortestcqi (unsigned char, unsigned char);
unsigned char __builtin_ia32_kortestzqi (unsigned char, unsigned char);
unsigned char __builtin_ia32_kaddqi (unsigned char, unsigned char);
unsigned short __builtin_ia32_kaddhi (unsigned short, unsigned short);
unsigned int __builtin_ia32_kmovb (unsigned char);
unsigned char __builtin_ia32_knotqi (unsigned char);
unsigned char __builtin_ia32_korqi (unsigned char, unsigned char);
unsigned char __builtin_ia32_kxnorqi (unsigned char, unsigned char);
unsigned char __builtin_ia32_kxorqi (unsigned char, unsigned char);
unsigned char __builtin_ia32_kandqi (unsigned char, unsigned char);
unsigned char __builtin_ia32_kandnqi (unsigned char, unsigned char);
unsigned short __builtin_ia32_lzcnt_u16 (unsigned short);
unsigned int __builtin_ia32_lzcnt_u32 (unsigned int);
unsigned long long __builtin_ia32_lzcnt_u64 (unsigned long long);
unsigned short __builtin_ia32_tzcnt_u16 (unsigned short);
unsigned int __builtin_ia32_tzcnt_u32 (unsigned int);
unsigned long long __builtin_ia32_tzcnt_u64 (unsigned long long);
unsigned int __builtin_ia32_rdpid ();
unsigned short __builtin_ia32_kshiftlihi (unsigned short, unsigned char);
unsigned short __builtin_ia32_kshiftrihi (unsigned short, unsigned char);
unsigned int __builtin_ia32_kshiftlisi (unsigned int, unsigned char);
unsigned long long __builtin_ia32_kshiftlidi (unsigned long long, unsigned char);
unsigned int __builtin_ia32_kshiftrisi (unsigned int, unsigned char);
unsigned long long __builtin_ia32_kshiftridi (unsigned long long, unsigned char);
unsigned char __builtin_ia32_kshiftliqi (unsigned char, unsigned char);
unsigned char __builtin_ia32_kshiftriqi (unsigned char, unsigned char);
#ifdef __cplusplus
template<typename T>
T* __builtin_addressof(T);
#else // __cplusplus
void* __builtin_addressof();
#endif // __cplusplus
#endif // __COVERITY_GCC_VERSION_AT_LEAST(7, 0)

#ifdef __COVERITY_CILKPLUS
#define _Cilk_spawn
#define _Cilk_sync
#define _Cilk_for for
#endif /* __cilk */



#pragma builtin end
#endif /* COVERITY_COMPAT_d682614b494b1ee17acf3e5fa6fb52f8 */
