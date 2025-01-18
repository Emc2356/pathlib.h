/*
 * Copyright (c) 2025 Stamelos Vasilis
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file pathlib.h
 * @brief a simple library for path manipulation
 *
 * pathlib is a headers only library that aims to simplify interactions
 * with the filesystem while keeping it simple. It is written in C89 so
 * it can be compiled with a lot of different compilers. Pathlib only
 * operates on "normal" paths, aka not extra long paths that are generated
 * by some sort of dynamic allocation. Since it is a headers only library
 * to get the implementations you must define PATHLIB_IMPLEMENTATION.
 *
 * @copyright MIT license
 */

#ifndef _PATHLIB_C_H_
#define _PATHLIB_C_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <stdarg.h>
#include <wctype.h>

#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>
    #include <shlwapi.h>

    #define PATHLIB_MAX_PATH MAX_PATH
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <pwd.h>
    #include <dirent.h>
    #include <fnmatch.h>
    
    #ifdef __linux__
        #include <linux/limits.h>
    #endif

    #define PATHLIB_MAX_PATH PATH_MAX
#endif


#ifndef PATHLIB_MALLOC
/**
 * @brief the function that it will use to allocate memory
 * @note if PATHLIB_MALLOC is defined then #PATHLIB_FREE(ptr) must be defined too
 * @note it is recommended to use a temperory allocator because pathlib deals with a lot of small strings
 */
#define PATHLIB_MALLOC(sz) malloc(sz)
/**
 * @brief the function that it will use to free the pointer that it got from #PATHLIB_MALLOC(sz)
 * @note if #PATHLIB_MALLOC(sz) refers to a temperory allocator then PATHLIB_FREE should be defined as an empty function like marco
*/
#define PATHLIB_FREE(ptr) free(ptr)
#endif /* PATHLIB_MALLOC */

#ifndef PATHLIB_ASSERT
    #define PATHLIB_ASSERT(statement) assert(statement)
#endif /* PATHLIB_ASSERT */

#ifndef PATHLIB_API
/**
 * @brief redefine this macro to add a custom "attributes" to every function, eg static inline
 */
#define PATHLIB_API
#endif /* PATHLIB_API */

#define PATHLIB_NULLABLE
#define PATHLIB_ARRSIZE(arr) (sizeof(arr) / sizeof(*arr))

/**
 * @brief a struct that represents a path in segments
 *
 * @struct Path
 */
typedef struct Path {
    const char** parts;      /**< the parts that the path is made of */
    unsigned short size;     /**< the amount of parts are inside part */
    unsigned short capacity; /**< how many parts have been allocated */
} Path;

/**
 * @brief a dynamic array for paths.
 *
 * @struct Paths
 * @see Path pathlib_paths_add pathlib_paths_pop pathlib_paths_free
 */
typedef struct Paths {
    Path* paths;     /**< the paths that it holds */
    size_t size;     /**< the count of paths that paths refers to */
    size_t capacity; /**< how many paths have been allocated */
} Paths;

/**
 * @brief Represents error codes for file and directory operations.
 *
 * The `Pathlib_Error` enumeration defines specific error codes to indicate 
 * the result of file and directory operations in a cross-platform environment.
 *
 * @enum Pathlib_Error
 */
typedef enum Pathlib_Error {
    /**
     * @brief No error occurred.
     *
     * This value indicates that the operation completed successfully.
     */
    PATHLIB_NONE = 0,
    /**
     * @brief File or directory already exists.
     *
     * This value indicates that a conflict occurred because the file or 
     * directory being created already exists.
     */
    PATHLIB_EXISTS = 1,
    /**
     * @brief File or directory does not exist.
     *
     * This value indicates that the specified file or directory was not found.
     */
    PATHLIB_NEXISTS = 2,
    /**
     * @brief Undefined operating system error.
     *
     * This value indicates that i have not handled all os level errors.
     */
    PATHLIB_OSERROR = 3
} Pathlib_Error;

/**
 * @brief a variable that represents a possible error after a function call.
 *
 * if a function is supposed to set this variable then it will reset it
 * first with PATHLIB_NONE
 *
 * @see Pathlib_Error
 */
extern Pathlib_Error pathlib_error;

/**
 * @brief constructs a Path object from a string
 *
 * @param str the string that it will convert
 * @return a path object that represents the input path
 * @warning str must not be `NULL`
 */
PATHLIB_API Path pathlib_from_str(const char* str);
/**
 * @brief adds a new part to the path object
 *
 * @param path the path that it will add the part to
 * @param part the new part that it will append
 * @warning path and part must not be `NULL`
 */
PATHLIB_API void pathlib_add_part(Path* path, const char* part);
/**
 * @brief copies the input path into a new
 *
 * @param path the path that it will copy
 * @return a copy of the input path
 * @warning path must not be `NULL`
 */
PATHLIB_API Path pathlib_copy(const Path* path);
/**
 * @brief creates a path that represents the current working directory
 *
 * @return a path object that represents the current working directory
 * @note sets pathlib_error to PATHLIB_OSERROR in case of error 
 * @see Pathlib_Error
 */
PATHLIB_API Path pathlib_cwd(void);
/**
 * @brief creates a path that represents the path to the home folder
 *
 * @return a path object that represents the path to the home folder
 * @note sets pathlib_error to PATHLIB_OSERROR in case of error 
 * @see Pathlib_Error
 */
PATHLIB_API Path pathlib_home(void);
/**
 * @brief creates a path that represents the path to the temp folder
 *
 * @return a path object that represents the path to the temp folder
 * @note sets pathlib_error to PATHLIB_OSERROR in case of error 
 * @see Pathlib_Error
 */
PATHLIB_API Path pathlib_temp(void);
/**
 * @brief deallocated a Path struct and zero it out
 *
 * @param path the path that it will clean up 
 * @warning path must not be `NULL`
 */
PATHLIB_API void pathlib_destroy(Path* path);
/**
 * @brief checks whether a path exists
 *
 * @param path the path that it will check 
 * @warning path must not be `NULL`
 */
PATHLIB_API int pathlib_exists(const Path* path);
/**
 * @brief checks whether a path is a directory
 *
 * @param path the path that it will check 
 * @warning path must not be `NULL`
 */
PATHLIB_API int pathlib_is_dir(const Path* path);
/**
 * @brief checks whether a path is a file
 *
 * @param path the path that it will check 
 * @note sets pathlib_error to PATHLIB_OSERROR in case of error 
 * @warning path must not be `NULL`
 */
PATHLIB_API int pathlib_is_file(const Path* path);
/**
 * @brief checks whether a path is a symlink
 *
 * @param path the path that it will check 
 * @note sets pathlib_error to PATHLIB_OSERROR in case of error 
 * @warning path must not be `NULL`
 */
PATHLIB_API int pathlib_is_symlink(const Path* path);
/**
 * @brief checks whether the path points to a mount point
 *
 * @param path the path that it will check 
 * @note sets pathlib_error to PATHLIB_OSERROR in case of error 
 * @warning path must not be `NULL`
 */
PATHLIB_API int pathlib_is_mount(const Path* path);
/**
 * @brief checks whether the path points to a block device
 *
 * @param path the path that it will check 
 * @note sets pathlib_error to PATHLIB_OSERROR in case of error 
 * @warning path must not be `NULL`
 */
PATHLIB_API int pathlib_is_block_device(const Path* path);
/**
 * @brief checks whether the path points to a character device
 *
 * @param path the path that it will check 
 * @note sets pathlib_error to PATHLIB_OSERROR in case of error 
 * @warning path must not be `NULL`
 */
PATHLIB_API int pathlib_is_char_device(const Path* path);
/**
 * @brief checks whether the path points to a socket
 *
 * @param path the path that it will check 
 * @warning path must not be `NULL`
 * @note sets pathlib_error to PATHLIB_OSERROR in case of error 
 * @note always returns false on windows
 */
PATHLIB_API int pathlib_is_socket(const Path* path);
/**
 * @brief checks whether a path points to a named pipe (FIFO)
 *
 * @param path the path that it will check 
 * @note sets pathlib_error to PATHLIB_OSERROR in case of error 
 * @warning path must not be `NULL`
 */
PATHLIB_API int pathlib_is_fifo(const Path* path);
/**
 * @brief checks whether the path is absolute
 *
 * @param path the path that it will check 
 * @warning path must not be `NULL`
 */
PATHLIB_API int pathlib_is_absolute(const Path* path);
/**
 * @brief checks whether a path is relative to another
 *
 * @param path the base path
 * @param other the path that will be compared to param
 * @warning path and other must not be `NULL`
 */
PATHLIB_API int pathlib_is_relative(const Path* path, const Path* other);
/**
 * @brief The last dot-separated portion of the final component, if any
 *
 * @param path the path that it will try to extract the suffix
 * @return the suffix, if any
 * @warning path must not be `NULL`
 */
PATHLIB_API char* pathlib_suffix(const Path* path);
/**
 * @brief A list of the path’s suffixes, if any
 *
 * attempts to extract the suffixes from a given path and return them.
 * the suffix_count parameter is nullable because suffixes[suffix_count] == NULL
 *
 * @param path the path that it will try to extract the suffixes
 * @param suffix_count the amount of suffixes it found
 * @return the suffixes, if any
 * @warning path must not be `NULL`
 */
