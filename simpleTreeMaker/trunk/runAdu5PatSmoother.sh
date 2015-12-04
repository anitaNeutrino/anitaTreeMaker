for run in `seq 1 439`; do
    RUN_DIR="/unix/anita3/flight1415/root/run$run"
    if [[ -d $RUN_DIR ]] ; then
	echo "Got $RUN_DIR"
	./makeAdu5PatFiles $run
    else
	echo "Missing $RUN_DIR"
    fi
done
