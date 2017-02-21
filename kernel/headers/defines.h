#ifndef DEFINES_H
#define DEFINES_H

#define MASTER_COMMAND 0x20
#define MASTER_DATA 0x21
#define SLAVE_COMMAND 0xA0
#define SLAVE_DATA 0xA1
#define PIC_RESET 0x20

typedef void * vaddr_t;
typedef uintptr_t paddr_t;

struct multiboot_info {
	uint32_t	mi_flags;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_MEMORY. */
	uint32_t	mi_mem_lower;
	uint32_t	mi_mem_upper;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_BOOT_DEVICE. */
	uint8_t	mi_boot_device_part3;
	uint8_t	mi_boot_device_part2;
	uint8_t	mi_boot_device_part1;
	uint8_t	mi_boot_device_drive;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_CMDLINE. */
	char *	mi_cmdline;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_MODS. */
	uint32_t	mi_mods_count;
	struct multiboot_module * mi_mods_addr;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_{AOUT,ELF}_SYMS. */
	uint32_t	mi_elfshdr_num;
	uint32_t	mi_elfshdr_size;
	vaddr_t	mi_elfshdr_addr;
	uint32_t	mi_elfshdr_shndx;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_MMAP. */
	uint32_t	mi_mmap_length;
	vaddr_t	mi_mmap_addr;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_DRIVES. */
	uint32_t	mi_drives_length;
	vaddr_t	mi_drives_addr;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_CONFIG_TABLE. */
	void *	unused_mi_config_table;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_LOADER_NAME. */
	char *	mi_loader_name;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_APM. */
	void *	unused_mi_apm_table;

	/* Valid if mi_flags sets MULTIBOOT_INFO_HAS_VBE. */
	void *	unused_mi_vbe_control_info;
	void *	unused_mi_vbe_mode_info;
	paddr_t	unused_mi_vbe_interface_seg;
	paddr_t	unused_mi_vbe_interface_off;
	uint32_t	unused_mi_vbe_interface_len;
};

struct multiboot_mmap {
	uint32_t	mm_size;
	uint64_t	mm_base_addr;
	uint64_t	mm_length;
	uint32_t	mm_type;
};

struct multiboot_module {
  vaddr_t start;
  vaddr_t end;
  char* cmdline;
  uint32_t reserved;
};

#endif