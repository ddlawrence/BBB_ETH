//
// libc.c - spoofed C Library routines
//
#include <stdint.h>
#define size_t            unsigned int
#define CONSOLE_UART      0x44E09000  // this is SOC_UART_0_REGS

extern void uart_tx(uint32_t uart_base_addr, uint32_t byte);

//
// memcmp
//
int memcmp(const char *cs, const char *ct, size_t n) {  //   TODO UNTESTED
  size_t i;   
  for (i = 0; i < n; i++, cs++, ct++) {
    if (*cs < *ct) return -1;
    if (*cs > *ct) return 1;
  }
  return 0;
}

//
// memcpy    from Daniel Vik
//
void *memcpy(void *dest, const void *src, size_t count) {
  char* dst8 = (char*)dest;
  char* src8 = (char*)src;
  while (count--) {
    *dst8++ = *src8++;
  }
  return dest;
}

//
// memset
//
void *memset(void *ptr, int value, size_t num) {
  char* dst8 = (char*)ptr;
  unsigned char val = (unsigned char)value;
  while (num--) {
    *dst8++ = val;
  }
  return ptr;
}

//
// strcat   from Jonny Haryana
//
char *strcat(char p[], char q[]) {  //    TODO UNTESTED
  int c, d;
  c = 0;
  while (p[c] != '\0') {
    c++;  	
  }
  d = 0;
  while (q[d] != '\0') {
    p[c] = q[d];
    d++;
    c++;	
  }
  p[c] = '\0';
  return p;
}

//
// strcmp    from Mathias Van Malderen
//
int strcmp(const char *s1, const char *s2)         //    TODO UNTESTED
{
  while((*s1 && *s2) && (*s1 == *s2))
    s1++,s2++;
  return *s1 - *s2;
}

//
// strncmp
//
int strncmp(const char *cs, const char *ct, size_t num)         //    TODO UNTESTED
{
  size_t i;   
  for (i = 0; i < num; i++, cs++, ct++) {
    if(*cs < *ct) return -1;
    if(*cs > *ct) return 1;
    if(*cs == 0x0) break;
  }
  return 0;
}

//
// strcpy    from Jonny Haryana
//
char *strcpy(char d[], char s[]) {
  int c = 0;
  while (s[c] != '\0') {
    d[c] = s[c];
    c++;
  }
  d[c] = '\0';
  return d;
}

//
// strncpy    from Daniel Vik
//
char *strncpy(char *dest, char *src, size_t count) {         //    TODO UNTESTED
  char* dst8 = (char*)dest;
  char* src8 = (char*)src;
  while (count--) {
    *dst8++ = *src8++;
  }
  return dest;
}

//
// strlen
//
size_t strlen(const char *str) {  //    TODO UNTESTED
	register const char *s;
	for (s = str; *s; ++s);
	return(s - str);
}

//
// idiv from github.com/auselen
//
typedef struct { int quot; int rem; } idiv_return;
typedef struct { size_t quot; size_t rem; } uidiv_return;

idiv_return __aeabi_idivmod(int num, int denom) {
  idiv_return r;
  int q = 0;
  while (num >= denom) {
    num -= denom;
    q++;
  }
  r.quot = q;
  r.rem = num;
  return r;
}

int __aeabi_idiv(int num, int denom) {
  int q = 0;
  while (num >= denom) {
    num -= denom;
    q++;
  }
  return q;
}

uidiv_return __aeabi_uidivmod(size_t num, size_t denom) {
  uidiv_return r;
  size_t q = 0;
  while (num >= denom) {
    num -= denom;
    q++;
  }
  r.quot = q;
  r.rem = num;
  return r;
}

size_t __aeabi_uidiv(size_t num, size_t denom) {
  size_t q = 0;
  while (num >= denom) {
    num -= denom;
    q++;
  }
  return q;
}

//
// ConsolePrintf - ascii formatted print to consoleUART
//
// hacked from github.com/auselen       beautiful job
//
// __builtin_frame_address(0) - is the address of the first word pushed 
//                              on to the stack by the function
//
// void ConsolePrintf(const char *fmt, ...) {
void ConsolePrintf(char *fmt, ...) {
  int *stack_head = __builtin_frame_address(0);
  stack_head += 2; // skip fmt, skip stack_head
  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt++) {
        case 'c': {
          uart_tx(CONSOLE_UART, *stack_head++);
          break;
        }
        case 's': {
          const char *s = (char *) *stack_head++;
          while (*s) {
            uart_tx(CONSOLE_UART, *s++);
          }
          break;
        }
        case 'x': {
          int num = *stack_head++;
          int shift = 28;
          while (shift >= 0) {
            int hd = (num >> shift) & 0xf;
            if (hd > 9)
              hd += 'A' - 10;
            else
              hd += '0';
            uart_tx(CONSOLE_UART, hd);
            shift -= 4;
          }
          break;
        }
        case 'd': {
          int num = *stack_head++;
          char buf[16];
          char *s = buf + (sizeof(buf) / sizeof(buf[0])) - 1;
          char *e = s;
          do {
            *--s = '0' + num % 10;
          } while (num /= 10);
          while (s < e)
            uart_tx(CONSOLE_UART, *s++);
            break;
        }
        default:
          uart_tx(CONSOLE_UART, '?');
      }
    } else {
      uart_tx(CONSOLE_UART, *fmt++);
    }
  }
}

// eof
