float sqrt()
{
float x = 2;
float l = 0, r = x+1, m;
while(r-l > 0.00001)
{
m = (l+r)/2;
if(m*m < x) l = m; else r = m;
}
return (l+r)/2;
}