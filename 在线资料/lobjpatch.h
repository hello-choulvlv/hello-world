//
//  lobjpatch.h
//  cocos2d_lua_bindings
//
//  Created by playcrab on 29/10/15.
//
//

#ifndef lobjpatch_h
#define lobjpatch_h

// for testing
#define LUA_ENCRYPT_NUMBER 1

#if LUA_ENCRYPT_NUMBER

#ifndef LUA_NUMBER_KEY
#   define LUA_NUMBER_KEY 0x86a3d75c693b695e
#endif

#ifdef setnvalue
#   undef setnvalue
#endif
#ifdef nvalue
#   undef nvalue
#endif

/*
 ** Union of all Lua values
 */
typedef union {
    GCObject *gc;
    void *p;
    lua_Number n;
    int b;
    
    // works only when sizeof(lua_Number)==8
    unsigned int u32[2];
    unsigned long long u64;
} ValueX;

typedef struct lua_TValueX {
    ValueX value; int tt;
} TValueX;

// universal implementation

#define setnvalue(obj, x) { \
    TValueX* i_o = (TValueX*)(obj); \
    i_o->value.n = x; \
    i_o->value.u64 ^= LUA_NUMBER_KEY; \
    i_o->tt=LUA_TNUMBER; \
}

#define nvalue(o)  check_exp(ttisnumber(o), \
    ({ \
        union { lua_Number n; long long i; } tmp; \
        tmp.i = ((TValueX*)(o))->value.u64 ^ LUA_NUMBER_KEY; \
        tmp.n; \
    }) \
)

// optimizations
#if defined(__APPLE__)

#if defined(__arm64__)

#   undef setnvalue
#   undef nvalue

#   define setnvalue(obj, x) { \
    TValueX* i_o = (TValueX*)(obj); \
    __asm__ __volatile__(\
        "fmov %[dst], %d[src]\n\t" \
        "eor  %[dst], %[dst], %[key]\n\t" \
        : [dst]"=&r"((i_o)->value.u64) : [src]"w"(x), [key]"r"(LUA_NUMBER_KEY&0xffffffff00000000): ); \
    (i_o)->tt = LUA_TNUMBER; \
}

#   define nvalue(o) check_exp(ttisnumber(o), \
    ({ register lua_Number x; \
    register unsigned long long _tmp = ((TValueX*)(o))->value.u64; \
    __asm__ __volatile__(\
        "eor  %[tmp], %[tmp], %[key]\n\t" \
        "fmov %d[dst], %[tmp]\n\t" \
        : [dst]"=w"(x), [tmp]"+&r"(_tmp) : [key]"r"(LUA_NUMBER_KEY&0xffffffff00000000): ); \
    x; }) \
)

#elif defined(__arm__)

#   undef setnvalue
#   undef nvalue

#   define setnvalue(obj, x) { \
    TValueX* i_o = (TValueX*)(obj); \
    __asm__ __volatile__(\
        "vmov   %[dst0], %[dst1], %[src]\n\t" \
        /*"eor    %[dst0], %[dst0], %[key]\n\t"*/ \
        "eor    %[dst1], %[dst1], %[key]\n\t" \
        : [dst0]"=&r"((i_o)->value.u32[0]), [dst1]"=&r"((i_o)->value.u32[1]) \
        : [src]"w"(x), [key]"r"(LUA_NUMBER_KEY>>32) \
        : ); \
    (i_o)->tt = LUA_TNUMBER; \
}

#   define nvalue(obj) check_exp(ttisnumber(obj), \
    ({ register lua_Number x; \
    register unsigned int _tmp0,_tmp1; \
    __asm__ __volatile__(\
        "ldrd	%[src0], %[src1], [%[psrc]]\n\t" \
        /*"eor    %[src0], %[src0], %[key]\n\t"*/ \
        "eor    %[src1], %[src1], %[key]\n\t" \
        "vmov   %[dst], %[src0], %[src1]\n\t" \
        : [dst]"=w"(x), [src0]"+&r"(_tmp0), [src1]"+&r"(_tmp1) \
        : [psrc]"r"(((const TValueX*)(obj))->value.u32), [key]"r"(LUA_NUMBER_KEY>>32) \
        : ); \
    x; }) \
)

#elif defined(__x86_64__)

#   undef setnvalue
#   undef nvalue

#   define setnvalue(obj, x) { \
    TValue *i_o=(obj); \
    __asm__ __volatile__("xorq %2, %0" : "=r"((i_o)->value.n) : "0"(x),"r"(LUA_NUMBER_KEY)); \
    (i_o)->tt = LUA_TNUMBER; \
}

#   define nvalue(o) check_exp(ttisnumber(o), \
    ({ register lua_Number x; __asm__ __volatile__("xorq %2, %0" : "=r"(x) : "0"((o)->value.n),"r"(LUA_NUMBER_KEY)); x; }) \
)

#elif defined(__i386__)
    // no optimizations
#endif

#elif defined(ANDROID)

// no optimizations

#endif // defined(__APPLE__)

//#define luai_numadd(a,b)	((a)+(b))
//#define luai_numsub(a,b)	((a)-(b))
//#define luai_nummul(a,b)	((a)*(b))
//#define luai_numdiv(a,b)	((a)/(b))
//#define luai_nummod(a,b)	((a) - floor((a)/(b))*(b))
//#define luai_numpow(a,b)	(pow(a,b))
//#define luai_numunm(a)		(-(a))
//#define luai_numeq(a,b)		((a)==(b))
//#define luai_numlt(a,b)		((a)<(b))
//#define luai_numle(a,b)		((a)<=(b))
//#define luai_numisnan(a)	(!luai_numeq((a), (a)))

#undef luai_nummod
#define luai_nummod(a,b)	({lua_Number _a=(a);lua_Number _b=(b);(_a) - floor((_a)/(_b))*(_b);})
#undef luai_numisnan
#define luai_numisnan(a)	({lua_Number _a=(a);!luai_numeq((_a), (_a));})

#endif // LUA_ENCRYPT_NUMBER

#endif // lobjpatch_h
