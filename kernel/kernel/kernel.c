#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <kernel/tty.h>

#include <kernel/vga.h>

#include <kernel/io.h>

#include <kernel/disk.h>

#include <kernel/cpu.h>

const char vv[] = "0123456789abcdef";
unsigned char IDT_TABLE[256*8];
unsigned char GDT_TABLE[4*8];
char* hexdump(unsigned int v, char vvv[9]) 
{
	
	vvv[0] = vv[(v/268435456)];
	v=(v%268435456);
	vvv[1] = vv[(v/16777216)];
	v=(v%16777216);
	vvv[2] = vv[(v/1048576)];
	v=(v%1048576);
	vvv[3] = vv[(v/65536)];
	v=(v%65536);
	vvv[4] = vv[(v/4096)];
	v=(v%4096);
	vvv[5] = vv[(v/256)];
	v=(v%256);
	vvv[6] = vv[v/16];
	v=(v%16);
	vvv[7] = vv[v];
	vvv[8] = '\0';return vvv;
}


void halt(){asm volatile ("hlt");}
void failwith(char * err){printf("KERNEL FAILURE: %s",err);halt();}
void kernel_early(void)
{
    outb(0x3D4, 0x0F);
    outb(0x3D5, 0xFFFF);
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char )((0xFFFF>>8)&0xFF));
    terminal_initialise();
}

char* pad_num(char* v){
	if(strlen(v)==1) {char* dest = "00"; memcpy(dest+1,v,1); return dest;} else {return v;}
	
}
struct div_t{
  int quot;
  int rem;
};
struct div_t div (int a, int b)
{
    struct div_t c = {.quot = a/b,.rem = a%b};
    return c;
}
struct time{
    char hours;
    char minutes;
    char seconds;
};
struct time get_time(){
    struct time t;
    char s = ask_cmos(0x00);
	char m = ask_cmos(0x02);
	char h = ask_cmos(0x04);
    s = (s & 0x0F)+(s/16)*10;
    m = (m & 0x0F) + ((m / 16) * 10);
    h = ((h & 0x0F) + (((h & 0x70) / 16) * 10) ) | (h & 0x80);
    t.hours = h;
    t.minutes = m;
    t.seconds = s;
    return t;
}
struct time add_time(struct time a, struct time b){
    struct time c;
    struct div_t ss = div(a.seconds+b.seconds,60);
    c.seconds = ss.rem;
    struct div_t mm = div(a.minutes+b.minutes,60);

    c.minutes = mm.rem+ss.quot;
    struct div_t hh = div(a.hours+b.hours,24);
    c.hours = hh.rem + mm.quot;
    
