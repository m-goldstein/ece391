#ifndef TESTS_C
#define TESTS_C
#include "include/tests.h"
#include "include/x86_desc.h"
#include "include/lib.h"
#include "include/idt.h"
#include "include/page.h"
#include "include/terminal.h"
#include "include/fs.h"
#include "include/directory.h"
#include "include/keyboard.h"
#include "include/rtc.h"
#include "include/memory.h"
#include "include/task.h"
#include "include/sys_call.h"
#include "include/vga.h"
#include "include/sys.h"
#define PASS 1
#define FAIL 0
#define TEST_CASE_BUF 15
uint8_t test_entry[TEST_CASE_BUF];

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

void test_get_page(uint32_t page) {
    pte_t pte = *((pte_t*)&page);
    printf("Information for page: %x\n", page);
    printf("Global        : %d\n", pte.global);
    //printf("Dirty         : %d\n", pte.dirty);
    //printf("Accessed      : %d\n", pte.accessed);
    //printf("Cache Disabled: %d\n", pte.cache_dis);
    //printf("Write Through : %d\n", pte.w_thru);
    //printf("User Enable   : %d\n", pte.user_en);
    //printf("Read/Write    : %d\n", pte.rw_en);
    printf("Present       : %d\n", pte.present);
    //printf("Accessed      : %d\n", pte.accessed);
    printf("Address       : %d\n", pte.address);
}
void test_get_page_directory(uint32_t pde_) {
    pde_t pde = *((pde_t*)&pde_);
    printf("Information for page: %x\n", pde_);
    //printf("Global        : %d\n", pde.global);
    printf("Extended Paging : %d\n", pde.extended_paging);
    //printf("Accessed      : %d\n", pde.accessed);
    //printf("Cache Disabled: %d\n", pde.cache_dis);
    //printf("Write Through : %d\n", pde.w_thru);
    //printf("User Enable   : %d\n", pde.user_en);
    //printf("Read/Write    : %d\n", pde.rw_en);
    printf("Present       : %d\n", pde.present);
    //printf("Accessed      : %d\n", pde.accessed);
    printf("Address       : %d\n", pde.address);
}

void test_get_ext_page_directory(uint32_t pde_) {
    pde_ext_t pde = *((pde_ext_t*)&pde_);
    printf("Information for page: %x\n", pde_);
    //printf("Global        : %d\n", pde.global);
    printf("Extended Paging : %d\n", pde.extended_paging);
    //printf("Accessed      : %d\n", pde.accessed);
    //printf("Cache Disabled: %d\n", pde.cache_dis);
    //printf("Write Through : %d\n", pde.w_thru);
    //printf("User Enable   : %d\n", pde.user_en);
    //printf("Read/Write    : %d\n", pde.rw_en);
    printf("Present       : %d\n", pde.present);
    //printf("Accessed      : %d\n", pde.accessed);
    printf("Address       : %d\n", pde.address);
}
void test_page_fault() {
    int *ip;
    ip = 0x0;
    printf("%d ", *ip);
}
/* Checkpoint 1 tests */
int test_paging() {
	TEST_HEADER;
	test_get_page(page_table.pages[0]);
	test_get_page_directory(page_directory.directory_table[0]);
	test_get_ext_page_directory(page_directory.directory_table[1]);

	// Test video memory
	test_get_page(page_table.pages[0xB8]);

	// Trigger page fault
	test_page_fault();
	return PASS;

}

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* divideBy0Test
 *
 * Tries to divide by 0
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Divide by exception should be created
 * Coverage: covers exception 0 should be in a infinite loop
 */
int divideBy0Test()
{
	TEST_HEADER;
	//asm volatile("int $0");
	int x,y,z;
	x = 5;
	y = 0;
	z = x/y;
	return FAIL;
}

/* invalOpcode
 *
 * Forces an invalid opcode fault
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: throws a invalid opcode fault
 * Coverage: testing exceptions
 */
int invalOpcode()
{
	TEST_HEADER;
	invalid_opcode();
	return FAIL;
}

/* sys_call_test
 *
 * Forces an invalid opcode fault
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: throws a invalid opcode fault
 * Coverage: testing exceptions
 */
int sys_call_test()
{
	TEST_HEADER;
	int i;
	for(i = 0; i< 10; i++)
		sys_call(i);
	return PASS;
}

// add more tests here

/* Checkpoint 2 tests */
/* rtc_set_freq_test
 *
 * Tests setting the frequency
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: changes the rtc freq
 * Coverage: helper function
 */
