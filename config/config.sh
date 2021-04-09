./waf build
for((i=14;i<=16;i+=1)) do
{
    ./waf --run "courier config/config-max-fetchers-$i.ini result/config-max-fe.log" 2> result/config-max-fe.error
}&
done
