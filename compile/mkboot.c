#ifdef linux
  #include <elf.h>
#else
  #include <sys/elf.h>
#endif

#include <sys/fcntl.h>

Elf32_Ehdr elfhdr;
Elf32_Phdr phdr, dphdr;
char buf[4096];

main(argc, argv)
	int argc;
	char **argv;
{
	int ifd, ofd;
	int text, data, bss;
	int count;
	int text_written = 0;
	int data_written = 0;

	if (argc < 3) {
		printf("usage: mkboot elf_file a.outfile \n");
		exit(1);
	}
	if ((ifd = open(argv[1], O_RDONLY)) ==  -1) {
		perror("open input");
		exit(1);
	}

		/* read ELF header */
	if (read(ifd, &elfhdr, sizeof(elfhdr)) < sizeof(elfhdr)) {
		perror("read elfhdr");
		exit(1);
	}

	if ((ofd = open(argv[2], O_RDWR | O_TRUNC | O_CREAT, 0777)) ==  -1) {
		perror("open aout");
		exit(1);
	}

	if (*(int *)(elfhdr.e_ident) != *(int *)(ELFMAG)){
		perror("elfmag");
		exit(1);
	}

	/*
	 * seek to program header spot and read text segment header.
	 */	
	if (lseek(ifd, elfhdr.e_phoff, 0) == -1) {
		perror("lseek");
		exit(1);
	}
	if (read(ifd, &phdr, sizeof(phdr)) < sizeof(phdr)) {
		perror("read phdr");
		exit(1);
	}
	/*
	 * read data segment header
	 */
	if (read(ifd, &dphdr, sizeof(dphdr)) < sizeof(dphdr)) {
		perror("read dphdr");
		exit(1);
	}

	text = phdr.p_memsz;
	data = dphdr.p_filesz;
	bss = dphdr.p_memsz - dphdr.p_filesz;

	/*
	 * seek to the start of the text segment
	 */
	if (lseek(ifd, phdr.p_offset, 0) == -1) {
		perror("lseek text");
		exit(1);
	}

	/* do text section first */
	while (text_written < text && 
	    (count = read(ifd, buf, sizeof(buf))) > 0) {
		if (count > text - text_written)
			count = (text - text_written);
		if (write(ofd, buf, count) < count) {
			perror("write text file");
			exit(1);
		}
		text_written += count;
	}

	/*
	 * seek to the start of the data segment
	 */
	if (lseek(ifd, dphdr.p_offset, 0) == -1) {
		perror("lseek data");
		exit(1);
	}

	/* Adavnce space between text and data */
	if (lseek(ofd, dphdr.p_vaddr - phdr.p_vaddr, 0) == -1) {
		perror("lseek data");
		exit(1);
	}

	while (data_written < data && 
	    (count = read(ifd, buf, sizeof(buf))) > 0) {
		if (count > data - data_written)
			count = (data - data_written);
		if (write(ofd, buf, count) < count) {
			perror("write data file");
			exit(1);
		}
		data_written += count;
	}
	printf("text: %d, data: %d, bss: %d, system size: %d (octets)\n", 
		text, data, bss, text+data);
	close(ofd);
	close(ifd);
}
