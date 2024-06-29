#include "buffered_open.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

buffered_file_t *buffered_open(const char *pathname, int flags, ...)
{
    buffered_file_t *bf = malloc(sizeof(buffered_file_t));
    if (!bf)
    {
        perror("malloc error");
        return NULL;
    }
    va_list args;
    va_start(args, flags);
    int mode = 0;
    if (flags & O_CREAT)
    {
        mode = va_arg(args, int);
    }
    va_end(args);

    bf->preappend = (flags & O_PREAPPEND) ? 1 : 0;
    flags &= ~O_PREAPPEND;

    bf->fd = open(pathname, flags, mode);
    if (bf->fd < 0)
    {
        perror("open error");
        free(bf);
        return NULL;
    }

    bf->read_buffer = malloc(BUFFER_SIZE);
    if (!bf->read_buffer)
    {
        perror("malloc error");
        close(bf->fd);
        free(bf);
        return NULL;
    }

    bf->write_buffer = malloc(BUFFER_SIZE);
    if (!bf->write_buffer)
    {
        perror("malloc error");
        close(bf->fd);
        free(bf->read_buffer);
        free(bf);
        return NULL;
    }

    bf->read_buffer_size = 0;
    bf->write_buffer_size = 0;
    bf->read_buffer_pos = 0;
    bf->write_buffer_pos = 0;

    bf->flags = flags;
    return bf;
}
ssize_t buffered_write(buffered_file_t *bf, const void *buf, size_t count)
{
    size_t save_count = count;
    size_t buff_left;
    size_t write_pos = bf->write_buffer_pos;
    if (bf->preappend)
    {
        off_t end = lseek(bf->fd, 0, SEEK_END);
        if ((end < 0) || (lseek(bf->fd, 0, SEEK_SET) < 0))
        {
            perror("lseek error");
            return -1;
        }
        char *file_info = malloc(end + count);
        if (!file_info)
        {
            perror("malloc error");
            return -1;
        }
        if (buffered_read(bf, file_info + count, end) < 0)
        {
            perror("buffered_read error");
            free(file_info);
            return -1;
        }

        size_t to_copy = count;
        size_t copied = 0;
        while (to_copy > 0)
        {
            size_t chunk_size = (to_copy > BUFFER_SIZE) ? BUFFER_SIZE : to_copy;
            memcpy(file_info + copied, buf + copied, chunk_size);
            copied += chunk_size;
            to_copy -= chunk_size;
        }

        lseek(bf->fd, 0, SEEK_SET);
        size_t total_size = end + count;
        size_t write_pos = 0;

        while (total_size > 0)
        {
            size_t chunk_size = (total_size > BUFFER_SIZE) ? BUFFER_SIZE : total_size;
            memcpy(bf->write_buffer, file_info + write_pos, chunk_size);

            bf->write_buffer_pos = chunk_size;
            if (buffered_flush(bf) < 0)
            {
                perror("buffered_flush error");
                free(file_info);
                return -1;
            }

            write_pos += chunk_size;
            total_size -= chunk_size;
        }

        free(file_info);
        return save_count;
    }
    else
    {
        int tot_wrote = 0;
        while (count > 0)
        {
            buff_left = BUFFER_SIZE - bf->write_buffer_pos;
            if (count >= buff_left)
            {
                memcpy(bf->write_buffer + bf->write_buffer_pos, (const char*)buf+tot_wrote, buff_left);
                tot_wrote += buff_left;
                bf->write_buffer_pos = buff_left;
                if (buffered_flush(bf) < 0)
                {
                    perror("buffered_flush error");
                    return -1;
                }
                count -= buff_left;
            }
            else
            {
                memcpy(bf->write_buffer + bf->write_buffer_pos,  (const char*)buf+tot_wrote, count);
                bf->write_buffer_pos += count;
                count = 0;
            }
        }
    }
    return save_count;
}

ssize_t buffered_read(buffered_file_t *bf, void *buf, size_t count)
{
    ssize_t tot_count = 0;
    int info;
    if (bf->write_buffer_pos > 0)
    {
        if (buffered_flush(bf) < 0)
        {
            perror("buffered_flush error");
            return -1;
        }
    }
    while (1)
    {
         if (bf->read_buffer_pos >= bf->read_buffer_size)
        {
            int read_buff_size = read(bf->fd, bf->read_buffer, BUFFER_SIZE);
            if (read_buff_size < 0)
            {
                perror("read error");
                return -1;
            }
            else if (read_buff_size == 0)
            {
                break;
            }
            bf->read_buffer_size = read_buff_size;
            bf->read_buffer_pos = 0;
        }

        info = bf->read_buffer_size - bf->read_buffer_pos;

        if (count > info)
        {
            memcpy(buf, bf->read_buffer + bf->read_buffer_pos, info);
            count -= info;
            tot_count += info;
            bf->read_buffer_pos += info;
            buf = (char *)buf + info;  
        }
        else
        {
            memcpy(buf, bf->read_buffer + bf->read_buffer_pos, count);
            tot_count += count;
            bf->read_buffer_pos += count;
            break;
        }
    }
    return tot_count;
}

int buffered_flush(buffered_file_t *bf)
{
    if (bf->write_buffer_pos > 0)
    {
        if (write(bf->fd, bf->write_buffer, bf->write_buffer_pos) < 0)
        {
            perror("write error");
            return -1;
        }
        bf->write_buffer_pos = 0;
    }
    return 0;
}
int buffered_close(buffered_file_t *bf)
{
    if (buffered_flush(bf) < 0)
    {
        return -1;
    }
    if (close(bf->fd) < 0)
    {
        perror("close error");
    }
    free(bf->read_buffer);
    free(bf->write_buffer);
    free(bf);
    return 0;
}
// int main() {
//     buffered_file_t *bf = buffered_open("example.txt", O_WRONLY | O_CREAT | O_PREAPPEND, 0644);
//     if (!bf) {
//         perror("buffered_open");
//         return 1;
//     }

//     const char *text = "Hello, World!";
//     if (buffered_write(bf, text, strlen(text)) == -1) {
//         perror("buffered_write");
//         buffered_close(bf);
//         return 1;
//     }

//     if (buffered_close(bf) == -1) {
//         perror("buffered_close");
//         return 1;
//     }

//     return 0;
// }


// while(count > 0){
//     size_t buff_left = BUFFER_SIZE- bf->write_buffer_pos;
//     if( count > buff_left){
//         memcpy()
//     }    
// }