    return c;
}
void print_time(struct time now)
{
    char* hh;
	printf(pad_num(itoa(now.hours,hh,10)));
	printf(":");
	char* mm;
	printf(pad_num(itoa(now.minutes,mm,10)));
	printf(":");
	char* ss;
	printf(pad_num(itoa(now.seconds,ss,10)));
}
char equal_time(struct time a, struct time b)
{
    return (a.seconds==b.seconds)&(a.minutes==b.minutes)&(a.hours==b.hours);
}
int atoi(char *p)
{
    int k = 0;
    while (*p) {
        k = (k<<3)+(k<<1)+(*p)-'0';
        p++;
     }
    return k;
}
void dump(unsigned char * p)
{
    unsigned int lines = 0;
    char vvv[9];
    unsigned char * p1 = p;
    printf("\t0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    while(1)
    {
    	//if (lines == 25){terminal_initialise();lines = 0;}
    	char * j = p;
    	printf("%x",(size_t)p-(size_t)p1);
    	printf("\t");
	for (int i = 0;i < 16;i++) {unsigned char v = *p;printf("%c%c ",vv[v/16],vv[v%16]);p++;}
    	printf("\t\t");
      	for (int i = 0;i < 16;i++) {printf("%c",*(j++));}
	puts("");
    	lines++;
	if(readchar()=='\n') return;
    }
}	/*
	puts(itoa(readchar(),v,10));
	printf ("TTL: ");
	char* tt = readline(v,1);
	int t = atoi(tt);
	puts(tt);
	puts(itoa(t,v,10));
	struct time init = get_time();
	struct time add = {.seconds=t};
	struct time stop = add_time(init,add);
	printf("Halting at: ");
	print_time(stop);
	printf("\n");
	char s = ask_cmos(0x00);
	while(1){
		//terminal_initialize();        
        s = ask_cmos(0x00);
		clearline();
        struct time now = get_time();
        print_time(now);
        if (equal_time(now,stop)) break;
		while(ask_cmos(0x00)==s);
	}
	terminal_initialise();
	puts("GOING DOWN NOW!");
	puts("But first, because it looks cool, we shall dump memory!");
	char vvv[9] = "000000000";
	unsigned char * p = 0x10000000;
	puts(p);
	puts(hexdump(&p,vvv));
	puts(hexdump(*vvv,vvv));
	terminal_initialise();
	while(1)
	{
    	//if (lines == 25){terminal_initialise();lines = 0;}
    	readcode();
    	unsigned int * j = p;
    	printf(hexdump((p),vvv));
    	printf("\t");
    	printf(hexdump(*p,vvv));p++;p++;p++;p++;
    	printf(hexdump(*p,vvv));p++;p++;p++;p++;
    	printf(hexdump(*p,vvv));p++;p++;p++;p++;
    	printf(hexdump(*p,vvv));p++;p++;p++;p++;
    	printf("\t\t");
    	terminal_putchar(*j);j++;
    	terminal_putchar(*j);j++;
    	terminal_putchar(*j);j++;
    	terminal_putchar(*j);j++;
    	terminal_putchar(*j);j++;
    	terminal_putchar(*j);j++;
    	terminal_putchar(*j);j++;
    	terminal_putchar(*j);j++;
    	terminal_putchar(*j);j++;
    	terminal_putchar(*j);j++;
    	terminal_putchar(*j);j++;
    	terminal_putchar(*j);j++;
    	terminal_putchar(*j);j++;
    	terminal_putchar(*j);j++;
    	terminal_putchar(*j);j++;
    	terminal_putchar(*j);j++;
    	puts("");
    	lines++;
    }
char* v;
    printf("CLOCKOS\n");
    readcode();
    char vvv[9] = "000000000";
    readcode();
    puts("DISKIO");
    readcode();
    halt();
    struct pci_device dev[8192];
    int i = pciEnumAll(dev);
    for (;i<0;i--) {printf(hexdump(dev[i].bus,vvv));printf(":");puts(hexdump(dev[i].device,vvv));};
    return;
    //while(1){char vv = ; if(vv!=x){io_wait();terminal_putchar(scancode[vv]);}}//printf(" ");puts(itoa(vv,v));};}x=vv	
*/
/*
{
        char v[9] = {0,0,0,0,0,0,0,0,0};
        printf(hexdump(sizeof(HBA_MEM),v));
    }
    puts("DISK IO");
    char vvv[9] = "000000000";
    struct pci_device d[32];
    int i = pciEnumAll(d);
    puts("DONE!");
    printf("Got ");
    printf(hexdump(i,vvv));
    puts(" devices.");
    puts("Select device:");
    puts("N\tBus\t\tDevice\tType\tVendor\tFunction");
    for (int j = i-1;j>=0;j--)
    {
        printf(hexdump(j,vvv)+7);printf("\t");
        printf(hexdump(d[j].bus,vvv)+4);printf("\t");
	printf(hexdump(d[j].device,vvv)+4);printf("\t");
	printf(hexdump(d[j].class,vvv)+4);printf("\t");
	printf(hexdump(d[j].vendor,vvv)+4);printf("\t");
	printf(hexdump(d[j].function,vvv)+4);printf("\t");
	puts("");
    };
    int c = readchar();
    puts(hexdump(c-'0',vvv));
    struct pci_device dev = d[c-'0'];
    FIS_REG_H2D * fis;
    memset(fis, 0, sizeof(FIS_REG_H2D));
    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->command = 0xEC;	// 0xEC
    fis->device = 0;			// Master device
    fis->c = 1;				// Write command register
    printf("Selected ");
    puts(hexdump(((unsigned int) dev.bus<<16|(unsigned int) dev.device<<8|(unsigned int) dev.function << 8),vvv)+2);
    
    HBA_MEM * m = PCI2HBA(dev);
    //dump(pciConfigReadReg(dev.bus,dev.device,dev.function,HBA_MEM_PCI_OFFSET));
    //unsigned char read(HBA_PORT *port, unsigned int startl, unsigned int starth, unsigned int count, unsigned short *buf);
    puts("enum");
    int v[32];
    enumerateHBA_MEM(m,v);
    printf("Dev: ");
    c = readchar();
    puts(&c);
    HBA_PORT * port = (m->ports)+(sizeof(HBA_PORT)*(c-'0'));
    unsigned short* b = 133769;
    unsigned char r = read(port,1,1,16,b);
    puts ("DONE");
    puts(hexdump(r,vvv));
    dump(133769);
*/
static unsigned int crc32tab[] = { /* CRC polynomial 0xedb88320 */
  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
  0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
  0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
  0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
  0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
  0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
  0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
  0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
  0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
  0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
  0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
  0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
  0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
  0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
  0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
  0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
  0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
  0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
  0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
  0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
  0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
  0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
  0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
  0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
  0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
  0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
  0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
  0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
  0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
  0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
  0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
  0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
  0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
  0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
  0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
  0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
  0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
  0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
  0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
  0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
  0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
  0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
  0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};
uint32_t crc32(const char *buf, size_t len)
{
  unsigned int crc = 0xFFFFFFFF;
  for ( ; len; --len, ++buf) crc = crc32tab[((crc) ^ (*buf)) & 0xff] ^ ((crc) >> 8);
  return ~crc;
}
typedef struct __attribute__((packed))
{
    unsigned long long sig;
    unsigned char rev[4];
    unsigned int size;
    unsigned int crc;
    unsigned int rsvd1;
    unsigned long long this:48;
    unsigned short ignore1;
    unsigned long long backup:48;
    unsigned short ignore2;
    unsigned long long first;
    unsigned long long last;
    unsigned char guid[16];
    unsigned long long part_start;
    unsigned int part_num;
    unsigned int part_ent_size;
    unsigned int part_crc;
    unsigned char rsvd2[420];
} GPT_HEAD;
typedef struct __attribute__((packed))
{
    unsigned char type_guid[16];
    unsigned char part_guid[16];
    unsigned long long first;
    unsigned long long last;
    unsigned char sys:1;
    unsigned char ignore:1;
    unsigned char legacy:1;
    unsigned char rsvd1:5;
    unsigned char rsvd2[5];
    unsigned char specific[3];
    unsigned char name[72];
} GPT_PART;

typedef struct __attribute__((packed))
{
    unsigned char sector:6;
    unsigned short cylinder:10;
    unsigned char head;
} CHS;

#define BAD_CHS {.cylinder = 1023, .head=255, .sector = 63}

typedef struct __attribute__((packed))
{
    unsigned char status;
    CHS start;
    unsigned char type;
    CHS end;
    unsigned int LBA;
    unsigned int sectors;
} MBR_PART;

typedef struct __attribute__((packed))
{
    unsigned char bootstrap[0x1b4];
    unsigned char id[0xA];
    MBR_PART parts[0x4];
    unsigned short sig;
} MBR_HEAD;
#define EFI_SIG 0x5452415020494645
#define MBR_SIG 0xAA55
#define EFI_REV {0,0,1,0}
#define GPT_SIZE 0x5C
void atou16(char * dest, char * str)
{
  dest--;
  for (int n = strlen(str);n;n--)
  {
    *dest = *str;
    str++;
    dest++;
    dest++;
  }
}
void gen_crc(GPT_HEAD * h)
{
    h->crc = crc32(h,GPT_SIZE);
}
void gen_part_crc(GPT_HEAD * h, GPT_PART * p)
{
  h->part_crc = crc32(p,h->part_num*h->part_ent_size);
}

void real_mode()
{
  pciEnumAll();
  //Finally leave real mode
  puts("Leaving real mode");
  asm("nop");
}
 
unsigned char rand( void ) // RAND_MAX assumed to be 32767
{
    static unsigned long int next = 1;
    next = next * 1103515245 + 12345;
    return (unsigned char)(next / 65536) % 256;
}
void randn(unsigned char * dest,size_t n)
{
  while (n)
  {
    *dest = rand();
    n--;
    dest++;
  }
}
void encodeGdtEntry(uint8_t target[8], GDT source)
{
    // Check the limit to make sure that it can be encoded
    if ((source.limit > 65536) && ((source.limit & 0xFFF) != 0xFFF)) {
        puts("You can't do that!");
	halt();
    }
    if (source.limit > 65536) {
        // Adjust granularity if required
        source.limit = source.limit >> 12;
        target[6] = 0xC0;
    } else {
        target[6] = 0x40;
    }
 
    // Encode the limit
    target[0] = source.limit & 0xFF;
    target[1] = (source.limit >> 8) & 0xFF;
    target[6] |= (source.limit >> 16) & 0xF;
 
    // Encode the base 
    target[2] = source.base & 0xFF;
    target[3] = (source.base >> 8) & 0xFF;
    target[4] = (source.base >> 16) & 0xFF;
    target[7] = (source.base >> 24) & 0xFF;
 
    // And... Type
    target[5] = source.type.byte;
}
void test(){puts("hi");}
size_t test_loc = (size_t) test;
//{0x0F,0xC6,0x3D,0xAF,0x84,0x83,0x47,0x72,0x8E,0x79,0x3D,0x69,0xD8,0x47,0x7D,0xE4}
void kernel_main(void)
{
    real_mode();
    printf("%x\n",test_loc);
    printf("%i",sizeof(DescriptorPointer));
    char v[256];
    //unsigned char* apic = InitApic();
    //if (!check_apic) {clear();puts("NO APIC! Halting!");}
    //printf("%i\n",apic);
    puts("Loading new IDT and gdt");
    DescriptorPointer idt = {.length = 0x800,.ptr = IDT_TABLE};
    DescriptorPointer gdt = {.length = 3*8,.ptr = GDT_TABLE};
    IDT descr =
      {
	.offset = test_loc,
	.selector = 0x8,
	.type_attr = IDT_STDINT|0b10000000
      };
    WriteAtIDT(IDT_TABLE,descr,0x80);
    //    WriteAtIDT(IDT_TABLE,descr,0x1);
    GDT gdt_tab[] =
    {
      {.base=0,.limit=0,.type={.byte=0}},
      {.base=0,.limit=0xFFFFFFFF,.type={.byte=0x9A}},
      {.base=0,.limit=0xFFFFFFFF,.type={.byte = 0x92}}
    };
    WriteAllGDT(GDT_TABLE,gdt_tab,3);
    dump(GDT_TABLE);
    puts("Adding IRQ Redirect");
    /*RedirectionEntry red =
      {
	.vector=0x50
	};*/
    //addIRQRed(apic,0,&red);
    LoadIDT(idt);
    LoadGDT(gdt);
    puts("Done!");
    puts("Int 0x80");
    readline(v,0,256);
    asm("cli;INT $0x80");
    /*
      The system is now up and running! This is good.
      So now we have to transfer execution to another bit of memory
      Since this is the basis of an operating system, it is easy and there is a immediate value based instruction!

      
      JK
    */
    puts("Here we go!");
    asm volatile(""::"a"(test));
    asm volatile("jmp %eax");
    puts("=(");
    return 0;
}


/*
    if (get_abar()) {puts("No AHCI device. Halting!");halt();}
    puts("found abar");
    char v[13]="????????????";
    printf ("Running on a %s processor\n", get_vendor(v));
    int ports[32];
    probe_port(ports);
    puts("Init disk!");
    HBA_PORT * port = &abar->ports[0];
    //Get the identify data
    puts("ident");
    AHCI_IDENTIFY id;
    identify_disk(port, &id);
    unsigned long long sectors;
    memcpy(&sectors,id.total_usr_sectors,3);
    printf("The disk has %i total sectors\n",sectors);
    char * h = "h";
  */

/*
    char* a = "a\0b";
    //Create the MBR header
    MBR_HEAD mbr_head = {.id={10,0,0,0,0,0,0,0,0,0},.sig = MBR_SIG};
    //Create the MBR partion
    MBR_PART mbr_part =
      {
	.status = 0x00,
	.type = 0xEE,
	.LBA = 1,
	.start = BAD_CHS,
	.end = BAD_CHS,
	.sectors = (unsigned int)sectors
      };
    //Put the MBR partition in the MBR header
    memcpy(&mbr_head.parts,&mbr_part,0x10);
    //Write the mbr header to disk
    write(port,0,0,1,&mbr_head);
    //Create the GPT header:
    GPT_HEAD gpt_head =
      {
	.sig = EFI_SIG,
	.rev = EFI_REV,
	.size = GPT_SIZE,
	.this = 1,
	.backup = (unsigned int)sectors-16,
	.first = 3,
	.last = (unsigned int)sectors-16,
	.guid = {1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6},
	.part_start = 2,
	.part_num = 4,
	.part_ent_size = 0x80
      };
    GPT_PART gpt_part1 =
      {
	.type_guid = {0x43, 0x79, 0x63, 0x6c, 0x69, 0x63, 0x4f, 0x53, 0x20, 0x50, 0x61, 0x72, 0x74, 0x0, 0x0, 0x0},
	.part_guid = {0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6},
	.first = 18,
	.last = 1024,
      };
    atou16(gpt_part1.name,"CyclicOS partition");
    //Create the GPT partition table
    GPT_PART parts[4];
    //Put the GPT partition into the table
    memcpy(parts,&gpt_part1,0x80);
    //memcpy(&parts[1],&gpt_part2,0x80);
    //Write the GPT partition table to disk
    write(port,2,0,1,parts);
    gen_part_crc(&gpt_head, &parts);
    gen_crc(&gpt_head);
    //Write the GPT header to disk
    write(port,gpt_head.this,0,1,&gpt_head);
    dump(&gpt_part1);
    /*
    //Crete the GPT backup header
    GPT_HEAD gpt_head_backup =
      {
	.sig = EFI_SIG,
	.rev = EFI_REV,
	.size = GPT_SIZE,
	.this = (unsigned int)sectors-16,
	.backup = 1,
	.first = 3,
	.last = 8,
	.part_start = 2,
	.part_num = 4,
	.part_ent_size = 0x80
      };
    //Write the GPT backup header to disk
    write(port,gpt_head.backup,0,0x200,&gpt_head);
    */
