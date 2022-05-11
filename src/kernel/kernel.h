#ifndef __KERNEL_H__
#define __KERNEL_H__

void kernel_main(void);
void remap_master_pic_C(void);

extern struct page_directory_handle kernel_page_directory;

#define ERROR(v) (void *)(v)
#define ERROR_INT(v) (int)(v)
#define IS_ERROR(v) ((int)(v < 0))

// PIC 8259 ports
#define PIC1_CMD_PORT                    0x20
#define PIC1_DATA_PORT                   0x21
#define PIC2_CMD_PORT                    0xA0
#define PIC2_DATA_PORT                   0xA1

#define PIC_ICW1_INIT                    0x10
#define PIC_ICW1_ICW4                    0x01
#define PIC_ICW4_8086_MODE               0x01
#define PIC_ICW3_SLAVE_AT_IRQ2           0x04
#define PIC_ICW3_SLAVE_ID                0x02
#endif /* __KERNEL_H__ */
