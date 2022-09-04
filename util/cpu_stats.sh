basedir=$PWD
stats_file=$basedir/top_stats.txt
stats_interval=5

touch $stats_file

num_cores=$(lscpu -p=cpu | sed '/^#/d' | wc -l)
top_stats_num_lines=$(( 5 + $num_cores ))

while true
do
    top -b -n 1 | head -n $top_stats_num_lines >> $stats_file
    sleep $stats_interval
done
