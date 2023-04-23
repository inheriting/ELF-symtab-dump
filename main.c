/*      This program reads an ELF binary file and displays information about its symbol table        */
#include <elf.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>


/*      Function to get the size of the ELF file         */
off_t get_elf_size(int elf_file_fd) {
    struct stat file_stats;
    fstat(elf_file_fd, &file_stats);
    return file_stats.st_size;
}

/*      Function to display the symbol table of the ELF file         */
void display_symbol_name(char *addr, Elf64_Ehdr *ehdr, Elf64_Shdr *shdr) {
    Elf64_Sym *symtab = NULL;
    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type != SHT_SYMTAB)
            continue;
                
        symtab = (Elf64_Sym *) (addr + shdr[i].sh_offset);

        int sym_link_idx = shdr[i].sh_link;
        int num_syms = shdr[i].sh_size / sizeof(Elf64_Sym);
        
        char *sym_name_offset = addr + shdr[sym_link_idx].sh_offset;

        for (int i = 0; i < num_syms; i++) {
            printf("\nSymbol name: %s\n", sym_name_offset + symtab[i].st_name);
            printf("Symbol value: %lx\n", symtab[i].st_value);
            printf("Symbol size: %lu\n", symtab[i].st_size);
            printf("Symbol binding: %d\n", ELF64_ST_BIND(symtab[i].st_info));
            printf("Symbol type: %d\n", ELF64_ST_TYPE(symtab[i].st_info));
            printf("Symbol visibility: %d\n", symtab[i].st_other & 0x3);
            printf("Symbol section index: %d\n", symtab[i].st_shndx);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) { printf("Usage: %s <filename>\n", argv[0]); }
  
    int elf_file_fd = open(argv[1], O_RDONLY);
    if (!elf_file_fd) { puts("No file descriptor\n\n"); return 1; }

    off_t file_size = get_elf_size(elf_file_fd);

    /*      Map the file into memory     */
    char *addr = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, elf_file_fd, 0);
    if (addr == MAP_FAILED) { puts("Mapping Error\n\n"); return 1;}

    Elf64_Ehdr *ehdr = (Elf64_Ehdr*) addr;

    Elf64_Shdr *shdr = (Elf64_Shdr *)(addr + ehdr->e_shoff);

    Elf64_Sym *symtab = NULL;
    char *string_table = addr + shdr[ehdr->e_shstrndx].sh_offset;


    display_symbol_name(addr, ehdr, shdr);

    munmap(addr, file_size);
    close(elf_file_fd);        
}