PATHLIB_API char** pathlib_suffixes(const Path* path, PATHLIB_NULLABLE size_t* suffix_count);
/**
 * @brief it changes the suffix of the path, if there is none then it is added
 *
 * @param path the path that will modify
 * @param suffix the suffix that it will add
 * @warning path and suffix must not be `NULL`
 */
PATHLIB_API void pathlib_with_suffix(Path* path, const char* suffix);
/**
 * @brief The final path component, without its suffix
 *
 * @param path the path that it will try to extract the suffix
 * @return the stem
 * @warning path must not be `NULL`
 */
PATHLIB_API char* pathlib_stem(const Path* path);
/**
 * @brief The logical parent of the path
 *
 * @param path the path
 * @return the logical parent
 * @warning path must not be `NULL`
 */
PATHLIB_API Path pathlib_parent(const Path* path);
/**
 * @brief The logical parents of the path
 *
 * @param path the path
 * @return the logicals parent
 * @warning path must not be `NULL`
 */
PATHLIB_API Paths pathlib_parents(const Path* path);
/**
 * @brief a string representing the final path component
 *
 * @param path the path
 * @return the name
 * @warning path must not be `NULL`
 */
PATHLIB_API const char* pathlib_name(const Path* path);
/**
 * @brief retrieves every file inside the directory 
 *
 * @param path the path
 * @return the files
 * @warning path must not be `NULL`
 */
PATHLIB_API Paths pathlib_listdir(const Path* path);
/**
 * @brief joins a and b and creates a new path 
 *
 * @param pathA the first half of the new path
 * @param pathB the second half of the new path
 * @return the new path
 * @warning pathA and pathB must not be `NULL`
 */
PATHLIB_API Path pathlib_joinpath(const Path* pathA, const Path* pathB);
/**
 * @brief deletes the file or symlink that the path points to
 *
 * @param path the path that it will delete
 * @return 1 on success and 0 on error
 * @note sets pathlib_error to PATHLIB_OSERROR or PATHLIB_NEXISTS in case of error 
 * @warning path must not be `NULL`
 */
PATHLIB_API int pathlib_unlink(const Path* path);
/**
 * @brief deletes the directory that path points too
 *
 * @param path the path that it will delete
 * @param remove_contents whether to delete the contents of the directory
 * @return 1 on success and 0 on error
 * @note sets pathlib_error to PATHLIB_OSERROR or PATHLIB_NEXISTS in case of error 
 * @warning path must not be `NULL`
 */
PATHLIB_API int pathlib_rmdir(const Path* path, int remove_contents);
/**
 * @brief creates the directory that path points too
 *
 * @param path the path that it will create
 * @return 1 on success and 0 on error
 * @note sets pathlib_error to PATHLIB_OSERROR or PATHLIB_EXISTS in case of error 
 * @note it creates the parents too if they dont exist
 * @warning path must not be `NULL`
 */
PATHLIB_API int pathlib_mkdir(const Path* path);
/**
 * @brief creates the file that the path points too
 *
 * @param path the path that it will create
 * @return 1 on success and 0 on error
 * @note sets pathlib_error to PATHLIB_OSERROR or PATHLIB_EXISTS in case of error 
 * @note it creates the parents too if they dont exist
 * @warning path must not be `NULL`
 */
PATHLIB_API int pathlib_touch(const Path* path);
/**
 * @brief it opens the file that the path points to
 *
 * @param path the path that it will attempt to open
 * @param mode the mode that it will pass to fopen
 * @return the file handle or NULL on error
 * @note it creates the file if it doesnt exist
 * @warning path must not be `NULL`
 */
PATHLIB_API FILE* pathlib_open(const Path* path, const char* mode);
/**
 * @brief it reads the contents of the file that path points to
 *
 * @param path the path that it will attempt to open
 * @return the file handle or NULL on error
 * @note it doesnt create the file if it doesnt exist
 * @warning path must not be `NULL`
 */
PATHLIB_API char* pathlib_read_text(const Path* path);
/**
 * @brief it reads the contents of the file that path points to
 *
 * @param path the path that it will attempt to open
 * @param byte_count the amount of bytes it read
 * @return the file handle or NULL on error
 * @note it doesnt create the file if it doesnt exist
 * @warning path must not be `NULL`
 */
PATHLIB_API unsigned char* pathlib_read_bytes(const Path* path, size_t* byte_count);
/**
 * @brief it writes text into the file that path points to
 *
 * @param path the path that it will write text into
 * @param text the text that it will write
 * @param text_size how many characters it will write
 * @return 1 on success and 0 on error
 * @note it creates the file if it doesnt exist
 * @warning path must not be `NULL`
 */
PATHLIB_API int pathlib_write_text(const Path* path, const char* text, size_t text_size);
/**
 * @brief it writes bytes into the file that path points to
 *
 * @param path the path that it will write bytes into
 * @param buff the buffer that it will write
 * @param buff_size how many bytes it will write
 * @return 1 on success and 0 on error
 * @note it creates the file if it doesnt exist
 * @warning path must not be `NULL`
 */
PATHLIB_API int pathlib_write_bytes(const Path* path, const unsigned char* buff, size_t buff_size);
/**
 * @brief it creates a string that represents the path
 *
 * @param path the path 
 * @return the string representation of the path
 * @warning path must not be `NULL`
 */
PATHLIB_API char* pathlib_to_str(const Path* path);
/**
 * @brief it creates a string that represents the path and puts it into buffer
 *
 * @param path the path 
 * @param buffer the buffer that it will write the string into
 * @param buffer_size the capacity of the buffer
 * @return 1 on success and 0 on false
 * @warning path and buffer must not be `NULL`
 */
PATHLIB_API int pathlib_render_str_to_buffer(const Path* path, char* buffer, size_t buffer_size);
/**
 * @brief it creates a hash for the input path
 *
 * @param path the path 
 * @return the final hash
 * @warning path must not be `NULL`
 */
PATHLIB_API unsigned long pathlib_hashfunc(const Path* path);
/**
 * @brief return names of files that match the pattern
 *
 * @param path the path that it will search
 * @param pattern the pattern that will try to match
 * @return the files that match the pattern inside the path directory
 * @note pathlib_glob is not recursive and does not support `**\/`, for recursive globbing check @ref pathlib_rglob
 * @note sets pathlib_error to PATHLIB_NEXISTS or PATHLIB_OSERROR
 * @note they results are not sorted
 * @warning path and pattern must not be `NULL`
 */
PATHLIB_API Paths pathlib_glob(const Path* path, const char* pattern);
/**
 * @brief return names of files that match the pattern recursivly
 *
 * @param path the path that it will search
 * @param pattern the pattern that will try to match
 * @return the files that match the pattern inside the path directory
 * @warning path and pattern must not be `NULL`
 * @note sets pathlib_error to PATHLIB_NEXISTS or PATHLIB_OSERROR
 * @note they results are not sorted
 */
PATHLIB_API Paths pathlib_rglob(const Path* path, const char* pattern);
/**
 * @brief it adds a new path to a Paths struct
 *
 * @param paths the struct that it will add the path
 * @param path the path that it will add to paths
 * @warning path must not be `NULL`
 */
PATHLIB_API void pathlib_paths_add(Paths* paths, const Path path);
/**
 * @brief it removes the i-th element from paths
 *
 * @param paths the struct that it will remove the element from
 * @param i the index of the element that it will delete
 * @warning paths must not be `NULL` and paths->size > i
 */
PATHLIB_API void pathlib_paths_pop(Paths* paths, size_t i);
/**
 * @brief it deallocates the contents of paths->paths and zero it out
 *
 * @param paths the struct that it will clean up
 * @warning paths must not be `NULL`
 */
PATHLIB_API void pathlib_paths_free(Paths* paths);

#endif /* _PATHLIB_C_H_ */

#ifdef PATHLIB_IMPLEMENTATION

Pathlib_Error pathlib_error = PATHLIB_NONE;

