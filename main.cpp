// TODO(oleksii): 
//
// [x] Explain the problem to people
// [x] Implement type-specific doubly linked list first.
//     [x] push_front (push a node to the front of the list)
//     [x] push_back (push a node to the back of the list)
//     [x] pop_front (pop from the front, if exists)
//     [x] pop_back (pop from the back, if exists)
//     [x] pop_node (remove an a node from the list)
//     [x] front (get first element in the list)
//     [x] back (get last element in the list)
//     [x] Implement Queue based on DLL(doubly linked list)
//     [x] Implement Stack based on DLL
// [x] Implement type-independent doubly linked list/queue/stack
// [x] Test those things
// [x] Should the implementaion of DLL allocate memory, or an arena itself.

// NOTE(oleksii): Think about it more!
// list_front() can be called before list_pop_front() to receive the element which is about to be poped
// list_back() can be called before list_pop_back() to receive the element which is about to be poped


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

#define global static
#define function static
#define local_persist static

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

/*
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
    */

struct List{
    List *next;
    List *prev;
    
    // data
    struct{
        I32 i32;
    };
};

struct AnotherList{
    AnotherList *next;
    AnotherList *prev;
    
    // data
    struct{
        I32 i32;
        F32 f32;
        struct{
            U8 name[1 << 10];
            U32 used;
        };
    };
};

function List *list_front(List *sentinel){
    return(sentinel->next);
}

function List *list_back(List *sentinel){
    return(sentinel->prev);
}

function void list_push_front(List *sentinel, List *node){
    if(!sentinel->next && !sentinel->prev){
        node->next = sentinel;
        node->prev = sentinel;
        sentinel->next = sentinel->prev = node;
    }
    else{
        node->next = sentinel->next;
        node->prev = sentinel;
        sentinel->next->prev = node;
        sentinel->next = node;
    }
}

function void list_push_back(List *sentinel, List *node){
    if(!sentinel->next && !sentinel->prev){
        node->next = sentinel;
        node->prev = sentinel;
        sentinel->next = sentinel->prev = node;
    }
    else{
        node->next = sentinel;
        node->prev = sentinel->prev;
        sentinel->prev->next = node;
        sentinel->prev = node;
    }
}

function void list_pop_front(List *sentinel){
    if(sentinel->next && sentinel->prev){
        List *snn = sentinel->next->next;
        snn->prev = sentinel;
        sentinel->next = snn;
    }
}

function void list_pop_back(List *sentinel){
    if(sentinel->next && sentinel->prev){
        List *spp = sentinel->prev->prev;
        spp->next = sentinel;
        sentinel->prev = spp;
    }
}

function void list_pop_node(List *sentinel, List *node){
    if(sentinel->next && sentinel->prev){
        node->next->prev = node->prev;
        node->prev->next = node->next;
    }
}

// First in first out
function void queue_push(List *sentinel, List *node){
    // If we are pushing to the front of the list, we have to pop from the back of the list
    // If we are pushing to the end of the list, we have to pop from the front of the list.
    list_push_front(sentinel, node);
}

function void queue_pop(List *sentinel){
    // If we are pushing to the front of the list, we have to pop from the back of the list
    // If we are pushing to the end of the list, we have to pop from the front of the list.
    list_pop_back(sentinel);
}

// First in last out
function void stack_push(List *sentinel, List *node){
    // if we are pushing to the front of the list, we have to pop from the front as well, 
    // If we are pushing to the end of the list, we have pop from there as well.
    list_push_front(sentinel, node);
}

function void stack_pop(List *sentinel){
    // if we are pushing to the front of the list, we have to pop from the front as well, 
    // If we are pushing to the end of the list, we have pop from there as well.
    list_pop_front(sentinel);
}

function List *list_init(Arena *arena){
    List *sentinel = ArenaPushStruct(arena, List);
    sentinel->next = sentinel;
    sentinel->prev = sentinel;
    return(sentinel);
}

////////////////////////////////
// NOTE(oleksii): Type-independent list/queue/stack
// TODO(oleksii): Try to play arond with it, maybe use arena, and allocate memory directly here for the sentinel
// List *sentinel = ArenaPushStruct(&arena, List);
// DLLInit(sentinel);
#define DLLInit(sentinel)\
(sentinel)->next = (sentinel);\
(sentinel)->prev = (sentinel);

