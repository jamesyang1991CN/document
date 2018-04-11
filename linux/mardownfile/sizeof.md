## c 字节大小

在学习中经常遇到 byte的处理 甚至关于bit的处理，还有涉及到结构体强制转化，分析字节占用内存的问题，
每次分析重头再来，耗费时间，于是将 字节的大小 整理归纳


## sizeof() 显神通

**用法**
> sizeof(类型说明符，数组名或表达式);<br/>
> sizeof (变量名);

```c
//64bit OS sizeof 计算
int main(int argc, char const *argv[]) {

  printf("char %ld\n", sizeof(char) ); //1
  printf("int  %ld\n", sizeof(int) ); //4
  printf("unsigned int  %ld\n", sizeof(unsigned int) ); //4
  printf("float  %ld\n", sizeof(float) ); //4
  printf("double  %ld\n", sizeof(double) ); //8

  char *pc ="abc";
  printf("pc  %ld\n", sizeof(pc) ); //8 表示指针地址 64位系统
  printf("*pc  %ld\n", sizeof(*pc) );//指针指向a char 所以答案是1

  int *pi;
  printf("pi  %ld\n", sizeof(pi) ); //8
  printf("*pi  %ld\n", sizeof(*pi) ); //4 int类型

  char **ppc = &pc;
  printf("ppc  %ld\n", sizeof(ppc) ); //8
  printf("*ppc  %ld\n", sizeof(*ppc) ); //8
  printf("**ppc  %ld\n", sizeof(**ppc) ); //1

  void (*pf)();//函数指针
  printf("pf  %ld\n", sizeof(pf) ); //8
  printf("*pf  %ld\n", sizeof(*pf) ); //1

  //sizeof 数组
  char a1[] = "abc";
  int a2[3];
  printf("a1  %ld\n", sizeof(a1) ); //4 如果是字符 表示字符串 引用地址， 表示首地址 指针指向首地址
  printf("a2  %ld\n", sizeof(a2) ); //12  是int 表示 数组占用的内存的大小

  struct MyStruct
  {
     double dda1; //8
     char dda;//1
     int type;//4
  };

  struct MyStruct mystruct;
  printf("mystruct  %ld\n", sizeof(mystruct) );//因为字节对齐 所以16 而不是13
}


```
