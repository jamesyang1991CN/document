# include<stdio.h>
/*void foo3(char a3[3]) {

 int c3 = sizeof(a3);
  printf("foo3  %ld\n", c3);
}
void foo4(char a4[]) {

 int c4 = sizeof(a4);
  printf("foo4  %ld\n", a4);
}*/

#include <arpa/inet.h>

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



  // foo3(a1);
  // foo4(a1);
  struct MyStruct
  {
     int dda1; //8
     char dda;//1
     int type;//4 //对齐
  }my,*mys;

  struct MyStruct mystruct;
  printf("mystruct  %ld\n", sizeof(mystruct) );//16

  unsigned char * chr = "123456abcdefghilmnopqist";

  mys = (struct MyStruct *)chr;

  printf("%d  %c %d\n",mys->dda1,mys->dda,mys->type );

  printf("short int %ld\n", sizeof(short int) ); //2
  printf("unsigned short int %ld\n", sizeof(unsigned short int) ); //2



  printf("in_addr_t %ld\n", sizeof(in_addr_t) ); //4






  return 0;

}
