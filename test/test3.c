float ln()
{
float x;
float y;
float ret;
int i;
x = 1.5;

i = 0;
x = x - 1;
y = x;
ret = 0;
while(i<100) {
if(i%2 == 0)
ret = ret + y/(i+1);
else{
ret = ret - y/(i+1);}
y = y*x;
i=i+1;
}
return ret;
}
