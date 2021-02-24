#ifdef DESASTRE
int fat16_read_root_dir(struct disk_t *disk, struct fat_header_t *header, struct fat_directory_t *root_out)
{
    root_out->first_sector = header->reserved_sectors + (header->fat_copies * header->sectors_per_fat);
    const uint32_t root_dir_size = header->root_dir_entries * sizeof(struct fat_entry_t);
    root_out->entries = kzalloc(root_dir_size);

    if (!root_out->entries) {
        return -ENOMEM;
    }

    int res = 0;

    struct disk_stream *stream = diskstream_new(disk->id);

    if (!stream) {
        res = -ENOMEM;
        goto out;
    }

    diskstream_seek(stream, root_out->first_sector * header->bytes_per_sector);
    int bytes = diskstream_read(stream, root_out->entries, root_dir_size);

out:
    if (res && root_out->entries) {
        kfree(root_out->entries);
    }

    if (stream) {
        diskstream_close(stream);
    }

    if (res) {
        return res;
    }

    return bytes > 0 ? 0 : bytes;
}

void fat16_advance_cursors(struct fat_descriptor_t *descriptor, uint32_t amount)
{
    const struct fat_private_data_t *private = descriptor->disk->fs_private;
    const struct fat_header_t *header = &private->header;

    uint32_t current_cluster_count = descriptor->pos / private->bytes_per_cluster;
    uint32_t current_cluster_number = fat16_sector_to_cluster_number(private, descriptor->current_cluster_first_sector);

    descriptor->pos += amount;

    if (fat16_item_is_root_dir(private, &descriptor->item) && descriptor->pos == private->data_sector) {
        descriptor->eof = 1;
    } else {
        descriptor->offset_in_cluster = descriptor->pos % private->bytes_per_cluster;
        uint32_t next_cluster_count = descriptor->pos / private->bytes_per_cluster;
        uint32_t next_cluster_number = fat16_cluster_chain_nth(private, current_cluster_number, next_cluster_count - current_cluster_count);

        if (next_cluster_number != FAT16_CLUSTER_CHAIN_END) {
            descriptor->current_cluster_first_sector = fat16_cluster_number_to_data_sector(private, next_cluster_number);
        } else {
            descriptor->eof = 1;
        }
    }

    uint32_t data_region_addr = descriptor->current_cluster_first_sector * header->bytes_per_sector;
    uint32_t offset = !fat16_item_is_root_dir(private, &descriptor->item) ? descriptor->offset_in_cluster : descriptor->pos;

    descriptor->address = data_region_addr + offset;
}

struct fat_item_t *fat16_find_entry_in_dir2(struct disk_t *disk, struct fat_item_t *dir, const char *filename, const char *extension)
{
    if (dir->type != FAT16_DIRECTORY_FLAG && dir->type != FAT16_ROOT_DIRECTORY_FLAG) {
        return NULL;
    }

    size_t filename_len = strnlen(filename, 8);
    size_t extension_len = strnlen(extension, 3);

    char padded_filename[8];
    char padded_extension[3];

    memset(&padded_filename, ' ', sizeof(padded_filename)); //NOLINT
    memset(&padded_extension, ' ', sizeof(padded_extension)); //NOLINT

    memcpy(&padded_filename, filename, filename_len); //NOLINT
    memcpy(&padded_extension, extension, extension_len); //NOLINT

    const struct fat_private_data_t *private = disk->fs_private;
    const struct fat_header_t *header = &private->header;

    struct fat_entry_t entry;
    uint32_t sector = dir->first_sector;

    uint32_t read = 0;
    uint32_t dir_position = 0;

    do {
        uint32_t address = sector * header->bytes_per_sector + read;
        diskstream_seek(private->dev, address);
        read += diskstream_read(private->dev, &entry, sizeof(struct fat_entry_t));

        if (!fat16_item_is_root_dir(private, dir) && read == private->bytes_per_cluster) {
            uint16_t current_cluster = fat16_sector_to_cluster_number(private, sector);
            uint16_t next_cluster = fat16_fat_next_cluster_number(private, current_cluster);

            if (next_cluster == FAT16_CLUSTER_CHAIN_END) {
                return NULL;
            }

            sector = fat16_cluster_number_to_data_sector(private, next_cluster);
            read = 0;
        }

        dir_position++;

        if (entry.attributes == FAT16_FILE_LFN || entry.attributes & FAT16_FILE_VOLUME_LABEL) {
            continue;
        }

        if (!strncmp(padded_filename, entry.filename, 8) || !strncmp(padded_extension, entry.extension, 3)) {
            continue;
        }

        struct fat_item_t *item = kzalloc(sizeof(struct fat_item_t));

        if (!item) {
            return NULL;
        }

        item->parent_dir_first_sector = dir->first_sector;
        item->parent_dir_position = dir_position;
        item->entry = entry;
        item->first_sector = fat16_cluster_number_to_data_sector(private, item->entry.low16_bits_first_cluster);

        return item;

    } while (entry.filename[0] != 0);

    return NULL;
}

int read_sector(struct disk_t *disk, uint32_t sector, unsigned int *buffer)
{
    const struct fat_private_data_t *private = disk->fs_private;
    const struct fat_header_t *header = &private->header;
    const uint32_t address = sector * header->bytes_per_sector;
    diskstream_seek(private->dev, address);
    int to_read = header->bytes_per_sector;
    int read = 0;
    while (read < to_read) {
        int chunk = diskstream_read(private->dev, buffer + read, to_read - read);
        if (chunk <= 0) {
            return chunk;
        }
        read += chunk;
    }
    return read;
}

int fat16_testing(struct disk_t *disk)
{

    struct fat_private_data_t *fat_private = (struct fat_private_data_t *)disk->fs_private;

    struct fat_descriptor_t *root_dir = fat16_open_item(disk, &fat_private->root_directory);

    char buffer1[65536 * 2] = { 0 };
    char buffer2[65536] = { 0 };
    char buffer3[65536] = { 0 };

    struct fat_item_t *quijote = fat16_find_entry_in_dir(root_dir, "FRAG", "TXT");
    struct fat_descriptor_t *descriptor = fat16_open_item(disk, quijote);
    int read1 = fat16_read_file(descriptor, buffer1, 65536 * 2);
    int read2 = fat16_read_file(descriptor, buffer2, 65536);
    fat16_seek(descriptor, 0, SEEK_SET);
    int read3 = fat16_read_file(descriptor, buffer3, 65536);

    (void)read1;
    (void)read2;
    (void)read3;

    kfree(quijote);
    kfree(descriptor);

    struct fat_item_t *monguito = fat16_find_entry_in_dir(root_dir, "FOLDER1", " ");
    struct fat_descriptor_t *descriptor_dir = fat16_open_item(disk, monguito);
    struct fat_item_t *entry_found = fat16_find_entry_in_dir(descriptor_dir, "ASD3", "");

    int count = fat16_count_dir_items(descriptor_dir);
    (void)count;

    kfree(monguito);
    kfree(descriptor_dir);
    kfree(entry_found);

    struct path_root *path = pathparser_path_parse("0:/FOLDER1/ASD3");
    struct fat_descriptor_t *asd3 = fat16_open(disk, path->first, 0);

    pathparse_path_free(path);
    kfree(asd3);
    kfree(root_dir);

    return 0;
}

#endif