void pathlib_print_error(const char *fmt, ...) {
    va_list args;

    fprintf(stderr, "[ERROR] ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

#ifdef _WIN32
    #define pathlib_print_os_error(failed_function_name, pathname) pathlib_print_error(failed_function_name" failed for `%s`: %zu", pathname, GetLastError())
    #define pathlib_print_func_failed(failed_function_name) pathlib_print_error(failed_function_name" failed: %zu", GetLastError())
#else /* _WIN32 */
    #define pathlib_print_os_error(failed_function_name, pathname) pathlib_print_error(failed_function_name" failed for `%s`: %s", pathname, strerror(errno))
    #define pathlib_print_func_failed(failed_function_name) pathlib_print_error(failed_function_name" failed: %s", strerror(errno))
#endif /* _WIN32 */

void* pathlib___malloc(size_t size, const char* file, size_t line) {
    void* region = PATHLIB_MALLOC(size);
    if (region) {
        return region;
    }
    
    fprintf(stderr, "[FATAL] out of memory at %s:%lu", file, (unsigned long int)line);
    abort();
}
#define pathlib__malloc(size) pathlib___malloc(size, __FILE__, __LINE__)

PATHLIB_API Path pathlib_cwd(void) {
    char* cwd_buff;
    size_t size;

    pathlib_error = PATHLIB_NONE;
    
    #ifdef _WIN32
        size = GetCurrentDirectory(0, NULL);
        if (size == 0) {
            pathlib_print_func_failed("GetCurrentDirectory");
            pathlib_error = PATHLIB_OSERROR;
            return pathlib_from_str(".");
        }

        cwd_buff = (char*)pathlib__malloc(size * sizeof(char));

        if (GetCurrentDirectory(size, cwd_buff) == 0) {
            pathlib_print_func_failed("GetCurrentDirectory");
            pathlib_error = PATHLIB_OSERROR;
            return pathlib_from_str(".");
        }
    #else
        size = 8;
        cwd_buff = (char*)pathlib__malloc(size * sizeof(char));

        while (getcwd(cwd_buff, size) == NULL) {
            if (errno == ERANGE) {
                size *= 2;
                PATHLIB_FREE(cwd_buff);
                cwd_buff = (char*)pathlib__malloc(size * sizeof(char));
            } else {
                pathlib_print_func_failed("getcwd");
                pathlib_error = PATHLIB_OSERROR;
                return pathlib_from_str(".");
            }
        }
    #endif

    return pathlib_from_str(cwd_buff);
}

PATHLIB_API Path pathlib_home(void) {
    char* home_dir = NULL;

    #ifdef _WIN32
        char* home_drive, *home_path;
        size_t size;
        
        pathlib_error = PATHLIB_NONE;
        
        home_dir = getenv("USERPROFILE");
        if (home_dir && strlen(home_dir) > 0) {
            return pathlib_from_str(home_dir);
        }

        home_drive = getenv("HOMEDRIVE");
        home_path = getenv("HOMEPATH");
        if (home_drive && home_path) {
            size = strlen(home_drive) + strlen(home_path) + 1;
            home_dir = (char*)pathlib__malloc(size * sizeof(char));
            sprintf(home_dir, "%s%s", home_drive, home_path);
            return pathlib_from_str(home_dir);
        }

    #else
        struct passwd* pw;

        pathlib_error = PATHLIB_NONE;
        
        home_dir = getenv("HOME");
        if (home_dir && strlen(home_dir) > 0) {
            return pathlib_from_str(home_dir);
        }

        pw = getpwuid(getuid());
        if (pw && pw->pw_dir) {
            return pathlib_from_str(pw->pw_dir);
        }
    #endif

    pathlib_print_error("failed to determine the home directory");
    pathlib_error = PATHLIB_OSERROR;
    return pathlib_from_str(".");
}

PATHLIB_API Path pathlib_temp(void) {
    char* temp_dir = NULL;
    
    #ifdef _WIN32
        DWORD size = GetTempPath(0, NULL);
        
        pathlib_error = PATHLIB_NONE;
        
        if (size == 0) {
            pathlib_print_func_failed("GetTempPath");
            pathlib_error = PATHLIB_OSERROR;
            return pathlib_from_str("./tmp");
        }

        temp_dir = (char*)pathlib__malloc(size * sizeof(char));

        if (GetTempPath(size, temp_dir) == 0) {
            pathlib_print_func_failed("GetTempPath");
            pathlib_error = PATHLIB_OSERROR;
            return pathlib_from_str("./tmp");
        }
        return pathlib_from_str(temp_dir);
    #else
        size_t i;
        const char* env_vars[] = {"TMPDIR", "TEMP", "TMP"};
        
        pathlib_error = PATHLIB_NONE;
        
        for (i = 0; i < sizeof(env_vars) / sizeof(env_vars[0]); i++) {
            temp_dir = getenv(env_vars[i]);
            if (temp_dir && strlen(temp_dir) > 0) {
                return pathlib_from_str(temp_dir);
            }
        }

        return pathlib_from_str("/tmp");
    #endif
}

PATHLIB_API Path pathlib_from_str(const char* instr) {
    char* start, *temp;
    size_t instr_len, part_count;
    char* str;
    Path path;

    PATHLIB_ASSERT(instr);

    instr_len = strlen(instr);
    str = memcpy(pathlib__malloc(instr_len + 1), instr, instr_len);
    str[instr_len] = 0;
    
    part_count = 1;
    for (temp = str; *temp; temp++) {
        if (*temp == '\\' || *temp == '/') {
            part_count++;
        }
    }
    
    path.parts = pathlib__malloc(sizeof(*path.parts) * part_count);
    path.size = 0;
    path.capacity = part_count;

    start = str;
    for (temp = str; ; temp++) {
        if (*temp == '/' || *temp == '\\') {
            size_t len = temp - start;
            start[len] = 0;
            path.parts[path.size++] = start;
            start = temp + 1;
            continue;
        } else if (*temp == '\0') {
            size_t len = temp - start;
            start[len] = 0;
            path.parts[path.size++] = start;
            break;
        }
    }

    return path;
}

PATHLIB_API void pathlib_add_part(Path* path, const char* part) {
    void* temp;

    PATHLIB_ASSERT(path);
    PATHLIB_ASSERT(part);

    if (path->size >= path->capacity) {
        path->capacity = path->capacity == 0 ? 4 : (int)(path->capacity*1.5 + 1);
        temp = pathlib__malloc(path->capacity * sizeof(*path->parts));
        if (path->size > 0) {
            memcpy(temp, path->parts, sizeof(*path->parts)*path->size);
        }
        PATHLIB_FREE(path->parts);
        path->parts = temp;
    }

    path->parts[path->size++] = part;
}

PATHLIB_API void pathlib_destroy(Path* path) {
    if (path) {
        if (path->parts) {
            PATHLIB_FREE(path->parts);
            path->parts = 0;
            path->size = 0;
            path->capacity = 0;
        }
    }
}

PATHLIB_API Path pathlib_parent(const Path* path) {
    Path ret;

    PATHLIB_ASSERT(path);

    if (path->size == 1) {
        ret.parts = pathlib__malloc(sizeof(*path->parts) * 2);
        ret.capacity = 2;
        ret.size = 1;
        ret.parts[0] = ".";
    } else if (path->size > 0) {
        ret.parts = pathlib__malloc(path->capacity * sizeof(*path->parts));
        ret.capacity = path->capacity;
        ret.size = path->size - 1;
        memcpy(ret.parts, path->parts, (ret.size) * sizeof(*path->parts));
    } else {
        ret.parts = NULL;
        ret.capacity = 0;
        ret.size = 0;
    }

    return ret;
}

PATHLIB_API Paths pathlib_parents(const Path* path) {
    Paths paths;
    const Path* path_ref;
    
    size_t i;
    
    PATHLIB_ASSERT(path);
    
    paths.paths = NULL;
    paths.size = 0;
    paths.capacity = 0;
        
    if (path->size == 0) {
        pathlib_paths_add(&paths, pathlib_from_str("."));
        return paths;
    }
    
    if (path->size == 1) {
        pathlib_paths_add(&paths, pathlib_from_str("."));
        return paths;
    }
    
    path_ref = path;
    for (i = 0; i < path->size; i++) {
        pathlib_paths_add(&paths, pathlib_parent(path_ref));
        path_ref = &paths.paths[i];
    }
    
    return paths;
}

PATHLIB_API char* pathlib_suffix(const Path* path) {
    size_t last_part_size, suffix_size;
    const char* last_part;
    char* suffix;

    PATHLIB_ASSERT(path);

    if (path->size == 0) {
        return "";
    }
    last_part = path->parts[path->size-1];
    last_part_size = strlen(last_part);
    suffix_size = 0;

    while (last_part_size > suffix_size) {
        if (last_part[last_part_size - suffix_size] == '.') break;
        suffix_size++;
    }

    if (*(last_part + (last_part_size - suffix_size)) != '.') {
        return "";
    }
    suffix = pathlib__malloc(suffix_size + 1);
    memcpy(suffix, last_part + (last_part_size - suffix_size), suffix_size);
    suffix[suffix_size] = 0;

    return suffix;
}

PATHLIB_API char** pathlib_suffixes(const Path* path, PATHLIB_NULLABLE size_t* suffix_count) {
    size_t count, i, last_part_size, suffix_size;
    char** suffixes;
    char* suffix;
    const char* last_part;
    
    (void) i;
    (void) last_part_size;
    
    PATHLIB_ASSERT(path);
    
    if (path->size == 0) {
        suffixes = pathlib__malloc(sizeof(*suffixes));
        suffixes[0] = NULL;
        if (suffix_count != NULL) {
            suffix_count = 0;
        }
        return suffixes;
    }
    
    last_part = path->parts[path->size - 1];
    last_part_size = 0;
    count = 0;
    
    while (*last_part) {
        if (*last_part == '.') {
            count += 1;
        }
        
        last_part_size++;
        last_part++;
    }
    
    if (count == 0) {
        suffixes = pathlib__malloc(sizeof(*suffixes));
        suffixes[0] = NULL;
        if (suffix_count != NULL) {
            suffix_count = 0;
        }
        return suffixes;
    }
    
    suffixes = pathlib__malloc(sizeof(*suffixes) * (count + 1));
    suffixes[count] = NULL;
    if (suffix_count) {
        *suffix_count = count;
    }
    
    suffix_size = 0;
    i = 0;
    while (count) {
        if (*last_part == '.') {
            count--;
            suffix = pathlib__malloc(suffix_size + 1);
            suffix[suffix_size] = 0;
            memcpy(suffix, last_part, suffix_size);
            suffixes[i] = suffix;
            i++;
            suffix_size = 1;
        } else {
            suffix_size++;
        }
        
        last_part--;
    }
    
    return suffixes;
}

PATHLIB_API void pathlib_with_suffix(Path* path, const char* new_suffix) {
    size_t last_part_size, suffix_size;
    const char* last_part;
    char* suffix;
    char* new_last_part;

    PATHLIB_ASSERT(path);

    if (path->size == 0) {
        pathlib_add_part(path, new_suffix);
        return;
    }
    last_part = path->parts[path->size-1];
    last_part_size = strlen(last_part);
    suffix_size = 0;

    while (last_part_size > suffix_size) {
        if (last_part[last_part_size - suffix_size] == '.') break;
        suffix_size++;
    }

    if (*(last_part + (last_part_size - suffix_size)) != '.') {
        new_last_part = pathlib__malloc(last_part_size + strlen(new_suffix) + 1);
        memcpy(new_last_part, last_part, last_part_size);
        memcpy(new_last_part + last_part_size, new_suffix, strlen(new_suffix));
        
        new_last_part[last_part_size + strlen(suffix)] = 0;
        
        path->parts[path->size-1] = new_last_part;
        return;
    }
    
    new_last_part = pathlib__malloc(last_part_size - suffix_size + strlen(new_suffix));
    
    memcpy(new_last_part, last_part, last_part_size - suffix_size);
    memcpy(new_last_part + last_part_size - suffix_size, new_suffix, strlen(new_suffix));
    
    new_last_part[last_part_size - suffix_size + strlen(new_suffix)] = 0;
    
    path->parts[path->size-1] = new_last_part;
    return;
}

PATHLIB_API char* pathlib_stem(const Path* path) {
    size_t last_part_size, suffix_size;
    const char* last_part;
    char* region;

    PATHLIB_ASSERT(path);

    if (path->size == 0) {
        return "";
    }
    last_part = path->parts[path->size-1];
    last_part_size = strlen(last_part);
    suffix_size = 0;

    while (last_part_size > suffix_size) {
        if (last_part[last_part_size - suffix_size] == '.') break;
        suffix_size++;
    }

    if (*(last_part + (last_part_size - suffix_size)) != '.') {
        return "";
    }
    region = pathlib__malloc(last_part_size - suffix_size + 1);
    memcpy(region, last_part, last_part_size - suffix_size);
    region[last_part_size - suffix_size] = 0;

    return region;
}

PATHLIB_API char* pathlib_to_str(const Path* path) {
    size_t total_size, i, p;
    char* region;

    PATHLIB_ASSERT(path);

    total_size = 0;
    for (i = 0; i < path->size; i++) {
        total_size += strlen(path->parts[i]);
    }

    /* add n-1 extra bytes for the seperators */
    if (path->size > 0) {
        total_size += path->size - 1;
    }
    /* +1 for null terminator */
    region = pathlib__malloc(total_size + 1);

    if (path->size == 0) {
        region[0] = 0;
        return region;
    }

    p = 0;
    for (i = 0; i < path->size; i++) {
        memcpy(region+p, path->parts[i], strlen(path->parts[i]));
        p += strlen(path->parts[i]);
        region[p] = '/';
        p += 1;
    }
    region[total_size] = 0;

    return region;
}

int pathlib_render_str_to_buffer(const Path* path, char* buffer, size_t buffer_size) {
    size_t i, str_size;
    
    PATHLIB_ASSERT(path);
    PATHLIB_ASSERT(buffer);
    
    if (path->size == 0) {
        if (buffer_size < 1) {
            return 0;
        }
        buffer[0] = 0;
        return 1;
    }
    
    i = 0;
    while (buffer_size > 0) {
        str_size = strlen(path->parts[i]);
        if (str_size > buffer_size) {
            return 0;
        }
        memcpy(buffer, path->parts[i], str_size);
        buffer += str_size;
        buffer_size -= str_size;
        
        i++;
        
        if (i != path->size) {
            if (buffer_size < 1) {
                return 0;
            }
            buffer[0] = '/';
            buffer++;
            buffer_size--;
        } else {
            if (buffer_size < 1) {
                return 0;
            }
            buffer[0] = 0;
            return 1;
        }
    }
    
    return 0;
}

/* internal function so no safety */
PATHLIB_API int pathlib__render_n_parts_to_buffer(const Path* path, char* buffer, size_t buffer_size, size_t n_parts) {
    size_t i, str_size;
        
    PATHLIB_ASSERT(path);
    PATHLIB_ASSERT(buffer);
    
    if (n_parts) {
        if (buffer_size < 1) {
            return 0;
        }
        buffer[0] = 0;
        return 1;
    }
    
    i = 0;
    while (buffer_size > 0) {
        str_size = strlen(path->parts[i]);
        if (str_size > buffer_size) {
            return 0;
        }
        memcpy(buffer, path->parts[i], str_size);
        buffer += str_size;
        buffer_size -= str_size;
        
        i++;
        
        if (i != n_parts) {
            if (buffer_size < 1) {
                return 0;
            }
            buffer[0] = '/';
            buffer++;
            buffer_size--;
        } else {
            if (buffer_size < 1) {
                return 0;
            }
            buffer[0] = 0;
            return 1;
        }
    }
    
    return 0;
}

PATHLIB_API int pathlib_exists(const Path* path) {
    char filename[PATHLIB_MAX_PATH];
    /* if it cant be rendered in a buffer that holds the system limit for a path then it doesnt exist */
    if (!pathlib_render_str_to_buffer(path, filename, PATHLIB_ARRSIZE(filename))) {
        return 0;
    }
    #ifdef _WIN32
        return GetFileAttributesA(filename) != INVALID_FILE_ATTRIBUTES;
    #else /* _WIN32 */
        return access(filename, F_OK) == 0;
    #endif /* _WIN32 */
}

PATHLIB_API int pathlib_is_file(const Path* path) {
    char filename[PATHLIB_MAX_PATH];
    #ifdef _WIN32
        DWORD attr;
    #else
        struct stat statbuf;
    #endif

    pathlib_error = PATHLIB_NONE;

    if (!pathlib_exists(path)) {
        return 0;
    }

    /* if it cant be rendered in a buffer that holds the system limit for a path then it doesnt exist */
    if (!pathlib_render_str_to_buffer(path, filename, PATHLIB_ARRSIZE(filename))) {
        return 0;
    }
    #ifdef _WIN32
        attr = GetFileAttributesA(filename);
        if (attr == INVALID_FILE_ATTRIBUTES) {
            pathlib_print_os_error("GetFileAttributesA", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }
        return (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
    #else /* _WIN32 */
        if (stat(filename, &statbuf) < 0) {
            pathlib_print_os_error("stat", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }

        return (statbuf.st_mode & S_IFMT) == S_IFREG;
    #endif /* _WIN32 */
}

PATHLIB_API int pathlib_is_dir(const Path* path) {
    char filename[PATH_MAX];
    #ifdef _WIN32
        DWORD attr;
    #else
        struct stat statbuf;
    #endif

    pathlib_error = PATHLIB_NONE;

    if (!pathlib_exists(path)) {
        return 0;
    }

    /* if it cant be rendered in a buffer that holds the system limit for a path then it doesnt exist */
    if (!pathlib_render_str_to_buffer(path, filename, PATHLIB_ARRSIZE(filename))) {
        return 0;
    }
    #ifdef _WIN32
        attr = GetFileAttributesA(filename);
        if (attr == INVALID_FILE_ATTRIBUTES) {
            pathlib_print_os_error("GetFileAttributesA", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }
        return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
    #else /* _WIN32 */
        if (stat(filename, &statbuf) < 0) {
            pathlib_print_os_error("stat", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }

        return (statbuf.st_mode & S_IFMT) == S_IFDIR;
    #endif /* _WIN32 */
}

PATHLIB_API int pathlib_is_symlink(const Path* path) {
    #ifdef _WIN32
        DWORD attributes;
    #else
        struct stat pathStat;
    #endif
    char filename[PATHLIB_MAX_PATH];

    PATHLIB_ASSERT(path);

    pathlib_error = PATHLIB_NONE;
    
    if (!pathlib_exists(path)) {
        return 0;
    }

    if (!pathlib_render_str_to_buffer(path, filename, PATHLIB_ARRSIZE(filename))) {
        return 0;
    }

    #ifdef _WIN32
        attributes = GetFileAttributesA(filename);
        if (attributes == INVALID_FILE_ATTRIBUTES) {
            pathlib_print_os_error("GetFileAttributesA", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }
        return (attributes & FILE_ATTRIBUTE_REPARSE_POINT) ? 1 : 0;

    #else
        if (lstat(filename, &pathStat) != 0) {
            pathlib_print_os_error("stat", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }
        return S_ISLNK(pathStat.st_mode) ? 1 : 0;
    #endif
}

PATHLIB_API int pathlib_is_mount(const Path* path) {
    char filename[PATHLIB_MAX_PATH];
    #ifdef _WIN32
        char volumePath[PATHLIB_MAX_PATH];
    #else
        struct stat pathStat, parentStat;
        char parentPath[PATHLIB_MAX_PATH];
    #endif

    PATHLIB_ASSERT(path);

    pathlib_error = PATHLIB_NONE;

    if (!pathlib_exists(path)) {
        return 0;
    }

    if (!pathlib_render_str_to_buffer(path, filename, PATHLIB_ARRSIZE(filename))) {
        return 0;
    }

    #ifdef _WIN32
        if (!GetVolumePathName(filename, volumePath, PATHLIB_ARRSIZE(volumePath))) {
            pathlib_print_os_error("GetVolumePathName", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }
        return strcmp(filename, volumePath) == 0 ? 1 : 0;

    #else
        if (stat(filename, &pathStat) != 0) {
            pathlib_print_os_error("stat", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }

        snprintf(parentPath, sizeof(parentPath), "%s/..", filename);

        if (stat(parentPath, &parentStat) != 0) {
            pathlib_print_os_error("stat", parentPath);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }

        return (pathStat.st_dev != parentStat.st_dev) ? 1 : 0;
    #endif
}

PATHLIB_API int pathlib_is_block_device(const Path* path) {
    char filename[PATHLIB_MAX_PATH];
    #ifdef _WIN32
        HANDLE hFile;
        DWORD bytesReturned;
        STORAGE_PROPERTY_QUERY query;
        BYTE buffer[512];
        BOOL result;
        STORAGE_DEVICE_DESCRIPTOR* descriptor;
    #else
        struct stat pathStat;
    #endif

    PATHLIB_ASSERT(path);

    pathlib_error = PATHLIB_NONE;

    if (!pathlib_exists(path)) {
        return 0;
    }

    if (!pathlib_render_str_to_buffer(path, filename, PATHLIB_ARRSIZE(filename))) {
        return 0;
    }

    #ifdef _WIN32
        hFile = CreateFile(
            filename,                           /* File path */
            0,                                  /* No access to the file */
            FILE_SHARE_READ | FILE_SHARE_WRITE, /* Allow shared access */
            NULL,                               /* Default security attributes */
            OPEN_EXISTING,                      /* Open the existing file */
            FILE_ATTRIBUTE_NORMAL,              /* Normal attributes */
            NULL                                /* No template file */
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            pathlib_print_os_error("CreateFile", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }

        query.PropertyId = StorageDeviceProperty;
        query.QueryType = PropertyStandardQuery;

        result = DeviceIoControl(
            hFile,
            IOCTL_STORAGE_QUERY_PROPERTY,
            &query,
            sizeof(query),
            buffer,
            sizeof(buffer),
            &bytesReturned,
            NULL
        );

        CloseHandle(hFile);

        if (!result) {
            pathlib_print_os_error("DeviceIoControl", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }

        descriptor = (STORAGE_DEVICE_DESCRIPTOR*)buffer;
        return (descriptor->BusType == BusTypeScsi || descriptor->BusType == BusTypeAta) ? 1 : 0;
    #else
        if (stat(filename, &pathStat) != 0) {
            pathlib_print_os_error("stat", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }

        return S_ISBLK(pathStat.st_mode) ? 1 : 0;
    #endif
}

PATHLIB_API int pathlib_is_char_device(const Path* path) {
    char filename[PATHLIB_MAX_PATH];
    #ifdef _WIN32
        HANDLE hFile;
        const char* knownDevices[] = {"CON", "PRN", "AUX", "NUL", "COM1", "COM2", "LPT1", NULL};
        const char** device;
    #else
        struct stat pathStat;
    #endif

    PATHLIB_ASSERT(path);

    pathlib_error = PATHLIB_NONE;
    
    if (!pathlib_exists(path)) {
        return 0;
    }

    if (!pathlib_render_str_to_buffer(path, filename, sizeof(filename) / sizeof(*filename))) {
        return 0;
    }

    #ifdef _WIN32
        hFile = CreateFile(
            filename,
            0,                                  /* No access to the file */
            FILE_SHARE_READ | FILE_SHARE_WRITE, /* Allow shared access */
            NULL,                               /* Default security attributes */
            OPEN_EXISTING,                      /* Open the existing file */
            FILE_ATTRIBUTE_NORMAL,              /* Normal attributes */
            NULL                                /* No template file */
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            for (device = knownDevices; *device; ++device) {
                if (_stricmp(filename, *device) == 0) {
                    return 1;
                }
            }

            pathlib_print_os_error("CreateFile", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }

        CloseHandle(hFile);
        return 1;
    #else
        if (stat(filename, &pathStat) != 0) {
            pathlib_print_os_error("stat", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }

        return S_ISCHR(pathStat.st_mode) ? 1 : 0;
    #endif
}

PATHLIB_API int pathlib_is_socket(const Path* path) {
    #ifdef _WIN32
        (void) path;
        return 0;
    #else
        char filename[PATHLIB_MAX_PATH];
        struct stat pathStat;

        PATHLIB_ASSERT(path);
        
        pathlib_error = PATHLIB_NONE;

        if (!pathlib_exists(path)) {
            return 0;
        }

        if (!pathlib_render_str_to_buffer(path, filename, sizeof(filename) / sizeof(*filename))) {
            return 0;
        }

        if (stat(filename, &pathStat) != 0) {
            pathlib_print_os_error("stat", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }

        return S_ISSOCK(pathStat.st_mode) ? 1 : 0;
    #endif
}

PATHLIB_API int pathlib_is_fifo(const Path* path) {
    char filename[PATHLIB_MAX_PATH];
    #ifndef _WIN32
        struct stat pathStat;
    #endif

    PATHLIB_ASSERT(path);
    
    pathlib_error = PATHLIB_NONE;

    if (!pathlib_exists(path)) {
        return 0;
    }

    if (!pathlib_render_str_to_buffer(path, filename, sizeof(filename) / sizeof(*filename))) {
        return 0;
    }

    #ifdef _WIN32
        /* TODO: not certain if this will ever be a string that is possible to be generated */
        return strncmp(filename, "\\\\.\\pipe\\", 9) == 0;
    #else
        if (stat(filename, &pathStat) != 0) {
            pathlib_print_os_error("stat", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }

        return S_ISFIFO(pathStat.st_mode) ? 1 : 0;
    #endif
}

PATHLIB_API int pathlib_is_absolute(const Path* path) {
    #ifdef _WIN32
        if (path->size == 0) {
            return 0;
        }
        if (strlen(path->parts[0]) != 2) {
            return 0;
        }
        /* Check for drive letter followed by ':' and '\' (implicit), e.g., "C:\" */
        if (isalpha(path->parts[0][0]) && path->parts[0][1] == ':') {
            return 1;
        }
        /* Check for UNC paths starting with "\\" */
        if (path->parts[0][0] == '\\' && path->parts[0][1] == '\\') {
            return 1;
        }
    #else
        size_t i;
        for (i = 0; i < path->size; i++) {
            if (path->parts[i][0] == '/') {
                return 1;
            }
        }
    #endif
    return 0;
}

PATHLIB_API int pathlib_is_relative(const Path* path, const Path* other) {
    size_t i;

    if (path->size > other->size) {
        return 0;
    }

    for (i = 0; i < path->size; i++) {
        if (strcmp(path->parts[i], other->parts[i]) != 0) {
            return 0;
        }
    }

    return 1;
}

PATHLIB_API int pathlib_mkdir(const Path* path) {
    char filename[PATHLIB_MAX_PATH];
    size_t i;
    
    pathlib_error = PATHLIB_NONE;
    
    if (path->size == 0) return 1;
    if (pathlib_exists(path) && pathlib_is_dir(path)) return 1;

    for (i = 1; i <= path->size; i++) {
        if (!pathlib__render_n_parts_to_buffer(path, filename, sizeof(filename) / sizeof(*filename), i)) {
            return 0;
        }
        #ifdef _WIN32
            if (CreateDirectory(filename, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
                if (i == path->size) {
                    pathlib_error = PATHLIB_EXISTS;
                }
                errno = EEXIST;
            } else {
                pathlib_print_os_error("CreateDirectory", filename);
                pathlib_error = PATHLIB_OSERROR;
                return 0;
            }
        #else /* _WIN32 */
            if (mkdir(filename, 0755) < 0) {
                if (errno == EEXIST) {
                    if (i == path->size) {
                        pathlib_error = PATHLIB_EXISTS;
                    }
                }
                pathlib_print_os_error("mkdir", filename);
                pathlib_error = PATHLIB_OSERROR;
                return 0;
            }
        #endif /* _WIN32 */
    }

    return 1;
}

PATHLIB_API int pathlib_touch(const Path* path) {
    #ifdef _WIN32
        HANDLE hFile;
    #else
        int fd;
    #endif
    char filename[PATHLIB_MAX_PATH];
    Path subpath;
    
    pathlib_error = PATHLIB_NONE;
    
    if (path->size == 0) return 1;
    if (path->size > 1) {
        subpath.parts = path->parts;
        subpath.size = path->size - 1;
        subpath.capacity = path->capacity;
        if (!pathlib_mkdir(&subpath)) {
            return 0;
        }
    }

    if (pathlib_exists(path) && pathlib_is_file(path)) {
        pathlib_error = PATHLIB_EXISTS;
        return 1;
    }
    
    if (!pathlib_render_str_to_buffer(path, filename, PATHLIB_ARRSIZE(filename))) {
        return 0;
    }

    #ifdef _WIN32
        hFile = CreateFile(
            filename,              /* File name */
            GENERIC_WRITE,         /* Desired access */
            0,                     /* Share mode (no sharing) */
            NULL,                  /* Security attributes */
            CREATE_NEW,            /* Creation disposition (create a new file) */
            FILE_ATTRIBUTE_NORMAL, /* File attributes */
            NULL                   /* Template file */
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            pathlib_print_os_error("CreateFile", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }
        CloseHandle(hFile);
        return 1;
    #else
        fd = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

        if (fd == -1) {
            pathlib_print_os_error("open", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }

        close(fd);
        return 1;
    #endif
}

PATHLIB_API Path pathlib_joinpath(const Path* path, const Path* other) {
    size_t i;
    Path ret;

    ret.parts = NULL;
    ret.size = 0;
    ret.capacity = 0;

    for (i = 0; i < path->size; i++) {
        pathlib_add_part(&ret, path->parts[i]);
    }

    for (i = 0; i < other->size; i++) {
        pathlib_add_part(&ret, other->parts[i]);
    }

    return ret;
}

PATHLIB_API const char* pathlib_name(const Path* path) {
    PATHLIB_ASSERT(path);

    if (path->size == 0) {
        return "";
    }

    return path->parts[path->size - 1];
}

PATHLIB_API Path pathlib_copy(const Path* path) {
    Path ret;

    ret.parts = pathlib__malloc(path->capacity * sizeof(*path->parts));

    memcpy(ret.parts, path->parts, path->size * sizeof(*path->parts));
    ret.size = path->size;
    ret.capacity = path->capacity;

    return ret;
}

PATHLIB_API FILE* pathlib_open(const Path* path, const char* mode) {
    char filename[PATHLIB_MAX_PATH];

    PATHLIB_ASSERT(path);
    PATHLIB_ASSERT(path);

    if (!pathlib_render_str_to_buffer(path, filename, sizeof(filename) / sizeof(*filename))) {
        return NULL;
    }

    if (!pathlib_exists(path)) {
        if (!pathlib_touch(path)) {
            return NULL;
        }
    }

    return fopen(filename, mode);
}

PATHLIB_API char* pathlib_read_text(const Path* path) {
    FILE* f;
    long int file_size;
    char* buff;
    
    if (!pathlib_exists(path)) {
        return NULL;
    }
    
    f = pathlib_open(path, "r");
    
    if (f == NULL) {
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) < 0) {
        return NULL;
    }
    file_size = ftell(f);
    if (file_size < 0) {
        return NULL;
    }
    if (fseek(f, 0, SEEK_SET) < 0) {
        return NULL;
    }
    
    buff = pathlib__malloc(file_size + 1);
    buff[file_size] = 0;
    fread(buff, file_size, 1, f);

    if (ferror(f)) {
        fclose(f);
        PATHLIB_FREE(buff);
        return NULL;
    }
    fclose(f);
    
    return buff;
}

PATHLIB_API unsigned char* pathlib_read_bytes(const Path* path, size_t* byte_count) {
    FILE* f;
    long int file_size;
    unsigned char* buff;
    
    if (!pathlib_exists(path)) {
        return NULL;
    }
    
    f = pathlib_open(path, "rb");
    
    if (f == NULL) {
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) < 0) {
        return NULL;
    }
    file_size = ftell(f);
    if (file_size < 0) {
        return NULL;
    }
    if (fseek(f, 0, SEEK_SET) < 0) {
        return NULL;
    }
    
    buff = pathlib__malloc(file_size + 1);
    buff[file_size] = 0;
    fread(buff, file_size, 1, f);

    if (ferror(f)) {
        fclose(f);
        PATHLIB_FREE(buff);
        return NULL;
    }
    fclose(f);
    
    *byte_count = file_size;
    return buff;
}
 
PATHLIB_API int pathlib_write_text(const Path* path, const char* text, size_t text_size) {
    FILE* f;
    size_t n;
    
    if (!pathlib_exists(path)) {
        if (!pathlib_touch(path)) {
            return 0;
        }
    }
    
    f = pathlib_open(path, "w");
    
    if (f == NULL) {
        return 0;
    }
    
    while (text_size > 0) {
        n = fwrite(text, 1, text_size, f);
        if (ferror(f)) {
            fclose(f);
            return 0;
        }
        text_size -= n;
        text  += n;
    }
    
    fclose(f);
    
    return 1;
}

PATHLIB_API int pathlib_write_bytes(const Path* path, const unsigned char* buff, size_t buff_size) {
    FILE* f;
    size_t n;
    
    if (!pathlib_exists(path)) {
        if (!pathlib_touch(path)) {
            return 0;
        }
    }
    
    f = pathlib_open(path, "wb");
    
    if (f == NULL) {
        return 0;
    }
    
    while (buff_size > 0) {
        n = fwrite(buff, 1, buff_size, f);
        if (ferror(f)) {
            fclose(f);
            return 0;
        }
        buff_size -= n;
        buff  += n;
    }
    
    fclose(f);
    
    return 1;
}

PATHLIB_API int pathlib_unlink(const Path* path) {
    char filename[PATHLIB_MAX_PATH];
   
    PATHLIB_ASSERT(path);
    
    pathlib_error = PATHLIB_NONE;
    
    if (!pathlib_exists(path)) {
        pathlib_error = PATHLIB_NEXISTS;
        return 0;
    }

    if (!pathlib_render_str_to_buffer(path, filename, sizeof(filename) / sizeof(*filename))) {
        /* this part should be unreachable */
        pathlib_error = PATHLIB_NEXISTS;
        return 0;
    }
    
    if (remove(filename) != 0) {
        pathlib_error = PATHLIB_OSERROR;
        return 0;
    }

    return 1;
}

#ifdef _WIN32
int pathlib__remove_directory_contents(const Path* path) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    char fullPath[MAX_PATH];
    char searchPath[MAX_PATH+3];
    size_t searchPathSize;
    Path subpath;
    
    pathlib_error = PATHLIB_NONE;
    
    if (!pathlib_render_str_to_buffer(path, searchPath, sizeof(fullPath) / sizeof(*fullPath))) {
        return 0;
    }
    searchPathSize = strlen(searchPath);
    searchPath[searchPathSize] = '\\';
    searchPath[searchPathSize+1] = '*';
    searchPath[searchPathSize+2] = '\0';

    hFind = FindFirstFile(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        pathlib_print_os_error("FindFirstFile", searchPath);
        pathlib_error = PATHLIB_OSERROR;
        return 0;
    }

    do {
        const char* name = findFileData.cFileName;

        /* Skip "." and ".." entries */
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            continue;
        }
        
        subpath = pathlib_copy(path);
        pathlib_add_part(&subpath, name);

        /* Recursive removal for directories, or delete files */
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!pathlib_rmdir(&subpath, 1)) {
                FindClose(hFind);
                return 0;
            }
        } else {
            if (!pathlib_render_str_to_buffer(&subpath, fullPath, sizeof(fullPath) / sizeof(*fullPath))) {
                return 0;
            }
            if (!DeleteFile(fullPath)) {
                FindClose(hFind);
                pathlib_print_os_error("DeleteFile", fullPath);
                pathlib_error = PATHLIB_OSERROR;
                return 0;
            }
        }

        pathlib_destroy(&subpath);

    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return 1;
}
#else /* _WIN32 */
int pathlib__remove_directory_contents(const Path* path) {
    DIR* dir;
    struct dirent* entry;
    struct stat pathStat;
    char fullPath[PATHLIB_MAX_PATH];
    Path subpath;
    
    pathlib_error = PATHLIB_NONE;
    
    if (!pathlib_render_str_to_buffer(path, fullPath, sizeof(fullPath) / sizeof(*fullPath))) {
        return 0;
    }

    dir = opendir(fullPath);
    if (dir == NULL) {
        pathlib_print_os_error("opendir", fullPath);
        pathlib_error = PATHLIB_OSERROR;
        return 0;
    }

    while ((entry = readdir(dir)) != NULL) {
        const char* name = entry->d_name;

        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            continue;
        }

        subpath = pathlib_copy(path);
        pathlib_add_part(&subpath, name);

        if (!pathlib_render_str_to_buffer(&subpath, fullPath, sizeof(fullPath) / sizeof(*fullPath))) {
            pathlib_destroy(&subpath);
            return 0;
        }
        
        if (stat(fullPath, &pathStat) != 0) {
            pathlib_print_os_error("stat", fullPath);
            pathlib_error = PATHLIB_OSERROR;
            closedir(dir);
            pathlib_destroy(&subpath);
            return 0;
        }

        if (S_ISDIR(pathStat.st_mode)) {
            if (!pathlib_rmdir(&subpath, 1)) {
                closedir(dir);
                pathlib_destroy(&subpath);
                return 0;
            }
        } else {
            if (unlink(fullPath) != 0) {
                closedir(dir);
                pathlib_destroy(&subpath);
                return 0;
            }
        }

        pathlib_destroy(&subpath);
    }

    closedir(dir);
    return 1;
}
#endif /* _WIN32 */

PATHLIB_API int pathlib_rmdir(const Path* path, int remove_contents) {
    char filename[PATHLIB_MAX_PATH];
    
    PATHLIB_ASSERT(path);
    
    pathlib_error = PATHLIB_NONE;
    
    if (!pathlib_exists(path)) {
        pathlib_error = PATHLIB_NEXISTS;
        return 0;
    }
    
    if (!pathlib_render_str_to_buffer(path, filename, sizeof(filename) / sizeof(*filename))) {
        pathlib_error = PATHLIB_NEXISTS;
        return 0;
    }
    
    
    if (remove_contents) {
        if (!pathlib__remove_directory_contents(path)) {
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }
    }
    
    #ifdef _WIN32
        if (!RemoveDirectory(filename)) {
            pathlib_print_os_error("RemoveDirectory", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }
    #else
        if (rmdir(filename) != 0) {
            pathlib_print_os_error("rmdir", filename);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }
    #endif
    
    return 1;
}

/* Djb2 hash function by Dan Bernstein */
PATHLIB_API unsigned long pathlib_hashfunc(const Path* path) {
    unsigned long hash = 5381;
    const char* str;
    int c;
    size_t i;
    
    for (i = 0; i < path->size; i++) {
        str = path->parts[i];
        while ((c = *str++)) {
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        }
        hash = ((hash << 5) + hash) + '/';
    }

    return hash;
}

PATHLIB_API void pathlib_paths_add(Paths* paths, const Path path) {
    void* temp;
    
    PATHLIB_ASSERT(paths);

    if (paths->size >= paths->capacity) {
        paths->capacity = paths->capacity == 0 ? 4 : paths->capacity*2;
        temp = pathlib__malloc(paths->capacity * sizeof(*paths->paths));
        if (paths->size > 0) {
            memcpy(temp, paths->paths, sizeof(*paths->paths)*paths->size);
        }
        PATHLIB_FREE(paths->paths);
        paths->paths = temp;
    }

    paths->paths[paths->size++] = path;
}

PATHLIB_API void pathlib_paths_pop(Paths* paths, size_t i) {
    PATHLIB_ASSERT(paths->size > i);
    
    if (paths->size == 0) {
        return;
    }
    if (paths->size - 1 == i) {
        paths->size--;
        return;
    }

    memmove(paths->paths + i, paths->paths + i + 1, sizeof(*paths->paths) * (paths->size - i));
    paths->size--;
}

PATHLIB_API void pathlib_paths_free(Paths* paths) {
    if (paths->paths) {
        PATHLIB_FREE(paths->paths);
        paths->capacity = 0;
        paths->size = 0;
        paths->paths = NULL;
    }
}

PATHLIB_API Paths pathlib_listdir(const Path* inpath) {
    Paths paths;
    char fullPath[PATHLIB_MAX_PATH];
    Path path;
    Path subpath;
    size_t name_len, count;
    const char* name;
    #ifdef _WIN32
        char searchPath[PATHLIB_MAX_PATH+3];
        WIN32_FIND_DATA findFileData;
        HANDLE hFind;
        size_t searchPathSize;
    #else /* _WIN32 */
        DIR* dir;
        struct dirent* entry;
    #endif /* _WIN32 */
        
    paths.paths = NULL;
    paths.size = 0;
    paths.capacity = 0;
    subpath.parts = NULL;
    subpath.size = 0;
    subpath.capacity = 0;
    
    pathlib_error = PATHLIB_NONE;
    
    if (!pathlib_exists(inpath)) {
        return paths;
    }
    
    if (!pathlib_render_str_to_buffer(inpath, fullPath, sizeof(fullPath) / sizeof(*fullPath))) {
        return paths;
    }
    
    /* doing this because this function appends one extra item to the end of the input path
    *  and this way it doesnt need to waste memory by doubling the capacity of the path */
    path.parts = memcpy(pathlib__malloc(sizeof(*path.parts) * (inpath->capacity + 1)), inpath->parts, sizeof(*path.parts) * (inpath->capacity + 1));
    path.size = inpath->size;
    path.capacity = inpath->capacity + 1;
    
    /* TODO */
    #ifdef _WIN32
        memcpy(searchPath, fullPath, PATHLIB_MAX_PATH);

        searchPathSize = strlen(searchPath);
        searchPath[searchPathSize] = '\\';
        searchPath[searchPathSize+1] = '*';
        searchPath[searchPathSize+2] = '\0';

        count = 0;
        hFind = FindFirstFile(searchPath, &findFileData);
        if (hFind == INVALID_HANDLE_VALUE) {
            pathlib_print_os_error("FindFirstFile", searchPath);
            pathlib_error = PATHLIB_OSERROR;
            return paths;
        }
        
        do {
            name = findFileData.cFileName;
    
            /* Skip "." and ".." entries */
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
                continue;
            }
            
            count++;
    
        } while (FindNextFile(hFind, &findFileData) != 0);
    
        FindClose(hFind);
        
        paths.paths = pathlib__malloc(sizeof(*paths.paths)*count);
        paths.capacity = count;
        
        hFind = FindFirstFile(searchPath, &findFileData);
        if (hFind == INVALID_HANDLE_VALUE) {
            pathlib_print_os_error("FindFirstFile", searchPath);
            pathlib_error = PATHLIB_OSERROR;
            return paths;
        }
    
        do {
            name = findFileData.cFileName;
    
            /* Skip "." and ".." entries */
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
                continue;
            }
            
            name_len = strlen(name);
            name = memcpy(pathlib__malloc(name_len + 1), name, name_len);
            
            subpath = pathlib_copy(&path);
            pathlib_add_part(&subpath, name);
            pathlib_paths_add(&paths, subpath);
    
        } while (FindNextFile(hFind, &findFileData) != 0);
    
        FindClose(hFind);
        
    #else /* _WIN32 */
        count = 0;
        dir = opendir(fullPath);
        if (dir == NULL) {
            pathlib_print_os_error("opendir", fullPath);
            pathlib_error = PATHLIB_OSERROR;
            return paths;
        }
    
        while ((entry = readdir(dir)) != NULL) {
            name = entry->d_name;
    
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
                continue;
            }
            count++;
        }
    
        closedir(dir);
        
        paths.paths = pathlib__malloc(sizeof(*paths.paths)*count);
        paths.capacity = count;
        
        dir = opendir(fullPath);
        if (dir == NULL) {
            pathlib_print_os_error("opendir", fullPath);
            pathlib_error = PATHLIB_OSERROR;
            return paths;
        }
    
        while ((entry = readdir(dir)) != NULL) {
            name = entry->d_name;
    
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
                continue;
            }
            
            name_len = strlen(name);
            name = memcpy(pathlib__malloc(name_len + 1), name, name_len);
            
            subpath = pathlib_copy(&path);
            pathlib_add_part(&subpath, name);
            pathlib_paths_add(&paths, subpath);
        }
    
        closedir(dir);
    #endif /* _WIN32 */
    
    pathlib_destroy(&path);
        
    return paths;
}

/* a slightly stripped down version of fnmatch from musl C stdlib */
/* BEGIN OF https://github.com/kraj/musl/blob/eb4309b142bb7a8bdc839ef1faf18811b9ff31c8/src/regex/fnmatch.c */
#define END 0
#define UNMATCHABLE -2
#define BRACKET -3
#define QUESTION -4
#define STAR -5
#define	FNM_NOMATCH 1
#define FNM_NOSYS   (-1)

static wctype_t pathlib__wctype(const char *s) {
   	int i;
   	const char *p;
   	/* order must match! */
   	static const char names[] =
  		"alnum\0" "alpha\0" "blank\0"
  		"cntrl\0" "digit\0" "graph\0"
  		"lower\0" "print\0" "punct\0"
  		"space\0" "upper\0" "xdigit";
   	for (i=1, p=names; *p; i++, p+=6)
  		if (*s == *p && !strcmp(s, p))
 			return i;
   	return 0;
}

static int pathlib__str_next(const char *str, size_t n, size_t *step) {
   	if (!n) {
  		*step = 0;
  		return 0;
   	}
   	*step = 1;
   	return str[0];
}

static int pathlib__pat_next(const char *pat, size_t m, size_t *step) {
   	if (!m || !*pat) {
  		*step = 0;
  		return END;
   	}
   	*step = 1;
   	if (pat[0]=='\\' && pat[1]) {
  		*step = 2;
  		pat++;
  		goto escaped;
   	}
   	if (pat[0]=='[') {
  		size_t k = 1;
  		if (k<m) if (pat[k] == '^' || pat[k] == '!') k++;
  		if (k<m) if (pat[k] == ']') k++;
  		for (; k<m && pat[k] && pat[k]!=']'; k++) {
 			if (k+1<m && pat[k+1] && pat[k]=='[' && (pat[k+1]==':' || pat[k+1]=='.' || pat[k+1]=='=')) {
				int z = pat[k+1];
				k+=2;
				if (k<m && pat[k]) k++;
				while (k<m && pat[k] && (pat[k-1]!=z || pat[k]!=']')) k++;
				if (k==m || !pat[k]) break;
 			}
  		}
  		if (k==m || !pat[k]) {
 			*step = 1;
 			return '[';
  		}
  		*step = k+1;
  		return BRACKET;
   	}
   	if (pat[0] == '*')
  		return STAR;
   	if (pat[0] == '?')
  		return QUESTION;
escaped:
   	return pat[0];
}

static int pathlib__match_bracket(const char *p, int k, int kfold) {
   	wchar_t wc;
   	int inv = 0;
   	p++;
   	if (*p=='^' || *p=='!') {
  		inv = 1;
  		p++;
   	}
   	if (*p==']') {
  		if (k==']') return !inv;
  		p++;
   	} else if (*p=='-') {
  		if (k=='-') return !inv;
  		p++;
   	}
   	wc = p[-1];
   	for (; *p != ']'; p++) {
  		if (p[0]=='-' && p[1]!=']') {
 			wchar_t wc2;
 			int l = mbtowc(&wc2, p+1, 4);
 			if (l < 0) return 0;
 			if (wc <= wc2)
				if ((unsigned)k-wc <= (unsigned)wc2-wc ||
				(unsigned)kfold-wc <= (unsigned)wc2-wc)
   					return !inv;
 			p += l-1;
 			continue;
  		}
  		if (p[0]=='[' && (p[1]==':' || p[1]=='.' || p[1]=='=')) {
 			const char *p0 = p+2;
 			int z = p[1];
 			p+=3;
 			while (p[-1]!=z || p[0]!=']') p++;
 			if (z == ':' && p-1-p0 < 16) {
				char buf[16];
				memcpy(buf, p0, p-1-p0);
				buf[p-1-p0] = 0;
				if (iswctype(k, pathlib__wctype(buf)) ||
				    iswctype(kfold, pathlib__wctype(buf)))
   					return !inv;
 			}
 			continue;
  		}
  		if ((unsigned char)*p < 128U) {
 			wc = (unsigned char)*p;
  		} else {
 			int l = mbtowc(&wc, p, 4);
 			if (l < 0) return 0;
 			p += l-1;
  		}
  		if (wc==k || wc==kfold) return !inv;
   	}
   	return inv;
}

static int pathlib__fnmatch_internal(const char *pat, size_t m, const char *str, size_t n) {
   	const char *p, *ptail, *endpat;
   	const char *s, *stail, *endstr;
   	size_t pinc, sinc, tailcnt=0;
   	int c, k;

   	for (;;) {
  		switch ((c = pathlib__pat_next(pat, m, &pinc))) {
  		case UNMATCHABLE:
 			return FNM_NOMATCH;
  		case STAR:
 			pat++;
 			m--;
 			break;
  		default:
 			k = pathlib__str_next(str, n, &sinc);
 			if (k <= 0)
				return (c==END) ? 0 : FNM_NOMATCH;
 			str += sinc;
 			n -= sinc;
 			if (c == BRACKET) {
				if (!pathlib__match_bracket(pat, k, k))
   					return FNM_NOMATCH;
 			} else if (c != QUESTION && k != c && k != c) {
				return FNM_NOMATCH;
 			}
 			pat+=pinc;
 			m-=pinc;
 			continue;
  		}
  		break;
   	}

   	/* Compute real pat length if it was initially unknown/-1 */
   	m = strlen(pat);
   	endpat = pat + m;

   	/* Find the last * in pat and count chars needed after it */
   	for (p=ptail=pat; p<endpat; p+=pinc) {
  		switch (pathlib__pat_next(p, endpat-p, &pinc)) {
  		case UNMATCHABLE:
 			return FNM_NOMATCH;
  		case STAR:
 			tailcnt=0;
 			ptail = p+1;
 			break;
  		default:
 			tailcnt++;
 			break;
  		}
   	}

   	/* Past this point we need not check for UNMATCHABLE in pat,
  	 * because all of pat has already been parsed once. */

   	/* Compute real str length if it was initially unknown/-1 */
   	n = strlen(str);
   	endstr = str + n;
   	if (n < tailcnt) return FNM_NOMATCH;

   	/* Find the final tailcnt chars of str, accounting for UTF-8.
  	 * On illegal sequences we may get it wrong, but in that case
  	 * we necessarily have a matching failure anyway. */
   	for (s=endstr; s>str && tailcnt; tailcnt--) {
  		if ((unsigned char)s[-1] < 128U || MB_CUR_MAX==1) s--;
  		else while ((unsigned char)*--s-0x80U<0x40 && s>str);
   	}
   	if (tailcnt) return FNM_NOMATCH;
   	stail = s;

   	/* Check that the pat and str tails match */
   	p = ptail;
   	for (;;) {
  		c = pathlib__pat_next(p, endpat-p, &pinc);
  		p += pinc;
  		if ((k = pathlib__str_next(s, endstr-s, &sinc)) <= 0) {
 			if (c != END) return FNM_NOMATCH;
 			break;
  		}
  		s += sinc;
  		if (c == BRACKET) {
 			if (!pathlib__match_bracket(p-pinc, k, k))
				return FNM_NOMATCH;
  		} else if (c != QUESTION && k != c && k != c) {
 			return FNM_NOMATCH;
  		}
   	}

   	/* We're all done with the tails now, so throw them out */
   	endstr = stail;
   	endpat = ptail;

   	/* Match pattern components until there are none left */
   	while (pat<endpat) {
  		p = pat;
  		s = str;
  		for (;;) {
 			c = pathlib__pat_next(p, endpat-p, &pinc);
 			p += pinc;
 			/* Encountering * completes/commits a component */
 			if (c == STAR) {
				pat = p;
				str = s;
				break;
 			}
 			k = pathlib__str_next(s, endstr-s, &sinc);
 			if (!k)
				return FNM_NOMATCH;
 			if (c == BRACKET) {
				if (!pathlib__match_bracket(p-pinc, k, k))
   					break;
 			} else if (c != QUESTION && k != c && k != c) {
				break;
 			}
 			s += sinc;
  		}
  		if (c == STAR) continue;
  		/* If we failed, advance str, by 1 char if it's a valid
 		 * char, or past all invalid bytes otherwise. */
  		k = pathlib__str_next(str, endstr-str, &sinc);
  		if (k > 0) str += sinc;
  		else for (str++; pathlib__str_next(str, endstr-str, &sinc)<0; str++);
   	}

   	return 0;
}

int pathlib__fnmatch(const char *pat, const char *str) {
   	return pathlib__fnmatch_internal(pat, -1, str, -1);
}

#undef END
#undef UNMATCHABLE
#undef BRACKET
#undef QUESTION
#undef STAR
#undef FNM_NOMATCH
#undef FNM_NOSYS

/* END OF https://github.com/kraj/musl/blob/eb4309b142bb7a8bdc839ef1faf18811b9ff31c8/src/regex/fnmatch.c */

static int pathlib__recursive_glob(char* base_path, const char* pattern, int recursive, Paths* results) {
    char fullpath[PATHLIB_MAX_PATH];

    #if defined(_WIN32)
        WIN32_FIND_DATA find_data;
        HANDLE hFind;
        char search_path[PATHLIB_MAX_PATH];
    
        if (snprintf(search_path, sizeof(search_path), "%s\\*", base_path) < 0) {
            return 0;
        }
    
        hFind = FindFirstFile(search_path, &find_data);
        if (hFind == INVALID_HANDLE_VALUE) {
            pathlib_print_os_error("FindFirstFile", search_path);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }
    
        do {
            const char* name = find_data.cFileName;
            
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
                continue;
            }
    
            if (snprintf(fullpath, sizeof(fullpath), "%s\\%s", base_path, name) < 0) {
                continue;
            }

            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (recursive) {
                    if (!pathlib__recursive_glob(fullpath, pattern, recursive, results)) {
                        FindClose(hFind);
                        return 0;
                    }
                }
                continue;
            }
    
            if (pathlib__fnmatch(pattern, name) == 0) {
                pathlib_paths_add(results, pathlib_from_str(fullpath));
            }
    
        } while (FindNextFile(hFind, &find_data) != 0);
    
        FindClose(hFind);
    #else /* _WIN32 */
        DIR* dir;
        struct dirent* entry;
        struct stat path_stat;
    
        dir = opendir(base_path);
        if (dir == NULL) {
            pathlib_print_os_error("opendir", base_path);
            pathlib_error = PATHLIB_OSERROR;
            return 0;
        }
    
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
    
            if (snprintf(fullpath, sizeof(fullpath), "%s/%s", base_path, entry->d_name) < 0) {
                continue;
            }
    
            if (stat(fullpath, &path_stat) != 0) {
                continue;
            }
    
            if (S_ISDIR(path_stat.st_mode)) {
                if (recursive) {
                    if (!pathlib__recursive_glob(fullpath, pattern, recursive, results)) {
                        closedir(dir);
                        return 0;
                    }
                }
                continue;
            }
    
            if (pathlib__fnmatch(pattern, entry->d_name) == 0) {
                pathlib_paths_add(results, pathlib_from_str(fullpath));
            }
        }
    
        closedir(dir);
    #endif /*_WIN32 */

    return 1;
}

PATHLIB_API Paths pathlib_glob(const Path* path, const char* pattern) {
    char fullpath[PATHLIB_MAX_PATH];
    Paths results;
    results.paths = NULL;
    results.size = 0;
    results.capacity = 0;
    
    PATHLIB_ASSERT(path);
    PATHLIB_ASSERT(pattern);
    
    pathlib_error = PATHLIB_NONE;
    
    if (!pathlib_is_dir(path)) {
        pathlib_error = PATHLIB_NEXISTS;
        return results;
    }
    
    if (!pathlib_render_str_to_buffer(path, fullpath, PATHLIB_ARRSIZE(fullpath))) {
        pathlib_error = PATHLIB_NEXISTS;
        return results;
    }
    
    if (!pathlib__recursive_glob(fullpath, pattern, 0, &results)) {
        pathlib_paths_free(&results);
    }

    return results;
}

PATHLIB_API Paths pathlib_rglob(const Path* path, const char* pattern) {
    char fullpath[PATHLIB_MAX_PATH];
    Paths results;
    results.paths = NULL;
    results.size = 0;
    results.capacity = 0;
    
    PATHLIB_ASSERT(path);
    PATHLIB_ASSERT(pattern);
    
    pathlib_error = PATHLIB_NONE;
    
    if (!pathlib_is_dir(path)) {
        pathlib_error = PATHLIB_NEXISTS;
        return results;
    }
    
    if (!pathlib_render_str_to_buffer(path, fullpath, PATHLIB_ARRSIZE(fullpath))) {
        pathlib_error = PATHLIB_NEXISTS;
        return results;
    }
    
    if (!pathlib__recursive_glob(fullpath, pattern, 1, &results)) {
        pathlib_paths_free(&results);
    }

    return results;
}

#endif /* PATHLIB_IMPLEMENTATION */