int rtc_set_freq_test() {
    TEST_HEADER;
    int hz;
    set_rtc_freq(6);
    outb(0x8A, 0x70);
    hz = inb(0x71);
    printf(" %x ", hz);
    hz &= 0x0F;
    if(hz == 6) {
        printf("good hz");
        return PASS;
    }
    printf("bad hz");
    return FAIL;
}
/* rtc_write_test_pwr_of_2
 *
 * Tests setting the frequency
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: changes the rtc freq
 * Coverage: helper function
 */
int rtc_write_test_pwr_of_2() {
    TEST_HEADER;
    uint32_t fd = 0;
    uint32_t nbytes = 0;
    uint32_t freq = 512;
    void * ptr = &freq;
    int hz;
    rtc_write(fd, ptr, nbytes);

    hz = inb(0x71);
    hz &= 0x0F;
    if(hz == 7) {
        printf(" %x ", hz);
        return PASS;
    }
    printf(" %x ", hz);
    return FAIL;
}
/* rtc_write_test_npwr_of_2
 *
 * Tests setting the frequency
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: changes the rtc freq
 * Coverage: helper function
 */
int rtc_write_test_npwr_of_2() {
    uint32_t fd = 0;
    uint32_t nbytes = 0;
    uint32_t freq = 1023;
    void * ptr = &freq;
    int hz;
    rtc_write(fd, ptr, nbytes);
    hz = inb(0x71);
    hz &= 0x0F;
    if(hz == 6) {
        printf(" %x ", hz);
        return PASS;
    }
    printf(" %x ", hz);
    return FAIL;
}
/* rtc_read_test
 *
 * Tests reading the frequency
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: changes the rtc freq
 * Coverage: helper function
 */
/*
int rtc_read_test() {
    TEST_HEADER;
    uint32_t fd = 0;
    uint32_t nbytes = 0;
    uint32_t freq = 512;
    void* ptr = &freq;
    if(rtc_read(fd, ptr, nbytes) == 0){
        return PASS;
    }
    return FAIL;
}
*/
/* rtc_freq_print_test
 *
 * Tests printing the freq
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: changes the rtc freq
 * Coverage: helper function
 */
int rtc_freq_print_test(uint32_t freq) {
    TEST_HEADER;
    uint32_t fd = 0;
    uint32_t nbytes = 0;
    void* ptr = &freq;
    int count = 0;
    rtc_write(fd, ptr, nbytes);

	int i;
    for(i=0;i<freq*3;i++) {
        while(rtc_read(fd, ptr, nbytes) != 0) {
            count = get_rtc_intr();
            printf("tests.c %d ", count);
        }
    }
    return PASS;
}
/* memory_test
 *
 * Tests memory acceses
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: tries to access memory locations
 * Coverage: paging
 */
  int memory_test()
  {
      TEST_HEADER;
      pde_t pde;
      pte_t pte;
      pde = *((pde_t*)&(page_directory.directory_table[4]));
      pte = *((pte_t*)&(page_table.pages[11]));
      //uint32_t user_vaddr = pte.address;
      //uint32_t user_paddr = (uint32_t)&pte;
      uint32_t virtual_addr = 0x800000;
      uint32_t physical_addr = (uint32_t)&pde;
      printf("Physical Address: %x\nVirtual Address: %x\n", physical_addr, virtual_addr);
      void* page_directory = __map_page_directory(0x800000, virtual_addr, PRESENT | RW_EN | USER_EN);
      //__map_page_directory(physical_addr, virtual_addr+0x400000, 0);
      //__map_page_directory(physical_addr, virtual_addr+0x800000, 0);
      //void* user_page = __map_user_page(0x800000, 0);
      //void* page      = __map_page(virtual_addr, 0);
      //getCurrentSession()->buffer = (uint8_t*)user_page;
      printf("Page directory mapped to %x\n", page_directory);
      //printf("User page mapped to: %x\n", user_page);
      //printf("Page mapped to: %x\n", page);

      return PASS;
  }
/* fs_test
 *
 * test file descriptor inodes
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: NONE
 * Coverage: file descriptors
 */
int fs_test()
{
    TEST_HEADER;
    printf("Filesystem image base address: %x\n", fs_img_addr);
    printf("Showing open file descriptors:\n");
    int i;
    for (i = 0; i < MAX_NUM_FD; i++) {
	fs_t* fd = &(curr_file_table->files[i]);
	printf("File #%d: %x\n",i, fd->inode);
    }
    return PASS;
}
/* std_io_test
 *
 * Test to read and write the terminal
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: NONE
 * Coverage: terminal
 */
