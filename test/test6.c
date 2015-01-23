int prime() {
int i=2020202020;
while(i>0)
{
int j=2;
while(j*j<=i)
{
if(i%j == 0) break;
j = j + 1;
}
if(i%j == 0)
{
i = i-1;
continue;
}
//return (i+1)%j;
break;
}
return i;
}