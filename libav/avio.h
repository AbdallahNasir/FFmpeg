#ifndef AVIO_H
#define AVIO_H

/* output byte stream handling */

typedef INT64 offset_t;

/* unbuffered I/O */

struct URLContext {
    struct URLProtocol *prot;
    int flags;        
    int is_streamed;  /* true if streamed (no seek possible), default = false */
    int max_packet_size;  /* if non zero, the stream is packetized with this max packet size */
    void *priv_data;
};

typedef struct URLContext URLContext;

typedef struct URLPollEntry {
    URLContext *handle;
    int events;
    int revents;
} URLPollEntry;

#define URL_RDONLY 0
#define URL_WRONLY 1
#define URL_RDWR   2

int url_open(URLContext **h, const char *filename, int flags);
int url_read(URLContext *h, unsigned char *buf, int size);
int url_write(URLContext *h, unsigned char *buf, int size);
offset_t url_seek(URLContext *h, offset_t pos, int whence);
int url_close(URLContext *h);
int url_exist(const char *filename);
offset_t url_filesize(URLContext *h);
int url_get_max_packet_size(URLContext *h);
/* not implemented */
int url_poll(URLPollEntry *poll_table, int n, int timeout);

typedef struct URLProtocol {
    const char *name;
    int (*url_open)(URLContext *h, const char *filename, int flags);
    int (*url_read)(URLContext *h, unsigned char *buf, int size);
    int (*url_write)(URLContext *h, unsigned char *buf, int size);
    offset_t (*url_seek)(URLContext *h, offset_t pos, int whence);
    int (*url_close)(URLContext *h);
    struct URLProtocol *next;
} URLProtocol;

extern URLProtocol *first_protocol;

int register_protocol(URLProtocol *protocol);

typedef struct {
    unsigned char *buffer;
    int buffer_size;
    unsigned char *buf_ptr, *buf_end;
    void *opaque;
    int (*read_packet)(void *opaque, UINT8 *buf, int buf_size);
    void (*write_packet)(void *opaque, UINT8 *buf, int buf_size);
    int (*seek)(void *opaque, offset_t offset, int whence);
    offset_t pos; /* position in the file of the current buffer */
    int must_flush; /* true if the next seek should flush */
    int eof_reached; /* true if eof reached */
    int write_flag;  /* true if open for writing */
    int is_streamed;
    int max_packet_size;
} ByteIOContext;

int init_put_byte(ByteIOContext *s,
                  unsigned char *buffer,
                  int buffer_size,
                  int write_flag,
                  void *opaque,
                  int (*read_packet)(void *opaque, UINT8 *buf, int buf_size),
                  void (*write_packet)(void *opaque, UINT8 *buf, int buf_size),
                  int (*seek)(void *opaque, offset_t offset, int whence));

void put_byte(ByteIOContext *s, int b);
void put_buffer(ByteIOContext *s, const unsigned char *buf, int size);
void put_le64(ByteIOContext *s, UINT64 val);
void put_be64(ByteIOContext *s, UINT64 val);
void put_le32(ByteIOContext *s, unsigned int val);
void put_be32(ByteIOContext *s, unsigned int val);
void put_le16(ByteIOContext *s, unsigned int val);
void put_be16(ByteIOContext *s, unsigned int val);
void put_tag(ByteIOContext *s, char *tag);

void put_be64_double(ByteIOContext *s, double val);
void put_strz(ByteIOContext *s, const char *buf);

offset_t url_fseek(ByteIOContext *s, offset_t offset, int whence);
void url_fskip(ByteIOContext *s, offset_t offset);
offset_t url_ftell(ByteIOContext *s);
int url_feof(ByteIOContext *s);

#define URL_EOF (-1)
int url_fgetc(ByteIOContext *s);
int url_fprintf(ByteIOContext *s, const char *fmt, ...);
char *url_fgets(ByteIOContext *s, char *buf, int buf_size);

void put_flush_packet(ByteIOContext *s);

int get_buffer(ByteIOContext *s, unsigned char *buf, int size);
int get_byte(ByteIOContext *s);
unsigned int get_le32(ByteIOContext *s);
UINT64 get_le64(ByteIOContext *s);
unsigned int get_le16(ByteIOContext *s);

double get_be64_double(ByteIOContext *s);
char *get_strz(ByteIOContext *s, char *buf, int maxlen);
unsigned int get_be16(ByteIOContext *s);
unsigned int get_be32(ByteIOContext *s);
UINT64 get_be64(ByteIOContext *s);

static inline int url_is_streamed(ByteIOContext *s)
{
    return s->is_streamed;
}

int url_fdopen(ByteIOContext *s, URLContext *h);
int url_setbufsize(ByteIOContext *s, int buf_size);
int url_fopen(ByteIOContext *s, const char *filename, int flags);
int url_fclose(ByteIOContext *s);
URLContext *url_fileno(ByteIOContext *s);
int url_fget_max_packet_size(ByteIOContext *s);

int url_open_buf(ByteIOContext *s, UINT8 *buf, int buf_size, int flags);
int url_close_buf(ByteIOContext *s);

int url_open_dyn_buf(ByteIOContext *s);
int url_open_dyn_packet_buf(ByteIOContext *s, int max_packet_size);
int url_close_dyn_buf(ByteIOContext *s, UINT8 **pbuffer);

/* file.c */
extern URLProtocol file_protocol;
extern URLProtocol pipe_protocol;

/* udp.c */
extern URLProtocol udp_protocol;
int udp_set_remote_url(URLContext *h, const char *uri);
int udp_get_local_port(URLContext *h);
int udp_get_file_handle(URLContext *h);

/* tcp.c  */
extern URLProtocol tcp_protocol;

/* http.c */
extern URLProtocol http_protocol;

#endif