int std_io_test()
{

    TEST_HEADER;
    init_file_table(curr_file_table);
    fs_t std_in = curr_file_table->files[0];
    fs_t std_out = curr_file_table->files[1];
    vga_printf("STDIN buffer: %x\n", std_in.buffer);
    vga_printf("STDOUT buffer: %x\n", std_out.buffer);
    vga_printf("Reading from STDIN...\n");
    std_in.op_ptr->read(0, std_in.buffer, TEST_CASE_BUF);
    vga_printf("Writing to STDOUT...\n");
    std_out.op_ptr->write(0, std_in.buffer, TEST_CASE_BUF);

    return PASS;
}
/* inode_list_test
 *
 * Lists all the files
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: NONE
 * Coverage: file system
 */
int inode_list_test() {
  /*  TEST_HEADER;
    int i;
		boot_block_t* directoryInfo = (boot_block_t*)(fs_img_addr);
		uint32_t current_inode_num;
		inode_t* current_inode;
		for (i = 0; i < directoryInfo->num_dirs; i++) { // First directory entry is boot stuff
		    current_inode_num = ((dentry_t)(directoryInfo->directory[i])).inode_num;
		    current_inode = (inode_t*)((uint8_t*)fs_img_addr + ((current_inode_num+1)  * BLOCK_SIZE));
		    printf("file_name: %s file_type: %d file_size: %d\n", ((dentry_t)(directoryInfo->directory[i])).file_name,((dentry_t)(directoryInfo->directory[i])).file_type, current_inode->length);
		}*/
		return PASS;
}
/* fs_open_test
 *
 * Tests opening the file descriptors
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: NONE
 * Coverage:  file descriptors/file system
 */
int fs_open_test()
{
    /*
    TEST_HEADER;
    uint8_t* fname_0 = (uint8_t*)"frame0.txt";
    uint8_t* fname_1 = (uint8_t*)"frame1.txt";
    uint8_t* fname_2 = (uint8_t*)"grep";
    uint8_t* fname_3 = (uint8_t*)"ls";
    uint8_t* fname_4 = (uint8_t*)"fish";
    uint8_t* fname_5 = (uint8_t*)"verylargetextwithverylongname.txt";
    curr_file_table = get_curr_file_table();
    fs_t* file_0     = &curr_file_table->files[2];
    fs_t* file_1     = &curr_file_table->files[3];
    fs_t* file_2     = &curr_file_table->files[4];
    fs_t* file_3     = &curr_file_table->files[5];
    fs_t* file_4     = &curr_file_table->files[6];
    fs_t* file_5     = &curr_file_table->files[7];


    if (file_0->io[OPEN](2, fname_0, 0) == FS_ERROR)
	return FAIL;
    printf("File #%d (%s) is open.\n", 2, fname_0);

    if (file_1->io[OPEN](3, fname_1, 0) == FS_ERROR)
	return FAIL;
    printf("File #%d (%s) is open. \n", 3, fname_1);

    if (file_2->io[OPEN](4, fname_2, 0) == FS_ERROR)
	return FAIL;
    printf("File #%d (%s) is open, \n", 4, fname_2);

    if (file_3->io[OPEN](5, fname_3, 0) == FS_ERROR)
	return FAIL;
    printf("File #%d (%s) is open.\n", 5, fname_3);

    if (file_4->io[OPEN](6, fname_4, 0) == FS_ERROR)
	return FAIL;
    printf("File #%d (%s) is open. \n", 6, fname_4);

    if (file_5->io[OPEN](7, fname_5, 0) == FS_ERROR)
	return FAIL;
    printf("File #%d (%s) is open, \n", 7, fname_5);
*/
    return PASS;
}
/* fs_read_test
 *
 * Tests reading files from file descriptors
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: NONE
 * Coverage:  file descriptors/file system
 */
