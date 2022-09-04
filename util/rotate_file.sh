basedir=/home/robin/object-store/erasure/test/liberasure/results

function rotate() {
    local backend=$1
    local filepath=$basedir/results.$backend.txt

    if [ -f $filepath ]
    then
        n=1
        while [ -f $filepath.$n ]; do ((n++)); done
        mv $filepath $basedir/results.$backend.txt.$n
    fi
}

for backend in "ic" "iv" "jc" "jv"
do
    rotate $backend
done
