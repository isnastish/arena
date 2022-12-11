#include "arena.h"

static Arena make_arena(U64 alignment=4u){
    Arena result;
    result.base_ptr = os_ptr->reserve_memory(MemDefaultReserveSize);
    result.alloc_ptr = result.base_ptr;
    result.max_count = MemDefaultReserveSize;
    result.commited = 0;
    result.alignment = alignment;
    result.temp_count = 0;
    return(result);
}

static void *arena_push(Arena *arena, U64 size){
    // NOTE(oleksii): This has to be revisited and tested correctly!!!
    // Because I probably has some issues!!! 
    // Debug and make sure that everything is consistent before using!!!
    
    U64 alignment_offset = 0;
    U64 alignment_mask = (arena->alignment - 1);
    U64 new_size = size;
    if(new_size & alignment_mask){
        alignment_offset = arena->alignment - (new_size & alignment_mask);
        new_size += alignment_offset;
    }
    
    void *result  = 0;
    U8 *start_ptr = (U8 *)arena->base_ptr;
    U8 *end_ptr   = (start_ptr + arena->max_count);
    U8 *alloc_ptr = (U8 *)arena->alloc_ptr;
    if((end_ptr - alloc_ptr) >= new_size){
        result = alloc_ptr;
        arena->alloc_ptr = (void *)((U8 *)arena->alloc_ptr + new_size);
        if(new_size > arena->commited){
            U64 commit_raw_amount = new_size;
            U64 commit_alignment_offset = 0;
            U64 commit_alignment_mask = (MemDefaultCommitSize - 1);
            if(commit_raw_amount & commit_alignment_mask){
                commit_alignment_offset = MemDefaultCommitSize - (commit_raw_amount & commit_alignment_mask);
                commit_raw_amount += commit_alignment_offset;
            }
            os_ptr->commit_memory(alloc_ptr, commit_raw_amount);
            arena->commited += commit_raw_amount;
        }
    }
    return(result);
}

// NOTE(oleksii): Not tested yet!
static void arena_reset_alloc_ptr(Arena *arena, void *ptr){
    U8 *reset_ptr = (U8 *)ptr;
    U8 *start_ptr = (U8 *)arena->base_ptr;
    U8 *alloc_ptr  = (U8 *)arena->alloc_ptr;
    if((ptr >= start_ptr) && (ptr < alloc_ptr)){
        arena->alloc_ptr = (void *)ptr;
    }
}

#define ArenaPushStruct(arena, type) (type *)arena_push((arena), sizeof(type)); // +
#define ArenaPushArray(arena, type, count) (type *)arena_push((arena), (sizeof(type)*count)); // + 
#define ArenaPushSize(arena, size) arena_push((arena), (size)); // + 


////////////////////////////////
// NOTE(oleksii): Temporary arena stuff here
static TempArena temp_arena_start(Arena *arena){
    TempArena result;
    result.arena = arena;
    result.pos_ptr = arena->alloc_ptr;
    result.arena->temp_count += 1;
    return(result);
}

static void temp_arena_end(TempArena temp_arena){
    temp_arena.arena->alloc_ptr = temp_arena.pos_ptr;
    temp_arena.arena->temp_count -= 1;
}


////////////////////////////////
// NOTE(oleksii): Not realated to Arenas and shouldn't really be here!!!
// It's just for testing. Didn't find a better place to put it.
static FileReadResult read_entire_file_into_memory(Arena *arena, U8 *file_name){
    FileReadResult result = {};
    FILE *file = fopen((char *)file_name, "rb");
    if(file){
        fseek(file, 0L, SEEK_END);
        U64 file_size = ftell(file);
        fseek(file, 0L, SEEK_SET);
        result.size = file_size;
        result.data = ArenaPushArray(arena, U8, file_size); // ArenaPushSize has to be used instead!
        fread(result.data, result.size, 1, file);
    }
    return(result);
}