int fs_read_test()
{
    /*
    TEST_HEADER;
    uint8_t buf[] = "\nPRESS ANY KEY TO CONTINUE\n";
    uint32_t length = strlen((int8_t*)buf);
    uint8_t* fname_0 = (uint8_t*)"frame0.txt";
    uint8_t* fname_1 = (uint8_t*)"frame1.txt";
    uint8_t* fname_2 = (uint8_t*)"grep";
    uint8_t* fname_3 = (uint8_t*)"ls";
    uint8_t* fname_4 = (uint8_t*)"fish";
    uint8_t* fname_5 = (uint8_t*)"verylargetextwithverylongname.txt";
    curr_file_table = get_curr_file_table();
    terminal_session_t* stdio = curr_file_table->files[0].sess;

    fs_t* file_0     = &curr_file_table->files[2];
    fs_t* file_1     = &curr_file_table->files[3];
    fs_t* file_2     = &curr_file_table->files[4];
    fs_t* file_3     = &curr_file_table->files[5];
    fs_t* file_4     = &curr_file_table->files[6];
    fs_t* file_5     = &curr_file_table->files[7];
    int test_num = 0;
    uint8_t mybuffer[128];
    //uint32_t pos = file_0->f_pos;
    for (test_num = 0; test_num < 6; test_num ++) {
	switch (test_num) {
	    case 0:
		{
		    while(file_0->f_pos < file_0->inode->length){
			uint32_t bread = file_0->io[READ](2, mybuffer, 128);
			stdio->term_io[WRITE](0, mybuffer, bread);
		    }
		    printf("File #%d (%s) was read.\n", 2, fname_0);
		    break;
		}
	    case 1:
		{

		    while(file_1->f_pos < file_1->inode->length){
			uint32_t bread = file_1->io[READ](3, mybuffer, 128);
			stdio->term_io[WRITE](0, mybuffer, bread);
		    }
		    printf("File #%d (%s) was read.\n", 3, fname_1);
		    break;
		}
	    case 2:
		{

		    while(file_2->f_pos < file_2->inode->length){
			uint32_t bread = file_2->io[READ](4, mybuffer, 128);
			stdio->term_io[WRITE](0, mybuffer, bread);
		    }
		    printf("File #%d (%s) was read.\n", 4, fname_2);
		    break;
		}
	    case 3:
		{

		    while(file_3->f_pos < file_3->inode->length){
			uint32_t bread = file_3->io[READ](5, mybuffer, 128);
			stdio->term_io[WRITE](0, mybuffer, bread);
		    }
		    printf("File #%d (%s) was read.\n", 5, fname_3);
		    break;
		}
	    case 4:
		{

		    while(file_4->f_pos < file_4->inode->length){
			uint32_t bread = file_4->io[READ](6, mybuffer, 128);
			stdio->term_io[WRITE](0, mybuffer, bread);
		    }
		    printf("File #%d (%s) was read.\n", 6, fname_4);
		    break;
		}
	    case 5:
		{

		    while(file_5->f_pos < file_5->inode->length){
			uint32_t bread = file_5->io[READ](7, mybuffer, 128);
			stdio->term_io[WRITE](0, mybuffer, bread);
		    }
		    printf("File #%d (%s) was read.\n", 7, fname_5);
		    break;
		}
	    default:
		break;
	}

	getCurrentSession()->term_io[WRITE](0,buf,length);
	getCurrentSession()->term_io[READ](0, getCurrentSession()->buffer, 1);
    }

    printf("Setting position in file back to zero.\n");
    seek_to_position(file_0, 0);
    seek_to_position(file_1, 0);
    seek_to_position(file_2, 0);
    seek_to_position(file_3, 0);
    seek_to_position(file_4, 0);
    seek_to_position(file_5, 0);
    */
    return PASS;
}
/* fs_write_test
 *
 * Tests writting to files
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: NONE
 * Coverage:  file descriptors/file system
 */
