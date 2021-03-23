#ifndef __KERNEL_H__
#define __KERNEL_H__

void kernel_main(void);
extern struct page_directory_handle kernel_page_directory;

#define ERROR(v) (void *)(v)
#define ERROR_INT(v) (int)(v)
#define IS_ERROR(v) ((int)(v < 0))

#endif /* __KERNEL_H__ */
