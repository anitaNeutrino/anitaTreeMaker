for run in `seq 127 439`; do
    RUN_DIR="$ANITA_ROOT_DATA/run$run"
    if [[ -d $RUN_DIR ]] ; then
	echo "Got $RUN_DIR"
	./makeGpsEventTreeBothAdu5s $run
    else
	echo "Missing $RUN_DIR"
    fi
done
