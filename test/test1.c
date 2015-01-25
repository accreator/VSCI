//test1
int meow()
{
int i,s;
i=0;
s = 0;
while(i<10)
{
s = s + i;
i = i + 1;
if(i%5==0) break;
}
s = s + 1 - 2 - 3;
return s;
}