int fs_write_test()
{

    TEST_HEADER;
    //terminal_session_t* stdio = curr_file_table->files[0].sess;
    //uint8_t buf[] = "\nPRESS ANY KEY TO CONTINUE\n";
    //uint32_t length = strlen((int8_t*)buf);
    //uint8_t* fname_0 = (uint8_t*)"frame0.txt";
    //uint8_t* fname_1 = (uint8_t*)"frame1.txt";
    //uint8_t* fname_2 = (uint8_t*)"grep";
    //uint8_t* fname_3 = (uint8_t*)"ls";
    //uint8_t* fname_4 = (uint8_t*)"fish";
    //uint8_t* fname_5 = (uint8_t*)"verylargetextwithverylongname.txt";
/*
    fs_t* file_0     = &curr_file_table->files[2];
    fs_t* file_1     = &curr_file_table->files[3];
    fs_t* file_2     = &curr_file_table->files[4];
    fs_t* file_3     = &curr_file_table->files[5];
    fs_t* file_4     = &curr_file_table->files[6];
    fs_t* file_5     = &curr_file_table->files[7];

    int test_num = 0;
    for (test_num = 0; test_num < 6; test_num ++) {
	switch (test_num) {
	    case 0:
		{
		    if (file_0->io[WRITE](2, (uint8_t*)"Linux!\n", file_0->inode->length) == -1) // For now this is how we do it
			return PASS;

		    if (file_0->io[WRITE](2, (uint8_t*)"Linux!\n", file_0->inode->length) == FS_ERROR)
			return FAIL;
		    printf("File #%d (%s) was written.\n", 2, fname_0);
		    stdio->term_io[WRITE](0, file_0->buffer, file_0->inode->length);
		    break;
		}
	    case 1:
		{
		    if (file_1->io[WRITE](3, (uint8_t*)"Gentoo!\n", file_1->inode->length) == FS_ERROR)
			return FAIL;
		    printf("File #%d (%s) was written.\n", 3, fname_1);
		    stdio->term_io[WRITE](0, file_1->buffer, file_1->inode->length);
		    break;
		}
	    case 2:
		{
		    if (file_2->io[WRITE](4, (uint8_t*)"Fedora!\n", file_2->inode->length) == FS_ERROR)
			return FAIL;
		    printf("File #%d (%s) was written.\n", 4, fname_2);
		    stdio->term_io[WRITE](0, file_2->buffer, file_2->inode->length);
		    break;
		}
	    case 3:
		{
		    if (file_3->io[WRITE](5, (uint8_t*)"Arch!\n", file_3->inode->length) == FS_ERROR)
			return FAIL;
		    printf("File #%d (%s) was written.\n", 5, fname_3);
		    stdio->term_io[WRITE](0, file_3->buffer, file_3->inode->length);
		    break;
		}
	    case 4:
		{
		    if (file_4->io[WRITE](6, (uint8_t*)"CentOS!\n", file_4->inode->length) == FS_ERROR)
			return FAIL;
		    printf("File #%d (%s) was written.\n", 6, fname_4);
		    stdio->term_io[WRITE](0, file_4->buffer, file_4->inode->length);
		    break;
		}
	    case 5:
		{
		    if (file_5->io[WRITE](7, (uint8_t*)"...Ubuntu!\n", file_5->inode->length) == FS_ERROR)
			return FAIL;
		    printf("File #%d (%s) was written.\n", 7, fname_5);
		    stdio->term_io[WRITE](0, file_5->buffer, file_5->inode->length);
		    break;
		}
	    default:
		break;
	}
	getCurrentSession()->term_io[WRITE](0,buf,length);
	getCurrentSession()->term_io[READ](0, getCurrentSession()->buffer, 1);
    }
*/
    /* this is only for the test cases so we can rerun the same tests*/
  //  printf("Setting position in file back to zero.\n");
    //seek_to_position(file_0, 0);
    //seek_to_position(file_1, 0);
    //seek_to_position(file_2, 0);
    //seek_to_position(file_3, 0);
    //seek_to_position(file_4, 0);
    //seek_to_position(file_5, 0);
    return PASS;
}
/* fs_close_test
 *
 * Tests closing file descriptors
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: NONE
 * Coverage: file descriptors/file system
 */
