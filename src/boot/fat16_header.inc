; this two lines are also part of the header

;jmp short init_code_segment
;nop

;times 33 db 0 ; filler, as the BIOS might write its BPB into this section.
; Fat 16 Header

oem_identifier      db 'AvocadOS'
bytes_per_sector    dw 0x200 ; generally ignored
sectors_per_cluster db 0x80
reserved_sectors    dw 200
fat_copies          db 0x2
root_dir_entries    dw 0x40
number_of_sectors   dw 0x0
media_type          db 0xF8
sectors_per_fat     dw 0x100
sectors_per_track   dw 0x20
number_of_heads     dw 0x40
hidden_sectors      dd 0x0
sectors_big         dd 0x773594
;extended BPB
drive_number        db 0x80
win_nt_bit          db 0
signature           db 0x29
volume_id           dd 0xD105
volume_id_string    db 'AvocadosFAT'
system_id_string    db 'FAT16   '