#define DLLFront(sentinel) ((sentinel)->next)
#define DLLBack(sentinel) ((sentinel)->prev)

// NOTE(oleksii): Assuming that the sentinel was initialized with DLLInit macro
#define DLLPushFront(sentinel, node)\
(node)->next = (sentinel)->next;\
(node)->prev = (sentinel);\
(sentinel)->next->prev = (node);\
(sentinel)->next = (node);

// NOTE(oleksii): Assuming that the sentinel was initialized with DLLInit macro
#define DLLPushBack(sentinel, node)\
(node)->next = (sentinel);\
(node)->prev = (sentinel)->prev;\
(sentinel)->prev->next = (node);\
(sentinel)->prev = (node);

#define DLLPopFront(sentinel)\
(sentinel)->next->next->prev = (sentinel);\
(sentinel)->next = (sentinel)->next->next;

#define DLLPopBack(sentinel)\
(sentinel)->prev->prev->next = (sentinel);\
(sentinel)->prev = (sentinel)->prev->prev;

// NOTE(oleksii): You have to make sure that node is a valid pointer to one of the elements in the DLL
#define DLLPopNode(sentinel, node)\
(node)->next->prev = (node)->prev;\
(node)->prev->next = (node)->next;


// Queue (First in first out)
#define QueuePush(sentinel, node) DLLPushFront(sentinel, node);
#define QueuePop(sentinel) DLLPopBack(sentinel); 

// Stack (First in last out)
#define StackPush(sentinel, node) DLLPushFront(sentinel, node);
#define StackPop(sentinel) DLLPopFront(sentinel);

