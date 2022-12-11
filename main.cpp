////////////////////////////////
// NOTE(oleksii): C-library headers. Type aliases.
#include <stdio.h>
#include <stdint.h>
typedef int8_t    I8;
typedef int16_t   I16;
typedef int32_t   I32;
typedef int64_t   I64;
typedef uint8_t   U8;
typedef uint16_t  U16;
typedef uint32_t  U32;
typedef uint64_t  U64;
typedef uintptr_t Umm;
typedef float     F32;
typedef double    F64;
typedef I32       B32;
#define DEBUG_Assert(expr)\
if(expr){}\
else{\
*(I32 *)0 = 0x1;\
}


#define MemDefaultReserveSize 4*(1ull << 30) // 4GB
#define MemDefaultCommitSize (1ull << 20) // 1MB

////////////////////////////////
// NOTE(oleksii): Os
struct Os{
    void *(*reserve_memory)(U64 size);
    void (*commit_memory)(void *ptr, U64 size);
    void (*decommit_memory)(void *ptr, U64 size);
    void (*release_memory)(void *ptr);
};

#include "arena.h"

////////////////////////////////
// NOTE(oleksii): Debug output
#include <windows.h>
#define DEBUG_Out(s) OutputDebugStringA((char *)(s))
#define DEBUG_Outf(fmt, ...)\
{\
char buf[8*(1 << 10)];\
sprintf_s(buf, sizeof(buf), (char *)(fmt), ## __VA_ARGS__);\
DEBUG_Out((char *)buf);\
}


////////////////////////////////
// NOTE(oleksii): Os memory
static void *w32_reserve_memory(U64 size=MemDefaultReserveSize){
    void *result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
    return(result);
}

static void w32_commit_memory(void *ptr, U64 size=MemDefaultCommitSize){
    VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
}

// NOTE(oleksii): After the operation the pages are in the reserved state.
static void w32_decommit_memory(void *ptr, U64 size=MemDefaultCommitSize){
    // TODO(oleksii): Investigate that. I'm not sure that this is right way of doing it.
    VirtualFree(ptr, size, MEM_DECOMMIT);
}

static void w32_release_memory(void *ptr){
    VirtualFree(ptr, 0, MEM_RELEASE);
}


////////////////////////////////
// NOTE(oleksii): K&R's memory allocator
#define ALLOC_SIZE 4*(1ull << 10)

static U8 alloc_buf[ALLOC_SIZE];
static U8 *alloc_ptr = alloc_buf;

static U8 *alloc_align(U64 size, U64 alignment=4){
    U8 *start_ptr = alloc_buf;
    U8 *end_ptr = (alloc_buf + ALLOC_SIZE);
    U64 new_size = size;
    U64 alignment_offset = 0;
    U64 alignment_mask = (alignment - 1);
    if(new_size & alignment_mask){ // has to aligned!
        alignment_offset = alignment - (new_size & alignment_mask);
        new_size += alignment_offset;
    }
    
    U8 *result = 0;
    if((end_ptr - alloc_ptr) >= new_size){
        result = alloc_ptr;
        alloc_ptr += new_size; // adjusted size due to the alignment.
    }
    return(result);
}

static U8 *alloc(U64 size){
    // NOTE(oleksii): I don't want to introduce a call to malloc here!!!
    U8 *result = 0;
    U8 *end_ptr = (alloc_buf + ALLOC_SIZE);
    DEBUG_Assert((end_ptr - alloc_buf) == ALLOC_SIZE);
    if((end_ptr - alloc_ptr) >= size){
        result = alloc_ptr;
        alloc_ptr += size;
    }
    return(result);
}

static void afree(U8 *ptr){
    U8 *start_ptr = alloc_buf;
    U8 *end_ptr = (alloc_buf + ALLOC_SIZE);
    if((ptr >= start_ptr) && (ptr < end_ptr)){
        alloc_ptr = ptr;
    }
}


#include "arena.cpp"


////////////////////////////////
// NOTE(oleksii): Entry point
INT WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int show_code){
#if 0
    {
        I32 count = (1 << 10); // 1KB
        I32 *ptr = (I32 *)malloc(sizeof(I32)*count);
        for(I32 i = 0; i < count; i += 1){
            ptr[i] = i;
        }
        for(I32 i = 0; i < count; i += 1){
            DEBUG_Outf("%i, %i\n", i, ptr[i]);
        }
        
        
        ////////////////////////////////
        // NOTE(oleksii): Pointer arithmetic
        I32 *end_ptr = ptr + count;
        I32 array_count = end_ptr - ptr;
        DEBUG_Assert(count == array_count);
        
        Umm ptr_v     = (Umm)ptr;
        Umm end_ptr_v = (Umm)end_ptr;
        
        I32 reverse_count = ptr - end_ptr;
        DEBUG_Assert(-count == reverse_count);
        
        
        // Let's get a value which is stored in ptr[1];
        I32 v = *((U8 *)ptr);
        DEBUG_Assert(!v);
        
        I32 v1_ = *(ptr + 1);
        I32 v1 = *((U8 *)ptr + sizeof(I32));
        DEBUG_Assert(v1_ == v1);
        
        I32 *p0 = ptr + 5;
        I32 *p1 = ptr + 10; // higher address!
        
        Umm p0_v = (Umm)p0;
        Umm p1_v = (Umm)p1;
        Umm p1p0_dif = (p1_v - p0_v) / sizeof(I32);
        
        DEBUG_Assert((p1 - p0) == 5);
        
        // NOTE(oleksii): Pointer to 100th element in the array
        void *ptr0 = (void *)((U8 *)p1 + (90 * sizeof(I32)));
        DEBUG_Assert(*(I32 *)ptr0 == 100);
        
        
        ////////////////////////////////
        // NOTE(oleksii): This is only true when two pointers point to the same array (memory).
        // Otherwise the behavior is undefined!!!
    }
#endif
    
#if 0
    {
        ////////////////////////////////
        // NOTE(oleksii): K&R's memory allocator test
        U64 count = (1 << 3);
        U8 *ptr = alloc(count);
        for(I32 i = 0; i < count; i += 1){
            ptr[i] = (U8)i;
        }
        afree(ptr);
        DEBUG_Assert(alloc_ptr == alloc_buf);
        
        // NOTE(oleksii): Test edge case
        count = ALLOC_SIZE;
        ptr = alloc(count);
        for(I32 i = 0; i < count; i += 1){
            *(ptr + i) = i;
        }
        
        U8 *ptr0 = alloc(1);
        DEBUG_Assert(!ptr0);
        
        afree(ptr);
        DEBUG_Assert(alloc_ptr == alloc_buf);
        
        count = (ALLOC_SIZE / 4); // 1KB
        ptr = alloc(count);
        
        // NOTE(oleksii): Should fail (return NULL pointer).
        count = (ALLOC_SIZE);
        U8 *null_ptr = alloc(count);
        DEBUG_Assert(!null_ptr);
        afree(ptr);
        DEBUG_Assert(ptr == alloc_buf);
        
        
        ////////////////////////////////
        // NOTE(oleksii): Alignment.
        U64 alignment = 4u;
        count = 7;
        ptr = alloc_align(count, alignment);
        DEBUG_Assert((alloc_ptr - ptr) == 8);
        
        alignment = (1 << 10); // 1KB
        ptr = alloc_align(count, alignment);
        DEBUG_Assert((alloc_ptr - ptr) == 1024);
    }
#endif
    
    Os os;
    os.reserve_memory  = w32_reserve_memory;
    os.commit_memory   = w32_commit_memory;
    os.decommit_memory = w32_decommit_memory;
    os.release_memory  = w32_release_memory;
    
    os_ptr = &os;
    
    {
        // NOTE(oleksii): Testing the Arena:
        Arena arena = make_arena();
        I32 count = (1 << 10); // 1KB
        I32 *i32_ptr = ArenaPushArray(&arena, I32, count);
        DEBUG_Assert(i32_ptr);
        
        for(I32 i = 0; i < count; i += 1){
            i32_ptr[i] = i*i;
        }
        for(I32 i = 0; i < count; i += 1){
            DEBUG_Outf("%i, %i\n", i, i32_ptr[i]);
        }
        
        // NOTE(oleksii): Doesn't have real value, it's just for testing purposes.
#pragma pack(push, 1)
        struct TestStruct{
            U8 c;
            U64 i;
            U64 j;
            U64 k;
            void *ptr;
            
            struct{
                U64 array[1 << 10];
                void *memory;
                void *memory_at;
                U64 used;
            };
        };
#pragma pack(pop)
        DEBUG_Outf("size: %llu\n", sizeof(TestStruct));
        
        
        // NOTE(oleksii): Pushing the struct to the arena.
        // We could have used TestStruct instead of void *
        void *struct_ptr = ArenaPushStruct(&arena, TestStruct);
        
        TestStruct ts = {};
        ts.i = 1024;
        ts.j = ts.i*2;
        ts.k = ts.j*3;
        
        *(TestStruct *)struct_ptr = ts;
        DEBUG_Assert(((TestStruct *)struct_ptr)->i == 1024); 
        
        
        // NOTE(oleksii): Reading the contents of the file into memory arena!
        U8 *file_name = (U8 *)("e:/work/arenas/code/arena.cpp");
        FileReadResult file_result = read_entire_file_into_memory(&arena, file_name);
        DEBUG_Out(file_result.data);
        
        
        // NOTE(oleksii): Testing temporary arenas.
        {
            void *ptr_before_tmp_arena = arena.alloc_ptr;
            TempArena temp_arena = temp_arena_start(&arena);
            I32 count = 4*(1 << 10);
            TestStruct *ts_ptr = ArenaPushArray(&arena, TestStruct, count);
            for(I32 i = 0; i < count; i += 1){
                TestStruct ts = {};
                ts.i = i;
                ts.j = i*ts.i;
                ts.k = i*ts.j;
                
                // put into the memory.
                ts_ptr[i] = ts;
            }
            U8 *fmt = 
                (U8 *)("ts_ptr[%i].i = %i\n"
                       "ts_ptr[%i].j = %i\n"
                       "ts_ptr[%i].k = %i\n\n");
            for(I32 i = 0; i < count; i += 1){
                DEBUG_Outf(fmt, ts_ptr[i].i, ts_ptr[i].j, ts_ptr[i].k);
            }
            
            temp_arena_end(temp_arena);
            DEBUG_Assert(ptr_before_tmp_arena == arena.alloc_ptr);
        }
    }
    
    return(0);
}