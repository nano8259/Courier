./waf build
for((i=14;i<=16;i+=1)) do
{
    ./waf --run "courier config-max-fetchers-$i.ini result/config-max-fetchers-$i.log" 2> result/config-max-fetchers-$i.error
}&
done