INT WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int show_code){
    Os os = {};
    os.reserve_memory = w32_reserve_memory;
    os.commit_memory  = w32_commit_memory;
    os.decommit_memory = w32_decommit_memory;
    os.release_memory  = w32_release_memory;
    os_ptr = &os;
    
    Arena arena = make_arena(8u);
    TempArena temp_arena = temp_arena_start(&arena);
    {
        List *sentinel = ArenaPushStruct(&arena, List);
        /*
        sentinel->next = sentinel;
        sentinel->prev = sentinel;
        */
        
#define IterateList(sentinel) \
{\
for(List *node = (sentinel)->next;\
node != (sentinel); \
node = node->next){\
DEBUG_Outf("node->i32: %i\n", node->i32); \
}\
}
        
        U32 count = 4;
        for(U32 i = 0; i < count; i += 1){
            // NOTE(oleksii): Allocate the memory for the node outside the implementation of the List
            List *node = ArenaPushStruct(&arena, List);
            node->i32 = i;
            list_push_front(sentinel, node);
        }
        for(U32 i = 0; i < count; i += 1){
            List *node = ArenaPushStruct(&arena, List);
            node->i32 = i + 4;
            list_push_back(sentinel, node);
        }
        IterateList(sentinel);
        
        // NOTE(oleksii): Pop from the beginning of the list
        list_pop_front(sentinel);
        list_pop_front(sentinel);
        list_pop_front(sentinel);
        DEBUG_Out("\n\n");
        IterateList(sentinel);
        
        // NOTE(oleksii): Pop form the end of the list
        list_pop_back(sentinel);
        list_pop_back(sentinel);
        list_pop_back(sentinel);
        DEBUG_Out("\n\n");
        IterateList(sentinel);
        
        for(U32 i = 0; i < count; i += 1){
            List *node = ArenaPushStruct(&arena, List);
            node->i32 = i + 8;
            list_push_back(sentinel, node);
        }
        DEBUG_Out("\n\n");
        IterateList(sentinel);
        
        List *spp = sentinel->prev->prev;
        list_pop_node(sentinel, spp);
        
        DEBUG_Out("\n\n");
        IterateList(sentinel);
        DEBUG_Out("\n\n");
        
        ////////////////////////////////
        // NOTE(oleksii): Testing the queue
        {
            List *queue = list_init(&arena);
            U32 count = 4;
            for(U32 i = 0; i < count; i += 1){
                List *node = ArenaPushStruct(&arena, List);
                node->i32 = i;
                queue_push(queue, node);
            }
            //IterateList(queue);
            queue_pop(queue);
            queue_pop(queue);
            IterateList(queue);
            
            {
                List *node = ArenaPushStruct(&arena, List);
                node->i32 = 12;
                queue_push(queue, node);
                
                node = ArenaPushStruct(&arena, List);
                node->i32 = 13;
                queue_push(queue, node);
            }
            DEBUG_Out("\n\n");
            IterateList(queue);
            DEBUG_Out("\n\n");
        }
        
        ////////////////////////////////
        // NOTE(oleksii): Testing the stack
        {
            List *stack = list_init(&arena);
            U32 count = 4;
            for(U32 i = 0; i < count; i += 1){
                List *node = ArenaPushStruct(&arena, List);
                node->i32 = i*10;
                stack_push(stack, node);
            }
            IterateList(stack);
            DEBUG_Out("\n\nBefore pop:\n");
            stack_pop(stack);
            stack_pop(stack);
            DEBUG_Out("After pop:\n");
            IterateList(stack);
            
            {
                List *node = ArenaPushStruct(&arena, List);
                node->i32 = 40;
                stack_push(stack, node);
                
                node = ArenaPushStruct(&arena, List);
                node->i32 = 50;
                stack_push(stack, node);
            }
            DEBUG_Out("\n\n");
            IterateList(stack);
            DEBUG_Out("\n\n");
        }
#undef IterateList
        
        ////////////////////////////////
        // NOTE(oleksii): Testing using macros
        {
            struct SomeStruct{ // test structure
                SomeStruct *next;
                SomeStruct *prev;
                
                struct{
                    I32 data;
                };
            };
#define Iterate(s)\
for(SomeStruct *node = (s)->next;\
node != (s);\
node = node->next){\
DEBUG_Outf("node->data: %i\n", node->data);\
}
            SomeStruct *list = ArenaPushStruct(&arena, SomeStruct);
            DLLInit(list); // list->next=list->prev=list
            for(I32 i = 0; i < 8; i += 1){
                SomeStruct *node = ArenaPushStruct(&arena, SomeStruct);
                node->data = i;
                DLLPushFront(list, node);
            }
            Iterate(list);
            DEBUG_Outf("\n\n");
            DLLPopFront(list);
            DLLPopFront(list);
            DLLPopBack(list);
            DLLPopFront(list);
            DLLPopBack(list);
            Iterate(list);
            DEBUG_Outf("\n\n");
            
            SomeStruct *queue = ArenaPushStruct(&arena, SomeStruct);
            DLLInit(queue);
            for(U32 j = 0; j < 8; j += 1){
                SomeStruct *node = ArenaPushStruct(&arena, SomeStruct);
                node->data = j*10;
                QueuePush(queue, node);
            }
            Iterate(queue);
            DEBUG_Outf("\n\n");
            
            // let's pop and push elements from the queue
            QueuePop(queue);
            QueuePop(queue);
            QueuePop(queue);
            QueuePop(queue);
            QueuePop(queue);
            Iterate(queue);
            DEBUG_Outf("\n\n");
            
            for(U32 j = 0; j < 4; j += 1){
                SomeStruct *node = ArenaPushStruct(&arena, SomeStruct);
                node->data = (j+8)*10;
                QueuePush(queue, node);
            }
            Iterate(queue);
            DEBUG_Outf("\n\n");
            
            QueuePop(queue);
            QueuePop(queue);
            QueuePop(queue);
            QueuePop(queue);
            Iterate(queue);
            DEBUG_Outf("\n\n");
            
            SomeStruct *stack = ArenaPushStruct(&arena, SomeStruct);
            DLLInit(stack); // stack->next=stack->prev=stack
            for(U32 k = 0; k < 16; k += 1){
                SomeStruct *node = ArenaPushStruct(&arena, SomeStruct);
                node->data = k;
                StackPush(stack, node);
            }
            Iterate(stack);
            DEBUG_Outf("\n\n");
            
            StackPop(stack);
            StackPop(stack);
            StackPop(stack);
            {
                SomeStruct *node = ArenaPushStruct(&arena, SomeStruct);
                node->data = 16;
                StackPush(stack, node);
            }
            StackPop(stack);
            StackPop(stack);
            {
                SomeStruct *node = ArenaPushStruct(&arena, SomeStruct);
                node->data = 17;
                StackPush(stack, node);
            }
            Iterate(stack);
            
#undef Iterate
        }
    }
    // TODO(oleksii): Debug! Maybe we want to clear all the memory that we've used so far
    temp_arena_end(temp_arena);
    
    return(0);
}