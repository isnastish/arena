#ifndef ARENA_H
static Os *os_ptr;

struct Arena{
    void *base_ptr;
    void *alloc_ptr; // NOTE(oleksii): Should be replaced with U64 alloc_pos;
    U64  max_count;
    U64  commited;
    U64  alignment;
    
    // NOTE(oleksii): Number of temporary arenas.
    I32 temp_count;
};

static Arena make_arena(U64 alignment);
static void *arena_push(Arena *arena, U64 size);

struct TempArena{
    Arena *arena;
    void *pos_ptr;
};

static TempArena temp_arena_start(Arena *arena);
static void temp_arena_end(TempArena temp_arena);
static void arena_reset_alloc_ptr(Arena *arena, void *ptr);


////////////////////////////////
// NOTE(oleksii): Just for testing and shouldn't really be here!!!
struct FileReadResult{
    U64 size;
    void *data;
};

static FileReadResult read_entire_file_into_memory(Arena *arena, U8 *file_name);

#define ARENA_H
#endif //ARENA_H