int fs_close_test()
{
    /*TEST_HEADER;
    uint8_t* fname_0 = (uint8_t*)"frame0.txt";
    uint8_t* fname_1 = (uint8_t*)"frame1.txt";
    uint8_t* fname_2 = (uint8_t*)"grep"      ;
    uint8_t* fname_3 = (uint8_t*)"ls"        ;
    uint8_t* fname_4 = (uint8_t*)"fish"      ;
    uint8_t* fname_5 = (uint8_t*)"verylargetextwithverylongname.txt";


    fs_t* file_0     = &curr_file_table->files[2];
    fs_t* file_1     = &curr_file_table->files[3];
    fs_t* file_2     = &curr_file_table->files[4];
    fs_t* file_3     = &curr_file_table->files[5];
    fs_t* file_4     = &curr_file_table->files[6];
    fs_t* file_5     = &curr_file_table->files[7];

    if (file_0->io[CLOSE](2, fname_0, 0) == FS_ERROR)
	return FAIL;
    printf("File #%d (%s) is closed.\n", 2, fname_0);
    if (file_1->io[CLOSE](3, fname_1, 0) == FS_ERROR)
	return FAIL;
    printf("File #%d (%s) is closed. \n", 3, fname_1);
    if (file_2->io[CLOSE](4, fname_2, 0) == FS_ERROR)
	return FAIL;
    printf("File #%d (%s) is closed. \n", 4, fname_2);

    if (file_3->io[CLOSE](5, fname_3, 0) == FS_ERROR)
	return FAIL;
    printf("File #%d (%s) is closed.\n", 3, fname_3);
    if (file_4->io[CLOSE](6, fname_4, 0) == FS_ERROR)
	return FAIL;
    printf("File #%d (%s) is closed. \n", 4, fname_4);
    if (file_5->io[CLOSE](7, fname_5, 0) == FS_ERROR)
	return FAIL;
    printf("File #%d (%s) is closed. \n", 5, fname_5);
*/
    return PASS;
}
int fs_dir_open_test()
{
    /*
    TEST_HEADER;
    uint8_t* fname = (uint8_t*)"frame0.txt";
    uint8_t* dirname = (uint8_t*)".";
    fs_t* file = &curr_file_table->files[2];
    fs_t* dir  = &curr_file_table->files[3];
    dir->io[OPEN] = dir_open;

    if (file->io[OPEN](2, fname, 0) == FS_ERROR) {
	printf("Error opening file.\n");
	return FAIL;
    }
    printf("File %s opened.\n", fname);
    if (dir->io[OPEN](3, dirname, 0) == FS_ERROR) {
	printf("Error opening directory.\n");
	return FAIL;
    }
    printf("Directory %s opened.\n", dirname);
    */
    return PASS;
}
int fs_dir_read_test()
{
/*
    TEST_HEADER;
    fs_t* dir = &curr_file_table->files[3];
    dir->io[OPEN]   = dir_open;
    dir->io[CLOSE]  = dir_close;
    dir->io[READ]   = dir_read;
    dir->io[WRITE]  = dir_write;
    terminal_session_t* stdio = (terminal_session_t*) &curr_file_table->files[0].sess;
    uint8_t buffer[32];
    int32_t nbytes = dir->io[READ](3, buffer, 0);
    if (nbytes == -1) {
	printf("Error reading directory.\n");
	return FAIL;
    }
    printf("%s", buffer);
    stdio->term_io[WRITE](0, buffer, nbytes);
    */
    return PASS;
}
int fs_dir_write_test()
{
    TEST_HEADER;
    return PASS;
}
int fs_dir_close_test()
{
    TEST_HEADER;
    /*
    fs_t* file = &curr_file_table->files[2];
    fs_t* dir  = &curr_file_table->files[3];
    if (file->io[CLOSE](2, (uint8_t*)" ", 0) == FS_ERROR) {
	printf("Error closing file.\n");
	return FAIL;
    }
    if (dir->io[CLOSE](3, (uint8_t*)" ", 0) == FS_ERROR) {
	printf("Error closing directory.\n");
	return FAIL;
    }*/
    return PASS;
}
int negative_len_io_read_test()
{/*
    uint8_t* fname = (uint8_t*)"frame0.txt";
    fs_t* file  = &curr_file_table->files[2];
    fs_t* stdio = &curr_file_table->files[0];
    if (stdio->sess->term_io[WRITE](0, (uint8_t*)"test", 4) == -1) {
	printf("Rejected positive length terminal I/O\n"); // should not occur
    }
    if (stdio->sess->term_io[WRITE](0, (uint8_t*)"invalid", -1) == -1) {
	printf("Rejected negative length terminal I/O\n");
    }
    if (stdio->sess->term_io[READ](0, (uint8_t*)"test", 4) == -1) {
	printf("Rejcted positive length terminal I/O\n"); // should not occur
    }
    if (stdio->sess->term_io[READ](0, (uint8_t*)"invalid", -1) == -1) {
	printf("Rejected negative length terminal I/O\n");
    }

    file->io[OPEN](2, fname, 0);

    if (file->io[READ](2, file->buffer, file->inode->length) == -1) {
	printf("Rejcted positive length file I/O\n");  // should read a fish
    }
    stdio->sess->term_io[WRITE](0, file->buffer, file->inode->length);

    if (file->io[READ](2, file->buffer, -1) == -1) {        // should not read
	printf("Rejected negative length file I/O\n");
    }
    stdio->sess->term_io[WRITE](0, file->buffer, -1);       // should not print anything

    printf("\n");
    */
    return PASS;
}
/* terminal_read_test
 *
 * Should no longer be able to type after pressing enter
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: NONE
 * Coverage: terminal
 */
int terminal_read_test(){
    TEST_HEADER;
    while(1);

    return FAIL;
}

