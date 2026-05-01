/**
 * @file
 * @ingroup kernel_code
 * @brief API minima de lectura de sectores para `06_disk_io`.
 */

#ifndef DISK_H_
#define DISK_H_

#define DISK_SECTOR_SIZE 512

void setup_disk_io(void);
void register_disk_monitor_commands(void);
int disk_is_ready(void);
unsigned int disk_get_sector_size(void);
int disk_get_selected_device_index(void);
int disk_read_sectors(unsigned int lba, unsigned char count, void * buffer);

#endif /* DISK_H_ */
