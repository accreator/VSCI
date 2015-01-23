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
return s;
}