/* Checkpoint 3 tests */
int test_switch_to_user() {
    TEST_HEADER;
    /*uint8_t* command = "shell";
    uint8_t* args    = "";
    uint8_t* swapper_cmd = "";
    uint8_t* swapper_args = "";
    proc_t swapper;
    save_regs(&(swapper.regs));
    swapper.terminal_id = 0;
    swapper.active      = 1;
    swapper.parent      = NULL;
    swapper.child       = NULL;

    *swapper.command     = *swapper_cmd;
    *swapper.args        = *swapper_args;
    init_task(&swapper)    ;
    mk_proc(command , args);
    switch_to_user_proc(1) ;
    */
    return PASS;
}
int execute_test() {
    TEST_HEADER;
    //execute((const uint8_t*)"shell");
    return PASS;
}
int open_test() {
    TEST_HEADER;
    open((const uint8_t*)"frame0.txt");
    return PASS;
}
int close_test() {
    TEST_HEADER;
    close(2);
    return PASS;
}
int read_test() {
    TEST_HEADER;
    uint8_t buf[128];
    terminal_session_t* stdio = curr_file_table->files[0].sess;
    fs_t* file = &curr_file_table->files[2];
    while(file->f_pos < file->inode->length){
	uint32_t bread = read(2, buf, 128);
	stdio->op_ptr->write(0, buf, bread);
    }
    return PASS;
}
int write_test() {
    TEST_HEADER;
    uint8_t buf[128];
    terminal_session_t* stdio = curr_file_table->files[0].sess;
    fs_t* file = &curr_file_table->files[2];
    while(file->f_pos < file->inode->length){
	uint32_t bread = write(2, buf, 128);
	stdio->op_ptr->write(0, buf, bread);
    }
    return PASS;
}
int halt_test() {
    TEST_HEADER;
    exception_handler(26);
    return PASS;
}
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */
static uint32_t interrupt_pointers[] =
{
    (uint32_t)idt0, (uint32_t)idt1, (uint32_t)idt2, (uint32_t)idt3, (uint32_t)idt4, (uint32_t)idt5, (uint32_t)idt6, (uint32_t)idt7, (uint32_t)idt8, (uint32_t)idt9,
    (uint32_t)idt10, (uint32_t)idt11, (uint32_t)idt12, (uint32_t)idt13, (uint32_t)idt14, (uint32_t)idt15, (uint32_t)idt16, (uint32_t)idt17, (uint32_t)idt18, (uint32_t)idt19,
    (uint32_t)idt20, (uint32_t)idt21, (uint32_t)idt22, (uint32_t)idt23, (uint32_t)idt24, (uint32_t)idt25, (uint32_t)idt26, (uint32_t)idt27, (uint32_t)idt28, (uint32_t)idt29,
    (uint32_t)idt30, (uint32_t)idt31
};
/* exception_content_test
 *
 * Testing exceptions
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: NONE
 * Coverage: paging
 */
int exception_content_test() {
    TEST_HEADER;
    int i;
    int result = PASS;
    for (i = 0; i < NUM_OF_EXCEPT; i++) {
	uint32_t offset = (idt[i].offset_31_16 << 16) + idt[i].offset_15_00;
	if (offset != (uint32_t)interrupt_pointers[i]) {
	    result = FAIL;
	}
	if (idt[i].seg_selector != KERNEL_CS) {
	    result = FAIL;
	}
	if (idt[i].present != 1) {
	    result = FAIL;
	}
	if (idt[i].dpl != 0) {
	    result = FAIL;
	}
    }

    return result;
}
/* terminal_read_test
 *
 * Tries to read from terminal after it is closed nothing should be able to happen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: NONE
 * Coverage: terminal
 */
