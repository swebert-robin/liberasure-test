basedir=/home/robin/object-store/erasure/test/liberasure
prog=$basedir/bin/main

function test() {
    local ec_k=9
    local ec_m=3
    local backend=$1
    local blocksize=$2
    local total_iterations=100000
    local min_iterations=1000
    local max_runtime=60

    outfile=$basedir/results/results.$backend.txt
    # outfile=$basedir/results/results.txt
    # outfile=/dev/null
    touch $outfile

    # local Ki="[0-9]*Ki"
    # local Mi="[0-9]*Mi"
    # local Gi="[0-9]*Gi"

    # if [[ $blocksize =~ $Ki ]]
    # then
    #     local min_iterations=1000
    #     local max_runtime=60
    # elif  [[ $blocksize =~ $Mi ]]
    # then
    #     local min_iterations=1000
    #     local max_runtime=40
    # elif  [[ $blocksize =~ $Gi ]]
    # then
    #     local min_iterations=1000
    #     local max_runtime=20
    # else
    #     local min_iterations=0
    #     local max_runtime=0
    # fi

    echo "blocksize: $blocksize"
    $prog $ec_k $ec_m `numfmt --from=iec-i $blocksize` $backend $total_iterations $min_iterations $max_runtime | tee -a $outfile
    echo "---" | tee -a $outfile
}

blocksizes=(
    4Ki
    16Ki
    32Ki
    64Ki
    128Ki
    256Ki
    1Mi
    2Mi
    4Mi
    16Mi
    32Mi
    64Mi
    128Mi
    256Mi
    512Mi
    1Gi
    1.25Gi
    2Gi
)

backends="jv jc iv ic"
if [ "$#" -eq 1 ]
then
    backends="$1"
fi

for backend in $backends
do
    for blocksize in ${blocksizes[*]} 
    do
        test $backend $blocksize
    done
done