int close_terminal_test() {
    TEST_HEADER;
    int result = PASS;
    getCurrentSession()->op_ptr->close(0,(uint8_t*)' ',128);
    getCurrentSession()->op_ptr->read(0,(uint8_t*)' ',128);
    return result;
}
int vm_alloc_tests()
{
    TEST_HEADER;
    return 0;
}
int test_terminal_switching()
{
    term_switch(1);
		return 0;
}
/* Test suite entry point */
void launch_tests(){
    vga_printf("Testing...\n");
    test_terminal_switching();
    //vm_alloc_tests();
    /*getCurrentSession()->op_ptr->write(0,buf,TEST_CASE_BUF);
    getCurrentSession()->op_ptr->read(0, getCurrentSession()->buffer, TEST_CASE_BUF);
    uint32_t length = strlen((const int8_t*)getCurrentSession()->buffer);
    if (length > 15)
	length = 15;
    int8_t test_num = parse_user_input(getCurrentSession()->buffer, length);
    switch(test_num)
    {
	case 0:
	    TEST_OUTPUT("idt_test", idt_test());
	    break;
	case 1:
	    TEST_OUTPUT("divideBy0Test", divideBy0Test());
	    break;
	case 2:
	    TEST_OUTPUT("paging init test" , test_paging());
	    break;
	case 3:
	    TEST_OUTPUT("memory fn test", memory_test());
	    break;
	case 4:
	    TEST_OUTPUT("init fs test", fs_test());
	    break;
	case 5:
	    TEST_OUTPUT("init std i/o tests", std_io_test());
	    break;
	case 6:
	    TEST_OUTPUT("inode_list_test", inode_list_test());
	    break;
	case 7:
	    TEST_OUTPUT("Invalid Opcode" , invalOpcode());
	    break;
	case 8:
	    TEST_OUTPUT("Sys call test" , sys_call_test());
	    break;
	case 9:
	    TEST_OUTPUT("Sys call test" , sys_call_test());
	    break;
	case 10:
	    TEST_OUTPUT("Term Read Test", terminal_read_test());
	    break;
	case 11:
	    TEST_OUTPUT("Close Terminal Test", close_terminal_test());
	    break;
	case 12:
	    TEST_OUTPUT("rtc test" , rtc_freq_print_test(freq));
	    ctrlL();
	    for(;freq != 1;)
	    {
		freq /= 2;
		rtc_freq_print_test(freq);
		ctrlL();
	    }
	    break;
	case 13:
	    TEST_OUTPUT("File open test", fs_open_test());
	    break;
	case 14:
	    TEST_OUTPUT("File read test", fs_read_test());
	    break;
	case 15:
	    TEST_OUTPUT("File write test", fs_write_test());
	    break;
	case 16:
	    TEST_OUTPUT("File close test", fs_close_test());
	    break;
	case 17:
	    TEST_OUTPUT("Directory open test", fs_dir_open_test());
	    break;
	case 18:
	    TEST_OUTPUT("Directory read test", fs_dir_read_test());
	    break;
	case 19:
	    TEST_OUTPUT("Directory write test", fs_dir_write_test());
	    break;
	case 20:
	    TEST_OUTPUT("Directory close test", fs_dir_close_test());
	    break;
	case 21:
	    TEST_OUTPUT("Terminal/FS I/O reject negative length reads", negative_len_io_read_test());
	    break;
	case 22:
	    break;
	case 23:
	    break;
	case 24:
	    TEST_OUTPUT("Switch to user mode test", test_switch_to_user());
	    break;
	case 25:
	    TEST_OUTPUT("Execute System Call test", execute_test());
	    break;
	case 26:
	    TEST_OUTPUT("Open System call test", open_test());
	    break;
	case 27:
	    TEST_OUTPUT("Close System Call test", close_test());
	    break;
	case 28:
	    TEST_OUTPUT("Read system call test", read_test());
	    break;
	case 29:
	    TEST_OUTPUT("Write system call test", write_test());
	    break;
	case 30:
	    TEST_OUTPUT("VM SLAB ALLOC Tests", vm_alloc_tests());
	    break;
	default:
	    vga_printf("Invalid entry. Valid tests are:\n");
	    vga_printf("0 -- IDT test\n1 -- Divide by 0 test\n2 -- Paging init test\n3 -- Memory functions test\n4 -- Init Filesystem test\n5 -- STD I/O tests\n6 -- Inode list test\n7 -- Invalid Opcode test\n8 -- System call test\n9 -- System call test\n10 -- Terminal read test\n11 -- Terminal close test\n12 -- RTC Functions Test\n13 -- FS File Open test\n14 -- FS File Read test\n15 -- FS File Write test\n16 -- FS File Close test\n17 -- FS Directory Open Test\n18 -- FS Directory Read Test\n19 -- FS Directory Write Test\n20 -- FS Directory Close Test\n");
	    vga_printf("PRESS ENTER TO SCROLL\n");
	    getCurrentSession()->op_ptr->read(0, (uint8_t*)" ", 0);
	    clear_terminal_buffer(0);
	    vga_printf("21 -- I/O Negative Length Read Test\n22 -- Map User Page Test\n23 -- Map Kernel Page Test\n24 -- Switch to user mode\n25 -- Execute System Call Test\n");
	    vga_printf("Press <F2> to continue.\n");
	    break;
    }
    */
    f2_key_flag = 0;
    // launch your tests here
}
#endif
